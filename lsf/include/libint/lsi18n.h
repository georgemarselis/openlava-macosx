/* $Id: lsi18n.h 397 2007-11-26 19:04:00Z mblack $
 * Copyright (C) 2007 Platform Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#pragma once

#include <sys/types.h>


// #define LS_CATD int

// #define I18N_CAT_MIN        1
// #define I18N_CAT_LIM        1
// #define I18N_CAT_PIM        2
// #define I18N_CAT_RES        3
// #define I18N_CAT_MBD        4
// #define I18N_CAT_SBD        5
// #define I18N_CAT_CMD        6
// #define I18N_CAT_MAX        6

enum I18N_CAT {
	I18N_CAT_MIN = 1,
	I18N_CAT_LIM = 1,
	I18N_CAT_PIM = 2,
	I18N_CAT_RES = 3,
	I18N_CAT_MBD = 4,
	I18N_CAT_SBD = 5,
	I18N_CAT_CMD = 6,
	I18N_CAT_MAX = 6
};

// #define MOD_LSBATCH         4
// #define MOD_LSB_BACC        5
// #define MOD_LSB_BHIST       6
// #define MOD_LSB_BSTATS      7
// #define MOD_LSB_CMD         8
// #define MOD_LSBD_MBD        10
// #define MOD_LSBD_SBD        11
// #define MOD_LSBD_MISC       12
// #define MOD_LSB_LIB         13
// #define MOD_LSF             21
// #define MOD_LSF_INTLIB      22
// #define MOD_LSF_LIB         23
// #define MOD_LSF_LIM         24
// #define MOD_LSF_LSADM       25
// #define MOD_LSF_LSTOOLS     27
// #define MOD_LSF_PIM         28
// #define MOD_LSF_RES         29
// #define MOD_MISC            33
// #define MOD_TIME_FORMAT     35

enum MOD {
	MOD_LSBATCH     = 4,
	MOD_LSB_BACC    = 5,
	MOD_LSB_BHIST   = 6,
	MOD_LSB_BSTATS  = 7,
	MOD_LSB_CMD     = 8,
	MOD_LSBD_MBD    = 10,
	MOD_LSBD_SBD    = 11,
	MOD_LSBD_MISC   = 12,
	MOD_LSB_LIB     = 13,
	MOD_LSF         = 21,
	MOD_LSF_INTLIB  = 22,
	MOD_LSF_LIB     = 23,
	MOD_LSF_LIM     = 24,
	MOD_LSF_LSADM   = 25,
	MOD_LSF_LSTOOLS = 27,
	MOD_LSF_PIM     = 28,
	MOD_LSF_RES     = 29,
	MOD_MISC        = 33,
	MOD_TIME_FORMAT = 35
};

// #define MAX_I18N_CTIME_STRING   80
// #define MIN_CTIME_FORMATID      0
// #define CTIME_FORMAT_DEFAULT    0
// #define CTIME_FORMAT_a_b_d_T_Y  1
// #define CTIME_FORMAT_b_d_T_Y    2
// #define CTIME_FORMAT_a_b_d_T    3
// #define CTIME_FORMAT_b_d_H_M    4
// #define CTIME_FORMAT_m_d_Y      5
// #define CTIME_FORMAT_H_M_S      6
// #define MAX_CTIME_FORMATID      6

enum CTIME_ENUM {
	MAX_I18N_CTIME_STRING  = 80,
	MIN_CTIME_FORMATID     = 0,
	CTIME_FORMAT_DEFAULT   = 0,
	CTIME_FORMAT_a_b_d_T_Y = 1,
	CTIME_FORMAT_b_d_T_Y   = 2,
	CTIME_FORMAT_a_b_d_T   = 3,
	CTIME_FORMAT_b_d_H_M   = 4,
	CTIME_FORMAT_m_d_Y     = 5,
	CTIME_FORMAT_H_M_S     = 6,
	MAX_CTIME_FORMATID     = 6
};

// #define I18N_CATFILE        "lsf"
// static const char I18N_CATFILE[] = "lsf";

// #ifdef NL_SETN
// #error
// #endif

// typedef int ls_catd; // FIXME FIXME FIXME FIXME FIXME remove.

// #define I18N(msgID, msg)   (_i18n_msg_get(ls_catd, NL_SETN, msgID, msg))

// #define I18N_m(msgID, msg) (_i18n_msg_get(ls_catd, 33, msgID, msg))

// #ifdef  I18N_COMPILE
// #else
// #define  _i18n_msg_get(catd,setID,msgID,msgStr)  (msgStr)
// #define  _i18n_msgArray_get(catd,setID,msgID_Array,msgArray) (msgArray)
// #endif


int    _i18n_end         ( void );
char  *_i18n_ctime       ( int, int, const time_t * );
char  *_i18n_msg_get     ( int, int typeofError, int catgetsNumber, char *errorMessage );
char **_i18n_msgArray_get( int, int, int *, char ** );
void   _i18n_ctime_init  ( int );

int   _i18n_init   ( int );
char *_i18n_printf (const char *, ...);




#define I18N_FUNC_FAIL              ("catgets 1: %s: %s() failed.")                                 /*catgets1 */
#define I18N_FUNC_FAIL_M            ("catgets 2: %s: %s() failed, %m.")                             /*catgets2 */
#define I18N_FUNC_FAIL_MM           ("catgets 3: %s: %s() failed, %M.")                             /*catgets3 */
#define I18N_FUNC_FAIL_S            ("catgets 4: %s: %s() failed, %s.")                             /*catgets4 */
#define I18N_FUNC_S_FAIL            ("catgets 5: %s: %s(%s) failed.")                               /*catgets5 */
#define I18N_FUNC_S_FAIL_M          ("catgets 6: %s: %s(%s) failed, %m.")                           /*catgets6 */
#define I18N_FUNC_S_FAIL_MM         ("catgets 7: %s: %s(%s) failed, %M.")                           /*catgets7 */
#define I18N_FUNC_D_FAIL            ("catgets 8: %s: %s(%d) failed.")                               /*catgets8 */
#define I18N_FUNC_D_FAIL_M          ("catgets 9: %s: %s(%d) failed, %m.")                           /*catgets9 */
#define I18N_FUNC_D_FAIL_MM         ("catgets 10: %s: %s(%d) failed, %M.")                          /*catgets10 */
#define I18N_JOB_FAIL_S             ("catgets 11: %s: Job <%s> failed in %s().")                    /*catgets11 */
#define I18N_JOB_FAIL_M             ("catgets 12: %s: Job <%s> failed, %m.")                        /*catgets12 */
#define I18N_JOB_FAIL_MM            ("catgets 13: %s: Job <%s> failed, %M.")                        /*catgets13 */
#define I18N_QUEUE_FAIL             ("catgets 14: %s: Queue <%s> failed in %s().")                  /*catgets14 */
#define I18N_HOST_FAIL              ("catgets 15: %s: Host <%s> failed in %s().")                   /*catgets15 */
#define I18N_CALENDAR_FAIL          ("catgets 16: %s: Calendar <%s> failed in %s().")               /*catgets16 */
#define I18N_GROUP_FAIL             ("catgets 17: %s: Group <%s> failed in %s().")                  /*catgets17 */
#define I18N_ERROR                  ("catgets 18: %s: Error in %s().")                              /*catgets18 */
#define I18N_ERROR_LD               ("catgets 19: %s error %ld.")                                   /*catgets19 */
#define I18N_S_ERROR_LD             ("catgets 20: %s: %s error %ld.")                               /*catgets20 */
#define I18N_CANNOT_OPEN            ("catgets 21: %s: Cannot open '%s' %m.")                        /*catgets21 */
#define I18N_FUNC_FAIL_ENO_D        ("catgets 22: %s: %s() failed, errno= %d.")                     /*catgets22 */
#define I18N_CLOSED_S               ("catgets 23: Closed %s.")                                      /*catgets23 */
#define I18N_OPEN_S                 ("catgets 24: Open %s.")                                        /*catgets24 */
#define I18N_SHOW_S                 ("catgets 25: Show %s.")                                        /*catgets25 */
#define I18N_LSADMIN_S              ("catgets 26: lsadmin %s.")                                     /*catgets26 */
#define I18N_BADMIN_S               ("catgets 27: badmin %s.")                                      /*catgets27 */
#define I18N_START_S                ("catgets 28: Startup %s.")                                     /*catgets28 */
#define I18N_RESTART_S              ("catgets 29: Restart %s.")                                     /*catgets29 */
#define I18N_SHUTDOWN_S             ("catgets 30: Shutdown %s.")                                    /*catgets30 */
#define I18N_FUNC_FAIL_EMSG_S       ("catgets 31: %s: %s() failed, errmsg: %s.")                    /*catgets31 */
#define I18N_FUNC_S_FAIL_EMSG_S     ("catgets 32: %s: %s(%s) failed, errmsg: %s.")                  /*catgets32 */
#define I18N_PREMATURE_EOF          ("catgets 33: %s: %s(%d): Premature EOF in section %s")         /*catgets33 */
#define I18N_FUNC_D_FAIL_S          ("catgets 34: %s: %s(%d) failed, %s.")                          /*catgets34 */
#define I18N_FUNC_S_S_FAIL_M        ("catgets 35: %s: %s(%s, %s) failed, %m.")                      /*catgets35 */
#define I18N_FUNC_S_D_FAIL_M        ("catgets 36: %s: %s(%s, %d) failed, %m.")                      /*catgets36 */
#define I18N_FUNC_S_S_FAIL_S        ("catgets 37: %s(%s) failed: %s.")                              /*catgets37 */
#define I18N_FUNC_FAILED            ("catgets 38: %s failed.")                                      /*catgets38 */
#define I18N_FUNC_D_D_FAIL_M        ("catgets 39: %s: %s(%d, %d) failed, %m.")                      /*catgets39 */
#define I18N_HORI_NOT_IMPLE         ("catgets 40: %s: File %s at line %d: Horizontal %s section not implemented yet; use vertical format; section ignored") /*catgets40 */
#define I18N_JOB_FAIL_S_S_M         ("catgets 41: %s: Job <%s> failed in %s(%s), %m.")              /*catgets41 */
#define I18N_JOB_FAIL_S_M           ("catgets 42: %s: Job <%s> failed in %s(), %m.")                /*catgets42 */
#define I18N_FUNC_S_ERROR           ("catgets 43: %s Error !")                                      /*catgets43 */
#define I18N_JOB_FAIL_S_D_M         ("catgets 44: %s: Job <%s> failed in %s(%d), %m.")              /*catgets44 */
#define I18N_JOB_FAIL_S_MM          ("catgets 45: %s: Job <%s> failed in %s(), %M.")                /*catgets45 */
#define I18N_JOB_FAIL_S_S_MM        ("catgets 46: %s: Job <%s> failed in %s(%s), %M.")              /*catgets46 */
#define I18N_JOB_FAIL_S_S           ("catgets 47: %s: Job <%s> failed in %s(%s).")                  /*catgets47 */
#define I18N_FUNC_S_S_FAIL          ("catgets 48: %s: %s(%s, %s) failed.")                          /*catgets48 */
#define I18N_PARAM_NEXIST           ("catgets 49: %s: parameter '%s' does not exist in '%s'.")      /*catgets49 */
#define I18N_NO_MEMORY              ("catgets 50: No enough memory.\n")                             /*catgets50 */
#define I18N_NOT_ROOT               ("catgets 51: %s: Not root, cannot collect kernal info")        /*catgets51 */
#define I18N_NEG_RUNQ               ("catgets 52: %s: negative run queue length: %f")               /*catgets52 */
#define I18N_FUNC_FAIL_NO_PERIOD    ("catgets 53: %s: %s() failed")                                 /*catgets53 */
#define I18N_FUNC_S_FAIL_MN         ("catgets 54: %s: %s(%s) failed, %k.")                          /*catgets54 */
#define I18N_FUNC_FAIL_NN           ("catgets 55: %s: %s() failed, %N.")                            /*catgets55 */
#define I18N_FUNC_FAIL_MN           ("catgets 56: %s: %s() failed, %k.")                            /*catgets56 */

