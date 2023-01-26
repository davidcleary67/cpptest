#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <cstdlib>
#include <time.h>
#include "backup.h"

using namespace std;

vector<SJob> vJobs;
string sUser;

void loadUserJobs(void)
{
    bool bResult = true;
    string sJob0;
    SJob stJob1;

    ifstream fConfig(BACKUPCONFIG);    
  
    // Read user 
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

void displayJob(SJob sJob)
{
    cout << "Job ID: " << sJob.sJob << " Src: " << sJob.sSrc << " Dst: " << sJob.sDst << "\n";
}

void displayJobs(void)
{
    for (auto i = vJobs.begin(); i != vJobs.end(); i ++)
    {
        displayJob(*i);
    }
}

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

void copyFile(string sSrc, string sDst)
{
    string sSrcFilename = sSrc.substr(sSrc.find_last_of("/") + 1);
    string sCommand = "cp " + sSrc + " " + sDst + "/" + sSrcFilename + dateTimeStamp();
    system(sCommand.c_str());
}

void copyDirectory(string sSrc, string sDst)
{
    string sSrcFilename = sSrc.substr(sSrc.find_last_of("/") + 1);
    string sCommand = "cp -a " + sSrc + " " + sDst + "/" + sSrcFilename + dateTimeStamp();
    system(sCommand.c_str());
}

void writeLogMessage(bool bStatus, string sMessage)
{
    ofstream fLog(BACKUPLOG, std::ios_base::app);    
    fLog << (bStatus ? "SUCCESS " : "FAILURE ") << dateTimeStamp(true) << " " << sMessage << "\n";
    fLog.close();
}

void sendEmailMessage(string sMessage)
{
    string sSubject = "Backup Failure";
    string sCommand = "echo \"" + dateTimeStamp(true) + " " + sMessage + "\" | mailx -s \"Backup Failure\" -r \"davidcgcleary@gmail.com\" -S smtp=\"smtp.gmail.com\" " + sUser;
    //# echo "The actual message goes here" | mailx -v -r "user@domain.com" -s "The actual subject line goes here" -S smtp="smtp.domain.com:587" -S smtp-use-starttls -S smtp-auth=login -S smtp-auth-user="user@domain.com" -S smtp-auth-password="password123" -S ssl-verify=ignore the_recipient_email@domain.com
    //cout << sCommand;
    system(sCommand.c_str());
}

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
        // Load all jobs from file
        loadUserJobs();
        //displayJobs();
        
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