
// Added by George Marselis <george@marsel.is> in 2019

#pragma once

/* daemons/sbatchd/job.c */
struct passwd *my_getpwnam(const char *name, char *caller);
void sbdChildCloseChan(int exceptChan);
sbdReplyType job_exec(struct jobCard *jobCardPtr, int chfd);
int sendNotification(struct jobCard *jobCardPtr);
void getJobTmpDir(char *tmpDirName, struct jobCard *jPtr);
void createJobTmpDir(struct jobCard *jobCardPtr);
void execJob(struct jobCard *jobCardPtr, int chfd);
char **execArgs(struct jobSpecs *jp, char **execArgv);
void resetEnv(void);
int setJobEnv(struct jobCard *jp);
int setLimits(struct jobSpecs *jobSpecsPtr);
int mysetLimits(struct jobSpecs *jobSpecsPtr);
int job_finish(struct jobCard *jobCard, int report);
void unlockHosts(struct jobCard *jp, unsigned int num);
int finishJob(struct jobCard *jobCard);
void status_report(void);
void rmvJobStarterStr(const char *line, const char *jobStarter);
int isLink(const char *filename);
void shouldCopyFromLsbatch(struct jobCard *jp, int *cpyStdoutFromLsbatch, int *cpyStderrFromLsbatch);
int send_results(struct jobCard *jp);
struct jobCard *addJob(struct jobSpecs *jobSpecs, int mbdVersion);
void renewJobStat(struct jobCard *jp);
int sbdStartupStopJob(struct jobCard *jp, int reasons, int subReasons);
void jobGone(struct jobCard *jp);
void refreshJob(struct jobSpecs *jobSpecs);
void inJobLink(struct jobCard *jp);
int setIds(struct jobCard *jobCardPtr);
void deallocJobCard(struct jobCard *jobCard);
void freeToHostsEtc(struct jobSpecs *jobSpecs);
void saveSpecs(struct jobSpecs *jobSpecs, struct jobSpecs *specs);
void setRunLimit(struct jobCard *jp, int initRunTime);
int setPGid(struct jobCard *jc);
char *getLoginShell(char *jfData, char *jobFile, struct hostent *hp, int readFile);
int createTmpJobFile(struct jobSpecs *jobSpecsPtr, struct hostent *hp, char *stdinFile);
int acctMapTo(struct jobCard *jobCard);
int acctMapOk(struct jobCard *jobCard);
void runQPre(struct jobCard *jp);
int runQPost(struct jobCard *jp);
int chPrePostUser(struct jobCard *jp);
int postJobSetup(struct jobCard *jp);
void runUPre(struct jobCard *jp);
void collectPreStatus(struct jobCard *jp, pid_t pid, char *context);
int requeueJob(struct jobCard *jp);
int reniceJob(struct jobCard *jp);
int updateRUsageFromSuper(struct jobCard *jp, char *mbuf);
void updateJUsage(struct jobCard *jPtr, const struct jRusage *jRusage);
void copyPidInfo(struct jobCard *jPtr, const struct jRusage *jRusage);
void writePidInfoFile(const struct jobCard *jPtr, const struct jRusage *jRusage);
void jobFinishRusage(struct jobCard *jp);
int initJobCard(struct jobCard *jp, struct jobSpecs *jobSpecs, int *reply);
void saveThresholds(struct jobSpecs *jobSpecs, struct thresholds *thresholds);
void freeThresholds(struct thresholds *thresholds);
void initJRusage(struct jRusage *jRusage);
int getJobVersion(struct jobSpecs *jobSpecs);
int lockHosts(struct jobCard *jp);
int REShasPTYfix(char *resPath);
void setJobArrayEnv(const char *jobName, int jobIndex);