#define I18N_All                    ("catgets 1000: All")                                           /*catgets1000 */
#define I18N_Array__Name            ("catgets 1001: Array_Name")                                    /*catgets1001 */
#define I18N_Apply                  ("catgets 1002: Apply")                                         /*catgets1002 */
#define I18N_activated              ("catgets 1003: activated")                                     /*catgets1003 */
#define I18N_active                 ("catgets 1004: active")                                        /*catgets1004 */
#define I18N_ATTRIBUTE              ("catgets 1005: ATTRIBUTE")                                     /*catgets1005 */
#define I18N_Advanced               ("catgets 1006: Advanced")                                      /*catgets1006 */
#define I18N_ACKNOWLEDGED           ("catgets 1007: ACKNOWLEDGED")                                  /*catgets1007 */
#define I18N_Add                    ("catgets 1008: Add")                                           /*catgets1008 */
#define I18N_Any                    ("catgets 1009: Any")                                           /*catgets1009 */
#define I18N_All_Hosts              ("catgets 1010: All Hosts")                                     /*catgets1010 */
#define I18N_Active                 ("catgets 1011: Active")                                        /*catgets1011 */
#define I18N_Analyzer               ("catgets 1012: Analyzer")                                      /*catgets1012 */

#define I18N_BEGIN                  ("catgets 1200: BEGIN")                                         /*catgets1200 */
#define I18N_Bottom                 ("catgets 1201: Bottom")                                        /*catgets1201 */
#define I18N_Browse                 ("catgets 1202: Browse")                                        /*catgets1202 */
#define I18N_Brief                  ("catgets 1203: Brief")                                         /*catgets1203 */
#define I18N_Backfilling            ("catgets 1204: Backfilling")                                   /*catgets1204 */
#define I18N_Boolean                ("catgets 1205: Boolean")                                       /*catgets1205 */
#define I18N_Batch                  ("catgets 1206: Batch")                                         /*catgets1206 */
#define I18N_busy                   ("catgets 1207: lock")                                          /*catgets1207 */
#define I18N_minus                  ("catgets 1213: -")                                             /*catgets1213 */
#define I18N_semibusy               ("catgets 1214: -busy")                                         /*catgets1214 */

