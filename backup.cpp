#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
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

// Close the file
int main(int argc, char *argv[])
{
    bool bResult = true;
    string sJob = "";
    string sSrc= "";
    string sDst = "";
    bool bSrcIsDir = true;
    
    if (argc != 2)
    {
        cout << "Usage: backup <job>\n";
    }
    else
    {
        loadJobs();
        displayJobs();
        
        sJob = argv[1];
        bResult = getJob(sJob, sSrc, sDst);
        
        if (!bResult)
        {
            cout << "Error: Job " << sJob << " not in jobs.\n";
        }
        else
        {
            //cout << "Job " << sJob << ".\n";
            struct stat sb;
            if (stat(sSrc.c_str(), &sb) != 0)
            {
                cout << "Error: The src " << sSrc << " is invalid.\n";
            }
            else
            {
                bSrcIsDir = (sb.st_mode & S_IFDIR);
                cout << "Src is valid and is a " << (bSrcIsDir ? "directory" : "file") << "\n";
            }
        }
    }
    return 0;
}