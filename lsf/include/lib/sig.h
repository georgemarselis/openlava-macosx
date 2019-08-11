// Added by George Marselis <george@marsel.is> Wed April 3rd 2019

#pragma once 



// #define SIG_NULL             -65535
// #define SIG_CHKPNT               -1
// #define SIG_CHKPNT_COPY          -2
// #define SIG_DELETE_JOB           -3

// #define  SIG_SUSP_USER           -4
// #define  SIG_SUSP_LOAD           -5
// #define  SIG_SUSP_WINDOW         -6
// #define  SIG_SUSP_OTHER          -7

// #define  SIG_RESUME_USER         -8
// #define  SIG_RESUME_LOAD         -9
// #define  SIG_RESUME_WINDOW       -10
// #define  SIG_RESUME_OTHER        -11

// #define  SIG_TERM_USER           -12
// #define  SIG_TERM_LOAD           -13
// #define  SIG_TERM_WINDOW         -14
// #define  SIG_TERM_OTHER          -15
// #define  SIG_TERM_RUNLIMIT       -16
// #define  SIG_TERM_DEADLINE       -17
// #define  SIG_TERM_PROCESSLIMIT   -18
// #define  SIG_TERM_FORCE          -19
// #define  SIG_KILL_REQUEUE        -20
// #define  SIG_TERM_CPULIMIT       -21
// #define  SIG_TERM_MEMLIMIT       -22
// #define  SIG_ARRAY_REQUEUE       -23


// const int SIG_NULL              = -65535;
// const int SIG_CHKPNT            = -1;
// const int SIG_CHKPNT_COPY       = -2;
// const int SIG_DELETE_JOB        = -3;

// const int SIG_SUSP_USER         = -4;
// const int SIG_SUSP_LOAD         = -5;
// const int SIG_SUSP_WINDOW       = -6;
// const int SIG_SUSP_OTHER        = -7;

// const int SIG_RESUME_USER       = -8;
// const int SIG_RESUME_LOAD       = -9;
// const int SIG_RESUME_WINDOW     = -10;
// const int SIG_RESUME_OTHER      = -11;

// const int SIG_TERM_USER         = -12;
// const int SIG_TERM_LOAD         = -13;
// const int SIG_TERM_WINDOW       = -14;
// const int SIG_TERM_OTHER        = -15;
// const int SIG_TERM_RUNLIMIT     = -16;
// const int SIG_TERM_DEADLINE     = -17;
// const int SIG_TERM_PROCESSLIMIT = -18;
// const int SIG_TERM_FORCE        = -19;
// const int SIG_KILL_REQUEUE      = -20;
// const int SIG_TERM_CPULIMIT     = -21;
// const int SIG_TERM_MEMLIMIT     = -22;
// const int SIG_ARRAY_REQUEUE     = -23;

enum SIG_LSF {
    SIG_NULL              = 0,
    SIG_CHKPNT            = 1,
    SIG_CHKPNT_COPY       = 2,
    SIG_DELETE_JOB        = 3,

    SIG_SUSP_USER         = 4,
    SIG_SUSP_LOAD         = 5,
    SIG_SUSP_WINDOW       = 6,
    SIG_SUSP_OTHER        = 7,

    SIG_RESUME_USER       = 8,
    SIG_RESUME_LOAD       = 9,
    SIG_RESUME_WINDOW     = 10,
    SIG_RESUME_OTHER      = 11,

    SIG_TERM_USER         = 12,
    SIG_TERM_LOAD         = 13,
    SIG_TERM_WINDOW       = 14,
    SIG_TERM_OTHER        = 15,
    SIG_TERM_RUNLIMIT     = 16,
    SIG_TERM_DEADLINE     = 17,
    SIG_TERM_PROCESSLIMIT = 18,
    SIG_TERM_FORCE        = 19,
    SIG_KILL_REQUEUE      = 20,
    SIG_TERM_CPULIMIT     = 21,
    SIG_TERM_MEMLIMIT     = 22,
    SIG_ARRAY_REQUEUE     = 23
};


// #define SIGEMT  SIGBUS
// #define SIGLOST SIGIO
// #define SIGIOT  SIGABRT

#if !defined(SIGWINCH) && defined(SIGWINDOW)
#    define SIGWINCH SIGWINDOW
#endif

#undef SIGSYS // https://www.ibiblio.org/oswg/oswg-nightly/oswg/en_GB.ISO_8859-1/books/linux-c-programming/GCC-HOWTO.html#INDEX.34

static unsigned int sig_map[] = { 0,
    SIGHUP,
    SIGINT,
    SIGQUIT,
    SIGILL,
    SIGTRAP,
    SIGIOT,
#ifdef SIGSYS // https://www.ibiblio.org/oswg/oswg-nightly/oswg/en_GB.ISO_8859-1/books/linux-c-programming/GCC-HOWTO.html#INDEX.34
    SIGEMT,
#endif
    SIGFPE,
    SIGKILL,
    SIGBUS,
    SIGSEGV,
    // SIGSYS,
    SIGPIPE,
    SIGALRM,
    SIGTERM,
    SIGSTOP,
    SIGTSTP,
    SIGCONT,
    SIGCHLD,
    SIGTTIN,
    SIGTTOU,
    SIGIO,
    SIGXCPU,
    SIGXFSZ,
    SIGVTALRM,
    SIGPROF,
    SIGWINCH,
    #ifdef __sparc__
    SIGLOST,
    #endif
    SIGUSR1,
    SIGUSR2
};

const char *sigSymbol[] = { 
    "",
    "HUP",
    "INT",
    "QUIT",
    "ILL",
    "TRAP",
    "IOT",
#ifdef SIGSYS // https://www.ibiblio.org/oswg/oswg-nightly/oswg/en_GB.ISO_8859-1/books/linux-c-programming/GCC-HOWTO.html#INDEX.34
    "EMT",
#endif
    "FPE",
    "KILL",
    "BUS",
    "SEGV",
    // "SYS",
    "PIPE",
    "ALRM",
    "TERM",
    "STOP",
    "TSTP",
    "CONT",
    "CHLD",
    "TTIN",
    "TTOU",
    "IO",
    "XCPU",
    "XFSZ",
    "VTALRM",
    "PROF",
    "WINCH",
    "LOST",
    "USR1",
    "USR2"
};

unsigned int NSIG_MAP = (sizeof (sig_map) / sizeof ( unsigned int)); // FIXME FIXME FIXME move this out to appropriate header

/* sig.c */
unsigned int sig_encode(unsigned int sig);
unsigned int sig_decode(unsigned int sig);
unsigned int getSigVal( const char *sigString);
char *getSigSymbolList(void);
SIGFUNCTYPE Signal_(unsigned int sig, void (*handler )(int ));
char *getSigSymbol(unsigned int sig);
int blockALL_SIGS_(sigset_t *newMask, sigset_t *oldMask);