#define I18N_Close                  ("catgets 1400: Close")                                         /*catgets1400 */
#define I18N_Contents               ("catgets 1401: Contents")                                      /*catgets1401 */
#define I18N_Checkpoint             ("catgets 1402: Checkpoint")                                    /*catgets1402 */
#define I18N_Command                ("catgets 1403: Command")                                       /*catgets1403 */
#define I18N_Cancel                 ("catgets 1404: Cancel")                                        /*catgets1404 */
#define I18N_Choose                 ("catgets 1405: Choose")                                        /*catgets1405 */
#define I18N_Clean                  ("catgets 1406: Clean")                                         /*catgets1406 */
#define I18N_closed                 ("catgets 1407: closed")                                        /*catgets1407 */
#define I18N_Conditions             ("catgets 1408: Conditions")                                    /*catgets1408 */
#define I18N_Calendar               ("catgets 1409: Calendar")                                      /*catgets1409 */
#define I18N_Check                  ("catgets 1410: Check")                                         /*catgets1410 */
#define I18N_Commit                 ("catgets 1411: Commit")                                        /*catgets1411 */
#define I18N_Clear                  ("catgets 1412: Clear")                                         /*catgets1412 */
#define I18N_Checkpointable         ("catgets 1413: Checkpointable")                                /*catgets1413 */
#define I18N_Cluster                ("catgets 1414: Cluster")                                       /*catgets1414 */
#define I18N_Client                 ("catgets 1415: Client")                                        /*catgets1415 */
#define I18N_CPU_Limit              ("catgets 1416: CPU Limit")                                     /*catgets1416 */
#define I18N_Core_Limit             ("catgets 1417: Core Limit")                                    /*catgets1417 */
#define I18N_Copy_From              ("catgets 1418: Copy From")                                     /*catgets1418 */
#define I18N_Configure              ("catgets 1419: Configure")                                     /*catgets1419 */
#define I18N_characters             ("catgets 1420: characters")                                    /*catgets1420 */
#define I18N_CPUF                   ("catgets 1421: CPUF")                                          /*catgets1421 */
#define I18N_Closed                 ("catgets 1422: Closed")                                        /*catgets1422 */
#define I18N_client                 ("catgets 1423: client")                                        /*catgets1423 */

