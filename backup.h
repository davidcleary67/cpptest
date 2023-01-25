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