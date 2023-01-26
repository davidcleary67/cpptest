using namespace std;

struct SJob
{
    string sJob;
    string sSrc;
    string sDst;
};

const string BACKUPCONFIG = "backup.cfg";
const string BACKUPLOG = "backup.log";

void loadUserJobs(void);
void displayJob(SJob sJob);
void displayJobs(void);
bool getJob(string sJob, string &sSrc, string &sDst);
string dateTimeStamp(bool bVerbose);
void copyFile(string sSrc, string sDst);
void copyDirectory(string sSrc, string sDst);
void writeLogMessage(bool bStatus, string sMessage);
void sendEmailMessage(string sMessage);