#define I18N_Done                   ("catgets 1600: Done")                                          /*catgets1600 */
#define I18N_DISABLED               ("catgets 1601: DISABLED")                                      /*catgets1601 */
#define I18N_Detail                 ("catgets 1602: Detail")                                        /*catgets1602 */
#define I18N_DONE                   ("catgets 1603: DONE")                                          /*catgets1603 */
#define I18N_Delete                 ("catgets 1604: Delete")                                        /*catgets1604 */
#define I18N_Default                ("catgets 1605: Default")                                       /*catgets1605 */
#define I18N_Day                    ("catgets 1606: Day")                                           /*catgets1606 */
#define I18N_Duration               ("catgets 1607: Duration")                                      /*catgets1607 */
#define I18N_Dependency             ("catgets 1608: Dependency")                                    /*catgets1608 */
#define I18N_Defaults               ("catgets 1609: Defaults")                                      /*catgets1609 */
#define I18N_Description            ("catgets 1610: Description")                                   /*catgets1610 */
#define I18N_Data_Limit             ("catgets 1611: Data Limit")                                    /*catgets1611 */
#define I18N_Dynamic                ("catgets 1612: Dynamic")                                       /*catgets1612 */
#define I18N_Decreasing             ("catgets 1613: Decreasing")                                    /*catgets1613 */
#define I18N_Directories            ("catgets 1614: Directories")                                   /*catgets1614 */
#define I18N_done                   ("catgets 1615: done")                                          /*catgets1615 */

#define I18N_Exiting                ("catgets 1800: Exiting")                                       /*catgets1800 */
#define I18N_Exited                 ("catgets 1801: Exited")                                        /*catgets1801 */
#define I18N_error                  ("catgets 1802: error")                                         /*catgets1802 */
#define I18N_ENABLED                ("catgets 1803: ENABLED")                                       /*catgets1803 */
#define I18N_END                    ("catgets 1804: END")                                           /*catgets1804 */
#define I18N_Exit                   ("catgets 1805: Exit")                                          /*catgets1805 */
#define I18N_EXIT                   ("catgets 1806: EXIT")                                          /*catgets1806 */
#define I18N_Exec__Host             ("catgets 1807: Exec_Host")                                     /*catgets1807 */
#define I18N_EVENT                  ("catgets 1808: EVENT")                                         /*catgets1808 */
#define I18N_except                 ("catgets 1809: except")                                        /*catgets1809 */
#define I18N_exit                   ("catgets 1810: exit")                                          /*catgets1810 */
#define I18N_EVENTS                 ("catgets 1811: EVENTS")                                        /*catgets1811 */
#define I18N_EXPIRED                ("catgets 1812: EXPIRED")                                       /*catgets1812 */
#define I18N_Edit                   ("catgets 1813: Edit")                                          /*catgets1813 */
#define I18N_Error_file             ("catgets 1814: Error file")                                    /*catgets1814 */
#define I18N_Exclusive_Job          ("catgets 1815: Exclusive Job")                                 /*catgets1815 */
#define I18N_Event                  ("catgets 1816: Event")                                         /*catgets1816 */
#define I18N_Exception              ("catgets 1817: Exception")                                     /*catgets1817 */

#ifdef MOTOROLA_XLSBATCH_ENH
    #define I18N_Ext__Msg           ("catgets 1818: External Message")                                                          /*catgets1818 */
    #define I18N_Err__Option        ("catgets 1819: Please select at least one job information source!")                        /*catgets1819 */
    #define I18N_Err__Days          ("catgets 1820: Number of day entered is not valid!\nPlease enter a non-negative integer.") /*catgets1820 */
#endif

#define I18N_From                   ("catgets 2000: From")                                          /*catgets2000 */
#define I18N_File                   ("catgets 2001: File")                                          /*catgets2001 */
#define I18N_Filter                 ("catgets 2002: Filter")                                        /*catgets2002 */
#define I18N_From__Host             ("catgets 2003: From_Host")                                     /*catgets2003 */
#define I18N_Finish__Time           ("catgets 2004: Finish_Time")                                   /*catgets2004 */
#define I18N_file                   ("catgets 2005: file")                                          /*catgets2005 */
#define I18N_From_file              ("catgets 2006: From file")                                     /*catgets2006 */
#define I18N_File_Limit             ("catgets 2007: File Limit")                                    /*catgets2007 */
#define I18N_Fairshare              ("catgets 2008: Fairshare")                                     /*catgets2008 */
#define I18N_Files                  ("catgets 2009: Files")                                         /*catgets2009 */

