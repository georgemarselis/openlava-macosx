

#pragma once


struct submitMbdReply
{
    char *queue;
    char *badJobName;
    int badReqIndx;
    char padding[4];
    long jobId;
};


struct submitReq
{
    char padding1[2];
    ushort niosPort;
    int options;
    int options2;
    int sigValue;
    pid_t restartPid;
    int userPriority;
    mode_t umask;
    uint nxf;
    uint maxNumProcessors;
    uint numAskedHosts;
    uint numProcessors;
    char padding3[4];
    char *jobName;
    char *queue;
    char *jobFile;
    char *fromHost;
    char *resReq;
    char *hostSpec;
    char *subHomeDir;
    char *inFile;
    char *outFile;
    char *errFile;
    char *command;
    char *inFileSpool;
    char *commandSpool;
    char **askedHosts;
    char *dependCond;
    char *chkpntDir;
    char *cwd;
    char *preExecCmd;
    char *mailUser;
    char *projectName;
    char *loginShell;
    char *schedHostType;
    char *userGroup;
    time_t beginTime;
    time_t termTime;
    time_t chkpntPeriod;
    time_t submitTime;
    int rLimits[LSF_RLIM_NLIMITS];
    char padding2[4];
    struct xFile *xf;
};

struct jobSwitchReq
{
  char *queue;
  char padding[4];
  long jobId;

};

struct jobMoveReq
{
  uint opCode;
  int position;
  long jobId;
};

struct jobPeekReply
{
  char *outFile;
  char *pSpoolDir;
};

struct modifyReq
{
	LS_LONG_INT jobId;
	char *jobIdStr;
	int delOptions;
	int delOptions2;
	struct submitReq submitReq;
};

struct migReq
{
	long jobId;
	int options;
	uint numAskedHosts;
	char **askedHosts;
};

struct jobInfoReply
{
	u_short port;
	char padding1[2];
	int status;
	int reasons;
	int subreasons;
	int execUid;
	int exitStatus;
	int jType;
	int jobPriority;
	int *reasonTb;
	uint numReasons;
	uint numToHosts;
	uint nIdx;
	char padding2[4];
	time_t startTime;
	time_t predictedStartTime;
	time_t endTime;
	time_t reserveTime;
	time_t jRusageUpdateTime;
	char *userName;
	char *execHome;
	char *execCwd;
	char *execUsername;
	char *parentGroup;
	char *jName;
	float *loadSched;
	float *loadStop;
	float cpuTime;
	uid_t userId;
	unsigned long jobPid;
	struct submitReq *jobBill;
	struct jRusage runRusage;
	int counter[NUM_JGRP_COUNTERS]; // FIXME FIXME what is this NUM_JGRP_COUNTERS ? make it configurable from configure.ac
	long jobId; 					// FIXME FIXME FIXME this must change to u_int64_t ( unsigned long, 64 bits )
	char **toHosts;
};

struct signalReq
{
  int sigValue;
  int actFlags;
  time_t chkPeriod;
  long jobId;
};

struct groupInfoReply
{
  uint numGroups;
  char padding[4];
  struct groupInfoEnt *groups;
};

struct jobPeekReq
{
  long jobId;
};

struct infoReq
{
  int options;
  uint numNames;
  char **names;
  char *resReq;
};

struct queueInfoReply
{
  uint badQueue;
  uint numQueues;
  uint nIdx;
  char padding[4];
  struct queueInfoEnt *queues;
};


struct jobInfoReq
{
  long jobId;
  int options;
  char padding[4];
  char *userName;
  char *jobName;
  char *queue;
  char *host;
};


struct hostDataReply
{
  uint badHost;
  uint numHosts;
  uint nIdx;
  int flag;
  struct hostInfoEnt *hosts;
};

struct userInfoReply
{
  uint badUser;
  uint numUsers;
  struct userInfoEnt *users;
};


struct controlReq
{
  char *name;
  int opCode;
  char padding[4];
};

struct debugReq
{
  int opCode;
  int level;
  int logClass;
  int options;
  char *hostName;
  char *logFileName;
};

struct lsbShareResourceInfoReply
{
  uint numResources;
  uint badResource;
  struct lsbSharedResourceInfo *resources;
};

