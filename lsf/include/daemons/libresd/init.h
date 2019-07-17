// added by George Marselis <george@marsel.is>

#pragma once

// #define RES_TIMEOUT_DEFAULT     60
// #define LOOP_ADDR       0x7F000001 // FIXME FIXME FIXME FIXME why is this here?

struct ttyStruct defaultTty;


enum RES_TIMEOUT {
    RES_TIMEOUT = 60,
} RES_TIMEOUT;

enum LOOP_ADDR {
    LOOP_ADDR = 0x7F000001
};

enum RES_TIMEOUT_DEFAULT {
    RES_TIMEOUT_DEFAULT = 60
} RES_TIMEOUT_DEFAULT;

/* daemons/resd/init.c */
void initConn2NIOS(void);
void init_res(void);
void init_AcceptSock(void);
void initChildRes(const char *envdir);
int resParent(int s, struct passwd *pw, struct lsfAuth *auth, struct resConnect *connReq, struct hostent *hostp);
void resChild(const char *arg, const char *envdir);