#define I18N_Group_Name             ("catgets 2200: Group Name")                                    /*catgets2200 */
#define I18N_Go_To_Directory        ("catgets 2201: Go To Directory")                               /*catgets2201 */

#define I18N_Help                   ("catgets 2400: Help")                                          /*catgets2400 */
#define I18N_History                ("catgets 2401: History")                                       /*catgets2401 */
#define I18N_Host                   ("catgets 2402: Host")                                          /*catgets2402 */
#define I18N_HOSTS                  ("catgets 2403: HOSTS")                                         /*catgets2403 */
#define I18N_Hour                   ("catgets 2404: Hour")                                          /*catgets2404 */
#define I18N_Hosts                  ("catgets 2405: Hosts")                                         /*catgets2405 */
#define I18N_Hold                   ("catgets 2406: Hold")                                          /*catgets2406 */
#define I18N_hours                  ("catgets 2407: hours")                                         /*catgets2407 */
#define I18N_Host_Types             ("catgets 2408: Host Types")                                    /*catgets2408 */
#define I18N_Host_Models            ("catgets 2409: Host Models")                                   /*catgets2409 */
#define I18N_Host_Names             ("catgets 2410: Host Names")                                    /*catgets2410 */
#define I18N_Host_Spec              ("catgets 2411: Host Spec")                                     /*catgets2411 */
#define I18N_Host_Type              ("catgets 2412: Host Type")                                     /*catgets2412 */
#define I18N_Host_Model             ("catgets 2413: Host Model")                                    /*catgets2413 */
#define I18N_Host_Name              ("catgets 2414: Host Name")                                     /*catgets2414 */
#define I18N_Host_Group             ("catgets 2415: Host Group")                                    /*catgets2415 */
#define I18N_Host_Partition         ("catgets 2416: Host Partition")                                /*catgets2416 */
#define I18N_HOST                   ("catgets 2417: HOST")                                          /*catgets2417 */
#define I18N_Host__Name             ("catgets 2418: Host_Name")                                     /*catgets2418 */

#define I18N_info                   ("catgets 2600: info")                                          /*catgets2600 */
#define I18N_Information            ("catgets 2601: Information")                                   /*catgets2601 */
#define I18N_inactivated            ("catgets 2602: inactivated")                                   /*catgets2602 */
#define I18N_inactive               ("catgets 2603: inactive")                                      /*catgets2603 */
#define I18N_invalid                ("catgets 2604: invalid")                                       /*catgets2604 */
#define I18N_Insert                 ("catgets 2605: Insert")                                        /*catgets2605 */
#define I18N_ID                     ("catgets 2606: ID")                                            /*catgets2606 */
#define I18N_Info                   ("catgets 2607: Info")                                          /*catgets2607 */
#define I18N_Input_file             ("catgets 2608: Input file")                                    /*catgets2608 */
#define I18N_Increasing             ("catgets 2609: Increasing")                                    /*catgets2609 */
#define I18N_Interactive            ("catgets 2610: Interactive")                                   /*catgets2610 */
#define I18N_Inactive               ("catgets 2611: Inactive")                                      /*catgets2611 */
#define I18N_Inact                  ("catgets 2612: Inact")                                         /*catgets2612 */
#define I18N_Inact__Adm             ("catgets 2613: Inact_Adm")                                     /*catgets2613 */
#define I18N_Inact__Win             ("catgets 2614: Inact_Win")                                     /*catgets2614 */

#define I18N_Job                    ("catgets 2800: Job")                                           /*catgets2800 */
#define I18N_JobArray               ("catgets 2801: JobArray")                                      /*catgets2801 */
#define I18N_Job__Id                ("catgets 2802: Job_Id")                                        /*catgets2802 */
#define I18N_Job__PID               ("catgets 2803: Job_PID")                                       /*catgets2803 */
#define I18N_Job__Name              ("catgets 2804: Job_Name")                                      /*catgets2804 */
#define I18N_JobGroup               ("catgets 2805: Job Group")                                     /*catgets2805 */
#define I18N_Job_ID                 ("catgets 2806: Job ID")                                        /*catgets2806 */
#define I18N_Job_Name               ("catgets 2807: Job Name")                                      /*catgets2807 */
#define I18N_Job_Dependency         ("catgets 2808: Job Dependency")                                /*catgets2808 */
#define I18N_JL_U                   ("catgets 2809: JL/U")                                          /*catgets2809 */
#define I18N_JL_P                   ("catgets 2810: JL/P")                                          /*catgets2810 */
#define I18N_JL_H                   ("catgets 2811: JL/H")                                          /*catgets2811 */
#define I18N_JOBID                  ("catgets 2812: JOBID")                                         /*catgets2812 */
#define I18N_JOB__NAME              ("catgets 2813: JOB_NAME")                                      /*catgets2813 */
#define I18N_Job_Priority           ("catgets 2814: Job Priority")                                  /*catgets2814 */
#define I18N_JobScheduler           ("catgets 2815: JobScheduler")                                  /*catgets2815 */

