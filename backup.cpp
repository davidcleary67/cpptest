/*
Program: backup
Usage: backup <jobName>
Author: David Cleary
Date: Jan 2023
Copyright: SuniTAFE 2023

Description: The backup program can be used to backup a file or a directory to
a specified location.  The program's argument specifies the name of the 
backup job to execute.  Job names, source source files or directories and 
destination directories are specified in the configuration file, backup.cfg, 
starting at the second line.  The first line specifies the email address of 
the user to whom error reports are sent if a backup fails.  Each backup up
file or directory has a datetime stamp appended to it.  Each successful or 
failed backup is recorded in the log file, backup.log.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <cstdlib>
#include <time.h>
#include "backup.h"

using namespace std;

// Information loaded from backup.cfg
vector<SJob> vJobs; // List of jobs.  Each job has a name, source and destination
string sUser;       // Email address of user to whom error reports are sent

// SMTP server access settings to send emails
string sSMTPUsername; // Email address of user to access SMTP server
string sSMTPPassword; // Password of user to access SMTP server

// Load user email address and all defined jobs from configuration file, backup.cfg
bool loadUserJobs(void)
{
    bool bResult = true;
    
    try
    {
        string sJob0;
        SJob stJob1;
    
        ifstream fConfig;
       
        fConfig.exceptions(ios::failbit | ios::badbit); 
        
        fConfig.open(BACKUPCONFIG);    
      
        fConfig.exceptions(ios::goodbit); 
        
        // Read user email address
        getline(fConfig, sUser);
       
        // Read jobs 
        while (getline(fConfig, sJob0))
        {
            stringstream stJob0(sJob0);
        
            getline(stJob0, stJob1.sJob, ' ');
            getline(stJob0, stJob1.sSrc, ' ');
            getline(stJob0, stJob1.sDst, ' ');
            
            vJobs.push_back(stJob1);
        }
        
        fConfig.close();
    }
    catch (exception &e)
    {
        string sMessage = "Error reading configuration file '" + BACKUPCONFIG + "'. Exception: " + e.what();
        writeLogMessage(false, sMessage);
        bResult = false;
    }
    
    return bResult;
}

// Load SMTP access settings from .backup file
bool loadSMTPSettings(void)
{
    bool bResult = true;
    
    try
    {
        ifstream fSettings;
        
        fSettings.exceptions(ios::failbit | ios::badbit); 
        
        fSettings.open(SMTPSETTINGS.c_str());    
        
        // Read SMTP user email address and password
        getline(fSettings, sSMTPUsername);
        getline(fSettings, sSMTPPassword);
       
        fSettings.close();
        
        //cout << sSMTPPassword << "\n";
    }
    catch (exception &e)
    {
        string sMessage = "Error reading SMTP access settings file '" + SMTPSETTINGS + "'. Exception: " + e.what();
        writeLogMessage(false, sMessage);
        bResult = false;
    }
    
    return bResult;
}

// Display a defined job 
void displayJob(SJob sJob)
{
    cout << "Job Name: " << sJob.sJob << " Src: " << sJob.sSrc << " Dst: " << sJob.sDst << "\n";
}

// Display all defined jobs 
void displayJobs(void)
{
    for (auto j = vJobs.begin(); j != vJobs.end(); j ++)
    {
        displayJob(*j);
    }
}

// Determine the source and destination for a specified job name
bool getJob(string sJob, string &sSrc, string &sDst)
{
    bool bResult = false;
   
    auto i = vJobs.begin();
    while (i != vJobs.end())
    {
        if (sJob == i->sJob)
        {
            sSrc = i->sSrc;
            sDst = i->sDst;
            bResult = true;
            break;
        }
        
        i ++;
    }
    return bResult;
}

// Get the current datetime stamp.  The argument, bVerbose, determines the
// datetime stamp format
string dateTimeStamp(bool bVerbose = false)
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, (bVerbose ? "%c" : "%Y%m%d-%H%M%S"), timeinfo);
    return buffer;
}

// Copy a specified file to a destination directory.  Append a datetime stamp
// to the copied file
void copyFile(string sSrc, string sDst)
{
    string sSrcFilename = sSrc.substr(sSrc.find_last_of("/") + 1);
    string sCommand = "cp " + sSrc + " " + sDst + "/" + sSrcFilename + dateTimeStamp();
    system(sCommand.c_str());
}

// Copy a specified directory to a destination directory.  Append a datetime stamp
// to the copied directory
void copyDirectory(string sSrc, string sDst)
{
    string sSrcFilename = sSrc.substr(sSrc.find_last_of("/") + 1);
    string sCommand = "cp -a " + sSrc + " " + sDst + "/" + sSrcFilename + dateTimeStamp();
    system(sCommand.c_str());
}

// Write a success or failure message to the log file, backup.log.  Include a
// datetime stamp and a description
void writeLogMessage(bool bStatus, string sMessage)
{
    ofstream fLog(BACKUPLOG, std::ios_base::app);    
    fLog << (bStatus ? "SUCCESS " : "FAILURE ") << dateTimeStamp(true) << " " << sMessage << "\n";
    fLog.close();
}

// Send an email to the user email address read from the configuration file to
// indicate that a backup job has failed.  Include a datetime stamp and a 
// description
void sendEmailMessage(string sMessage)
{
    string sSubject = "Backup Failure";
    string sCommand = "echo -e \"<div style='font-size:40px;'> "
                      "<img src='https://cdn1.iconfinder.com/data/icons/toolbar-std/512/error-512.png' "
                      "height='80px' width='80px' style='vertical-align:middle;'> "
                      + dateTimeStamp(true) + " " + sMessage + "</div>" + "\" | "
                      "mailx "//-r \"" + sUser + "\" "
                      "-s \"$(echo -e \"Backup Failure\nContent-Type: text/html;\n\n\")\" "
                      "-S smtp=\"smtp.gmail.com:587\" "
                      "-S smtp-use-starttls "
                      "-S smtp-auth=login "
                      "-S smtp-auth-user=\"" + sSMTPUsername + "\" "
                      "-S smtp-auth-password=\"" + sSMTPPassword + "\" "
                      "-S ssl-verify=ignore "
                      "-S nss-config-dir=/etc/pki/nssdb "
                      + sUser;
    //cout << sCommand << "\n";
    system(sCommand.c_str());
}

// Main function
int main(int argc, char *argv[])
{
    bool bResult = true;
    string sJob = "";
    string sSrc= "";
    string sDst = "";
    bool bSrcIsDir = true;
    struct stat sb;

    // Command line is program and one argument    
    if (argc != 2)
    {
        cout << "Usage: backup <job>\n";
        bResult = false;
    }
    
    if (bResult)
    {
        // Load SMTP access settings from file
        bResult = loadSMTPSettings();
    }
  
    if (bResult)
    {
        // Load all jobs from file
        bResult = loadUserJobs();
        //displayJobs();
    }
    
    if (bResult)
    {    
        // Command line argument is valid job
        sJob = argv[1];
        bResult = getJob(sJob, sSrc, sDst);
        if (!bResult) 
        {
            string sMessage = "Job '" + sJob + "' is not defined.";
            writeLogMessage(false, sMessage);
            sendEmailMessage(sMessage);
        }
    }
        
    if (bResult)
    {
        // Src is valid file or directory
        bResult = (stat(sSrc.c_str(), &sb) == 0);
        if (!bResult)
        {
            string sMessage = "Source '" + sSrc + "' is invalid.";
            writeLogMessage(false, sMessage);
            sendEmailMessage(sMessage);
        }
    }
    
    if (bResult)
    {
        // Determine if src is file or directory
        bSrcIsDir = (sb.st_mode & S_IFDIR);
        
        // Dst is valid directory
        bResult = (stat(sDst.c_str(), &sb) == 0) && (sb.st_mode & S_IFDIR);
        if (!bResult)
        {
            string sMessage = "Destination directory '" + sDst + "' is invalid.";
            writeLogMessage(false, sMessage);
            sendEmailMessage(sMessage);
        }
    }
    
    if (bResult)
    {
        // Backup directory
        if (bSrcIsDir)
        {
            copyDirectory(sSrc, sDst);    
            writeLogMessage(true, "Backed up directory '" + sSrc + "' to '" + sDst + "'.");
        }
        // Backup file
        else
        {
            copyFile(sSrc, sDst);
            writeLogMessage(true, "Backed up file '" + sSrc + "' to '" + sDst + "'.");
        }
    }
    
    return 0;
}