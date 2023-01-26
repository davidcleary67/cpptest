using namespace std;

struct SJob
{
    string sJob;
    string sSrc;
    string sDst;
};

void loadJobs(void);
void displayJob(SJob sJob);
void displayJobs(void);
bool getJob(string sJob, string &sSrc, string &sDst);
string dateTimeStamp(void);
void copyFile(string sSrc, string sDst);
void copyDirectory(string sSrc, string sDst);