#ifdef MOTOROLA_XLSBATCH_ENH
    #define I18N_Job__Source            ("catgets 2816: Job Information Source")                    /*catgets2816 */
    #define I18N_Job__Source_Current    ("catgets 2817: Current")                                   /*catgets2817 */
    #define I18N_Job__Source_History    ("catgets 2818: Historical")                                /*catgets2818 */
    #define I18N_Job__Source_Label      ("catgets 2819: Show jobs for the last")                    /*catgets2819 */
    #define I18N_Job__Source_Days       ("catgets 2820: days")                                      /*catgets2820 */
#endif

#define I18N_killed                 ("catgets 3000: killed")                                        /*catgets3000 */
#define I18N_KBytes                 ("catgets 3001: KBytes")                                        /*catgets3001 */

#define I18N_Limits                 ("catgets 3200: Limits")                                        /*catgets3200 */
#define I18N_Load                   ("catgets 3201: Load")                                          /*catgets3201 */
#define I18N_LSF_Batch              ("catgets 3202: LSF Batch")                                     /*catgets3202 */
#define I18N_LSF_Administrators     ("catgets 3203: LSF Administrators")                            /*catgets3203 */
#define I18N_Local                  ("catgets 3204: Local")                                         /*catgets3204 */
#define I18N_Load_Files             ("catgets 3205: Load Files")                                    /*catgets3205 */
#define I18N_lock                   ("catgets 3206: lock")                                          /*catgets3206 */
#define I18N_lockUW                 ("catgets 3207: -lockUW")                                       /*catgets3207 */
#define I18N_lockU                  ("catgets 3208: -lockU")                                        /*catgets3208 */
#define I18N_lockW                  ("catgets 3209: -lockW")                                        /*catgets3209 */
#define I18N_lockM                  ("catgets 3211: -lockM")                                        /*catgets3211 */
#define I18N_lockUM                 ("catgets 3212: -lockUM")                                       /*catgets3212 */
#define I18N_lockWM                 ("catgets 3213: -lockWM")                                       /*catgets3213 */
#define I18N_lockUWM                ("catgets 3214: -lockUWM")                                      /*catgets3214 */
#define I18N_LSF                    ("catgets 3210: LSF")                                           /*catgets3210 */

#define I18N_Mail                   ("catgets 3400: Mail")                                          /*catgets3400 */
#define I18N_more                   ("catgets 3401: more")                                          /*catgets3401 */
#define I18N_min                    ("catgets 3402: min")                                           /*catgets3402 */
#define I18N_Manipulate             ("catgets 3403: Manipulate")                                    /*catgets3403 */
#define I18N_Migrate                ("catgets 3404: Migrate")                                       /*catgets3404 */
#define I18N_Modify                 ("catgets 3405: Modify")                                        /*catgets3405 */
#define I18N_Month                  ("catgets 3406: Month")                                         /*catgets3406 */
#define I18N_Min                    ("catgets 3407: Min")                                           /*catgets3407 */
#define I18N_minutes                ("catgets 3408: minutes")                                       /*catgets3408 */
#define I18N_Max                    ("catgets 3409: Max")                                           /*catgets3409 */
#define I18N_Minutes                ("catgets 3410: Minutes")                                       /*catgets3410 */
#define I18N_Memory_Limit           ("catgets 3411: Memory Limit")                                  /*catgets3411 */
#define I18N_Manage                 ("catgets 3412: Manage")                                        /*catgets3412 */
#define I18N_More                   ("catgets 3413: More")                                          /*catgets3413 */
#define I18N_MAX                    ("catgets 3414: MAX")                                           /*catgets3414 */

#define I18N_Name                   ("catgets 3600: Name")                                          /*catgets3600 */
#define I18N_NONE                   ("catgets 3601: NONE")                                          /*catgets3601 */
#define I18N_No                     ("catgets 3602: No")                                            /*catgets3602 */
#define I18N_NJOBS                  ("catgets 3603: NJOBS")                                         /*catgets3603 */
#define I18N_None                   ("catgets 3604: None")                                          /*catgets3604 */
#define I18N_NAME                   ("catgets 3605: NAME")                                          /*catgets3605 */
#define I18N_New                    ("catgets 3606: New")                                           /*catgets3606 */
#define I18N_Next                   ("catgets 3607: Next")                                          /*catgets3607 */
#define I18N_NICE                   ("catgets 3608: NICE")                                          /*catgets3608 */
#define I18N_no_limit               ("catgets 3609: no limit")                                      /*catgets3609 */
#define I18N_Numeric                ("catgets 3610: Numeric")                                       /*catgets3610 */
#define I18N_Nonrelease             ("catgets 3611: Nonrelease")                                    /*catgets3611 */

#define I18N_OK                     ("catgets 3800: OK")                                            /*catgets3800 */
#define I18N_or                     ("catgets 3801: or")                                            /*catgets3801 */
#define I18N_OWNER                  ("catgets 3802: OWNER")                                         /*catgets3802 */
#define I18N_Options                ("catgets 3803: Options")                                       /*catgets3803 */
#define I18N_ok                     ("catgets 3804: ok")                                            /*catgets3804 */
#define I18N_opened                 ("catgets 3805: opened")                                        /*catgets3805 */
#define I18N_OPEN                   ("catgets 3806: OPEN")                                          /*catgets3806 */
#define I18N_Output_file            ("catgets 3807: Output file")                                   /*catgets3807 */
#define I18N_Only                   ("catgets 3808: Only")                                          /*catgets3808 */
#define I18N_open                   ("catgets 3809: open")                                          /*catgets3809 */
#define I18N_Open                   ("catgets 3810: Open")                                          /*catgets3810 */
#define I18N_semiok                 ("catgets 3811: -ok")                                           /*catgets3811 */

