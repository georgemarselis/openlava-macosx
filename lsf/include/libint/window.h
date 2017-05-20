
#pragma once

// #define NL_SETN      22

int mergeW( windows_t * wp, float ohour, float chour );
void insertW( windows_t ** window, float ohour, float chour );
windows_t *new_wind( void );
int addWindow( char *wordpair, windows_t * week[], char *context );
void checkWindow( struct dayhour *dayhour, char *active, time_t *wind_edge, windows_t *wp, time_t now);
void delWindow( windows_t* wp );

int parse_time (char *word, float *hour, unsigned int *day);
void getDayHour (struct dayhour *dayPtr, time_t nowtime);
