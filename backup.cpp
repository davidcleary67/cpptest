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

void loadJobs(void)
{
    bool bResult = true;
    string sJob0;
    SJob stJob1;

    ifstream fJobs("backup.cfg");    
    
    while (getline(fJobs, sJob0))
    {
        // cout << "Job: " << sJob0 << "\n";        
        stringstream stJob0(sJob0);
    
        getline(stJob0, stJob1.sJob, ' ');
        getline(stJob0, stJob1.sSrc, ' ');
        getline(stJob0, stJob1.sDst, ' ');
        
        vJobs.push_back(stJob1);
    }
    
    fJobs.close();
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
        // cout << i->sJob << i->sSrc << i->sDst << "\n";
    }
}

bool getJob(string sJob, string &sSrc, string &sDst)
{
    bool bResult = false;
   
    auto i = vJobs.begin();
    while (i != vJobs.end())
    {
        // cout << i->sJob << i->sSrc << i->sDst << "\n";
        
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

string dateTimeStamp(void)
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y%m%d-%H%M%S", timeinfo);
    return buffer;
}

void copyFile(string sSrc, string sDst)
{
    string sSrcFilename = sSrc.substr(sSrc.find_last_of("/") + 1);
    
    string sCommand = "cp " + sSrc + " " + sDst + "/" + sSrcFilename + dateTimeStamp();
    //cout << sCommand << "\n";
    
    system(sCommand.c_str());
}

void copyDirectory(string sSrc, string sDst)
{
    string sSrcFilename = sSrc.substr(sSrc.find_last_of("/") + 1);
    
    string sCommand = "cp -a " + sSrc + " " + sDst + "/" + sSrcFilename + dateTimeStamp();
    //cout << sCommand << "\n";
    
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
        loadJobs();
        displayJobs();
        
        // Command line argument is valid job
        sJob = argv[1];
        bResult = getJob(sJob, sSrc, sDst);
        if (!bResult) 
        {
            cout << "Error: Job " << sJob << " not in jobs.\n";
        }
    }
        
    if (bResult)
    {
        // Src is valid file or directory
        //cout << "Job " << sJob << ".\n";
        bResult = (stat(sSrc.c_str(), &sb) == 0);
        if (!bResult)
        {
            cout << "Error: The src " << sSrc << " is invalid.\n";
        }
    }
    
    if (bResult)
    {
        // Determine if src is file or directory
        bSrcIsDir = (sb.st_mode & S_IFDIR);
        cout << "Src is valid and is a " << (bSrcIsDir ? "directory" : "file") << ".\n";
        
        // Dst is valid directory
        //cout << "Job " << sJob << ".\n";
        bResult = (stat(sDst.c_str(), &sb) == 0) && (sb.st_mode & S_IFDIR);
        if (!bResult)
        {
            cout << "Error: The dst directory" << sDst << " is invalid.\n";
        }
    }
    
    if (bResult)
    {
        cout << "Dst is valid and is a directory.\n";
        
        // Backup directory
        if (bSrcIsDir)
        {
            copyDirectory(sSrc, sDst);    
        }
        // Backup file
        else
        {
            copyFile(sSrc, sDst);
        }
    }
    
    return 0;
}