#define I18N_Parameter              ("catgets 4000: Parameter")                                     /*catgets4000 */
#define I18N_Peek                   ("catgets 4001: Peek")                                          /*catgets4001 */
#define I18N_Print                  ("catgets 4002: Print")                                         /*catgets4002 */
#define I18N_PEND                   ("catgets 4003: PEND")                                          /*catgets4003 */
#define I18N_PSUSP                  ("catgets 4004: PSUSP")                                         /*catgets4004 */
#define I18N_Previous               ("catgets 4005: Previous")                                      /*catgets4005 */
#define I18N_Priority               ("catgets 4006: Priority")                                      /*catgets4006 */
#define I18N_Processor_Limit        ("catgets 4009: Processor Limit")                               /*catgets4009 */
#define I18N_parameters             ("catgets 4010: parameters")                                    /*catgets4010 */
#define I18N_PRIO                   ("catgets 4011: PRIO")                                          /*catgets4011 */
#define I18N_Process_Limit          ("catgets 4012: Process Limit")                                 /*catgets4012 */
#define I18N_pending                ("catgets 4013: pending")                                       /*catgets4013 */

#define I18N_Queue                  ("catgets 4200: Queue")                                         /*catgets4200 */
#define I18N_QUEUE                  ("catgets 4201: QUEUE")                                         /*catgets4201 */
#define I18N_Queue__Name            ("catgets 4202: Queue_Name")                                    /*catgets4202 */
#define I18N_QUEUE__NAME            ("catgets 4203: QUEUE_NAME")                                    /*catgets4203 */

#define I18N_Request                ("catgets 4400: Request")                                       /*catgets4400 */
#define I18N_resumed                ("catgets 4401: resumed")                                       /*catgets4401 */
#define I18N_RUNNING                ("catgets 4402: RUNNING")                                       /*catgets4402 */
#define I18N_Resume                 ("catgets 4403: Resume")                                        /*catgets4403 */
#define I18N_RUN                    ("catgets 4404: RUN")                                           /*catgets4404 */
#define I18N_Restart                ("catgets 4405: Restart")                                       /*catgets4405 */
#define I18N_Revert                 ("catgets 4406: Revert")                                        /*catgets4406 */
#define I18N_RESOLVED               ("catgets 4407: RESOLVED")                                      /*catgets4407 */
#define I18N_Resourses              ("catgets 4408: Resources")                                     /*catgets4408 */
#define I18N_Remove                 ("catgets 4409: Remove")                                        /*catgets4409 */
#define I18N_Replace                ("catgets 4410: Replace")                                       /*catgets4410 */
#define I18N_Rerunable_Job          ("catgets 4411: Rerunable Job")                                 /*catgets4411 */
#define I18N_Resources              ("catgets 4412: Resources")                                     /*catgets4412 */
#define I18N_Run_Limit              ("catgets 4413: Run Limit")                                     /*catgets4413 */
#define I18N_Rerunable              ("catgets 4414: Rerunable")                                     /*catgets4414 */
#define I18N_Resource               ("catgets 4415: Resource")                                      /*catgets4415 */
#define I18N_rename                 ("catgets 4416: rename")                                        /*catgets4416 */
#define I18N_Remote                 ("catgets 4417: Remote")                                        /*catgets4417 */
#define I18N_RSV                    ("catgets 4418: RSV")                                           /*catgets4418 */
#define I18N_running                ("catgets 4419: running")                                       /*catgets4419 */
#define I18N_SUSPENDED              ("catgets 4420: SUSPENDED")                                     /*catgets4420 */
#define I18N_Release                ("catgets 4421: Release")                                       /*catgets4421 */

