#pragma once


////////////////////////////
// portability stuff
// 
// #ifdef __linux__
// #include <bits/termios.h>
// #endif 

#ifdef __APPLE__
#include <termios.h>
#endif

#ifndef VREPRINT
#define VREPRINT VRPRNT
#endif

#ifndef VDISCARD
#define VDISCARD VFLUSHO
#endif

#define LOBLK 0040000
#define VDSUSP  11
#define VSWTCH  7

// unsigned short IN_TABLE_SIZE   = 13;
// unsigned short OUT_TABLE_SIZE  = 30;
// unsigned short CTRL_TABLE_SIZE = 12;
// unsigned short LOC_TABLE_SIZE  = 10;
// unsigned short CHR_TABLE_SIZE  = 18;
// unsigned short CHR_TABLE_SPLIT = 12;
// unsigned short BAUD_TABLE_SIZE = 16;

enum comms { IN_TABLE_SIZE = 13, OUT_TABLE_SIZE = 30, CTRL_TABLE_SIZE = 12, LOC_TABLE_SIZE = 10, CHR_TABLE_SIZE = 18, CHR_TABLE_SPLIT = 12, BAUD_TABLE_SIZE = 16 };

// #define IN_TABLE_SIZE 13
tcflag_t in_table[IN_TABLE_SIZE] = { IGNBRK, BRKINT, IGNPAR, PARMRK, INPCK, ISTRIP, INLCR, IGNCR, ICRNL, 
#ifndef __MACH__ 
	IUCLC,
#endif
	IXON, IXANY, IXOFF
};

// #define OUT_TABLE_SIZE 30
tcflag_t out_table[OUT_TABLE_SIZE] = { OPOST, 
#ifndef __MACH__
	OLCUC,
#endif
	ONLCR, OCRNL, ONOCR, ONLRET, OFILL, OFDEL, NLDLY, NL0, NL1,
	CRDLY, CR0, CR1, CR2, CR3, TABDLY, TAB0, TAB1, TAB2, TAB3, 
	BSDLY, BS0, BS1, VTDLY, VT0, VT1, FFDLY, FF0, FF1
};

// #define CTRL_TABLE_SIZE 12

tcflag_t ctrl_table[CTRL_TABLE_SIZE] = {
	CLOCAL, CREAD, CSIZE, CS5, CS6, CS7, CS8, CSTOPB, HUPCL, PARENB, PARODD, LOBLK
};

// #define LOC_TABLE_SIZE 10
tcflag_t loc_table[LOC_TABLE_SIZE] = { ISIG, ICANON,
#ifndef __CYGWIN__ 
#ifndef __APPLE__	// FIXME FIXME refactor define labels once past basic OSes.
#ifndef __linux__
#if defined(_AIX) || defined(__ANDROID__) || defined(__amigaos__) || defined(__bg__)  || defined(__hiuxmpp) || defined(_hpux) || defined(___hpux) || defined(__OS400__) || defined(sgi)  || defined(FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(EPLAN9) || defined(__QNX__) || defined(M_XENIX) || defined(sun) || defined(__sysv__) || defined(ultrix) || defined(__MVS__) 
	XCASE,			// XCASE is not supported in GNU/Linux: see http://man7.org/linux/man-pages/man3/termios.3.html
#endif
#endif
#endif
#endif
	ECHO, ECHOE, ECHOK, ECHONL, NOFLSH, TOSTOP, IEXTEN
};

// #define CHR_TABLE_SIZE        18
// #define CHR_TABLE_SPLIT       12
int chr_table[CHR_TABLE_SIZE] =
	{ 	VINTR, VQUIT, VERASE, VKILL, VEOF, VEOL, VEOL2, VSWTCH, VSTART, VSTOP,
		VMIN, VTIME, VSUSP, VDSUSP, VREPRINT, VDISCARD, VWERASE, VLNEXT
	};

// #define BAUD_TABLE_SIZE 16
speed_t baud_table[BAUD_TABLE_SIZE] = { 
	B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400,
	B4800, B9600, B19200, B38400
};

///////////////////////////////////////////////////////////////
// 
// Function prototypes
//
int encodeTermios_ ( XDR *xdrs, struct termios *ptr_termios );
int decodeTermios_ ( XDR *xdrs, struct termios *ptr_termios );

int encode_mode    ( XDR *xdrs, tcflag_t mode_set,    tcflag_t *attr_table, unsigned int table_count );
int decode_mode    ( XDR *xdrs, tcflag_t *attr_table, unsigned int table_count,  tcflag_t *mode_set );