#define I18N_sec                    ("catgets 4600: sec")                                           /*catgets4600 */
#define I18N_signaled               ("catgets 4601: signaled")                                      /*catgets4601 */
#define I18N_Sorry                  ("catgets 4602: Sorry")                                         /*catgets4602 */
#define I18N_stopped                ("catgets 4603: stopped")                                       /*catgets4603 */
#define I18N_Signal                 ("catgets 4604: Signal")                                        /*catgets4604 */
#define I18N_Sort                   ("catgets 4605: Sort")                                          /*catgets4605 */
#define I18N_Suspend                ("catgets 4606: Suspend")                                       /*catgets4606 */
#define I18N_Switch                 ("catgets 4607: Switch")                                        /*catgets4607 */
#define I18N_SSUSP                  ("catgets 4608: SSUSP")                                         /*catgets4608 */
#define I18N_Submit                 ("catgets 4609: Submit")                                        /*catgets4609 */
#define I18N_Summary                ("catgets 4610: Summary")                                       /*catgets4610 */
#define I18N_Stat                   ("catgets 4611: Stat")                                          /*catgets4611 */
#define I18N_Sub__Time              ("catgets 4612: Sub_Time")                                      /*catgets4612 */
#define I18N_Start__Time            ("catgets 4613: Start_Time")                                    /*catgets4613 */
#define I18N_STATUS                 ("catgets 4614: STATUS")                                        /*catgets4614 */
#define I18N_Select                 ("catgets 4615: Select")                                        /*catgets4615 */
#define I18N_seconds                ("catgets 4616: seconds")                                       /*catgets4616 */
#define I18N_SOURCE                 ("catgets 4617: SOURCE")                                        /*catgets4617 */
#define I18N_SEVERITY               ("catgets 4618: SEVERITY")                                      /*catgets4618 */
#define I18N_standard               ("catgets 4619: standard")                                      /*catgets4619 */
#define I18N_system                 ("catgets 4620: system")                                        /*catgets4620 */
#define I18N_Save                   ("catgets 4621: Save")                                          /*catgets4621 */
#define I18N_Save__As               ("catgets 4622: Save_As")                                       /*catgets4622 */
#define I18N_Save_As                ("catgets 4623: Save As")                                       /*catgets4623 */
#define I18N_Shared                 ("catgets 4624: Shared")                                        /*catgets4624 */
#define I18N_Server                 ("catgets 4625: Server")                                        /*catgets4625 */
#define I18N_Stack_Limit            ("catgets 4626: Stack Limit")                                   /*catgets4626 */
#define I18N_Swap_Limit             ("catgets 4627: Swap Limit")                                    /*catgets4627 */
#define I18N_Static                 ("catgets 4628: Static")                                        /*catgets4628 */
#define I18N_String                 ("catgets 4629: String")                                        /*catgets4629 */
#define I18N_Some_Hosts             ("catgets 4630: Some Hosts")                                    /*catgets4630 */
#define I18N_Shared_by              ("catgets 4631: Shared by:")                                    /*catgets4631 */
#define I18N_Selection              ("catgets 4632: Selection")                                     /*catgets4632 */
#define I18N_Save_Files             ("catgets 4633: Save Files")                                    /*catgets4633 */
#define I18N_Select_File            ("catgets 4634: Select File")                                   /*catgets4634 */
#define I18N_Status                 ("catgets 4635: Status")                                        /*catgets4635 */
#define I18N_SUSP                   ("catgets 4636: SUSP")                                          /*catgets4636 */
#define I18N_suspended              ("catgets 4637: suspended")                                     /*catgets4637 */

#define I18N_To                     ("catgets 4800: To")                                            /*catgets4800 */
#define I18N_terminated             ("catgets 4801: terminated")                                    /*catgets4801 */
#define I18N_Terminate              ("catgets 4802: Terminate")                                     /*catgets4802 */
#define I18N_KillRequeue            ("catgets 4806: Kill & Requeue")                                /*catgets4806 */
#define I18N_KillRemove             ("catgets 4806: Kill & Remove")                                 /*catgets4807 */
#define I18N_Top                    ("catgets 4803: Top")                                           /*catgets4803 */
#define I18N_Task                   ("catgets 4804: Task")                                          /*catgets4804 */
#define I18N_TOTAL                  ("catgets 4805: TOTAL")                                         /*catgets4805 */

#define I18N_Usage                  ("catgets 5000: Usage")                                         /*catgets5000 */
#define I18N_undefined              ("catgets 5001: undefined")                                     /*catgets5001 */
#define I18N_UNKNOWN                ("catgets 5002: UNKNOWN")                                       /*catgets5002 */
#define I18N_Update                 ("catgets 5003: Update")                                        /*catgets5003 */
#define I18N_Updated                ("catgets 5004: Updated")                                       /*catgets5004 */
#define I18N_User                   ("catgets 5005: User")                                          /*catgets5005 */
#define I18N_USUSP                  ("catgets 5006: USUSP")                                         /*catgets5006 */
#define I18N_unavail                ("catgets 5007: unavail")                                       /*catgets5007 */
#define I18N_USERS                  ("catgets 5009: USERS")                                         /*catgets5009 */
#define I18N_user                   ("catgets 5010: user")                                          /*catgets5010 */
#define I18N_User_Group             ("catgets 5010: User Group")                                    /*catgets5010 */
#define I18N_Users                  ("catgets 5011: Users")                                         /*catgets5011 */
#define I18N_USER                   ("catgets 5012: USER")                                          /*catgets5012 */
#define I18N_USER_GROUP             ("catgets 5013: USER/GROUP")                                    /*catgets5013 */
#define I18N_unknown                ("catgets 5014: unknown")                                       /*catgets5014 */
#define I18N_unreach                ("catgets 5015: unreach")                                       /*catgets5015 */
#define I18N_User_Priority          ("catgets 5016: User Priority")                                 /*catgets5016 */

#define I18N_View                   ("catgets 5200: View")                                          /*catgets5200 */
#define I18N_Value                  ("catgets 5201: Value")                                         /*catgets5201 */

#define I18N_Warning                ("catgets 5400: Warning")                                       /*catgets5400 */
#define I18N_WAITING                ("catgets 5401: WAITING")                                       /*catgets5401 */
#define I18N_Windows                ("catgets 5402: Windows")                                       /*catgets5402 */

#define I18N_Yes                    ("catgets 5800: Yes")                                           /*catgets5800 */

#define I18N_zombi                  ("catgets 6000: zombi")                                         /*catgets6000 */

