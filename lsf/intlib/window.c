/* $Id: window.c 397 2007-11-26 19:04:00Z mblack $
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


#include "libint/intlibout.h"
#include "libint/window.h"
// #include "lib/lproto.h"

// FIXME FIXME FIXME might want to use the GNU datetime library here


int addWindow (char *wordpair, windows_t * week[], char *context)
{
	unsigned int oday = 0;
	unsigned int cday = 0;
	float ohour       = 0.0F;
	float chour       = 0.0F;
	float oHourTd     = 0.0F;
	float cHourTd     = 0.0F;
	float oHourNd     = 0.0F;
	float cHourNd     = 0.0F;
	char *sp          = NULL;
	char *word        = NULL;

	sp = strchr (wordpair, '-');
	if (!sp) {
		ls_syslog (LOG_ERR, "%s: Bad time expression in %s", __func__, context);
		return -1;
	}

	*sp = '\0';
	sp++;
	word = sp;

	if (parse_time (word, &chour, &cday) < 0) {
		ls_syslog (LOG_ERR, "%s: Bad time expression in %s", __func__, context);
		return -1;
	}

	word = wordpair;

	if (parse_time (word, &ohour, &oday) < 0) {
	  ls_syslog (LOG_ERR, "%s: Bad time expression in %s", __func__, context);
	  return -1;
	}

	if (((oday && cday) == 0) && (oday != cday)) {
		ls_syslog (LOG_ERR, "%s: Ambiguous time in %s", __func__, context);
		return -1;
	}

	if (oday == 0 && cday == 0 && ohour == chour) {
		ohour = chour = 25.0;
	}

	if (oday == 0) {
		if (ohour > chour) {

			oHourTd = ohour;
			cHourTd = 25.0;
			oHourNd = -1.0;
			cHourNd = chour;
			for( unsigned int i = 1; i < 8; i++) { // FIXME FIXME FIXME FIXME 8 is awfully particular
				insertW (&week[i], oHourTd, cHourTd);
				insertW (&week[i], oHourNd, cHourNd);
			}
		}
		else {
			for ( unsigned int i = 1; i < 8; i++) { // FIXME FIXME FIXME FIXME 8 is awfully particular
				insertW (&week[i], ohour, chour);
			}
		}
	}
	else if (oday == cday) {

		if (ohour > chour) {
			ls_syslog (LOG_ERR, "%s: Ambiguous time in %s", __func__, context);
			return -1;
		}
		else if (ohour == chour) {
			ohour = 25.0;
			chour = 25.0;
		}
		insertW (&week[oday], ohour, chour);
		for ( unsigned int j = 1; j < 8; j++) { // FIXME FIXME FIXME FIXME 8 is awfully particular
			if ((week[j] == NULL) && (j != oday)) {
				insertW (&week[j], 25.0, 25.0);
			}
		}
	}
	else {
		for ( unsigned int i = oday; ; i++) { // FIXME FIXME FIXME FIXME --- NO TERMINATING CONDITIONS

			if (i == 8) {
				i = 1;
			}
			if (i == oday) {
				insertW (&week[i], ohour, 25.0);
				continue;
			}

			if (i == cday)
			{
				insertW (&week[i], -1.0, chour);
				for ( unsigned int j = 1; j < 8; j++) {
					if (week[j] == NULL) {
						insertW (&week[j], 25.0, 25.0);
					}
				}

				break; // FIXME FIXME FIXME FIXME this break is in the wrong possition, probably
			}	

			insertW (&week[i], -1.0, 25.0);
		}
	}

	return 0;
}

void insertW (windows_t ** window, float ohour, float chour)
{
	windows_t *wp = NULL;
	windows_t *nextw = NULL;
	int merged = 0;

	if (!*window) {
		if ((*window = new_wind ()) == NULL) {
			return;
		}
		wp = *window;
	}
	else {
		merged = FALSE;

		for (wp = *window;; wp = wp->nextwind) {
			if (mergeW (wp, ohour, chour) == TRUE ) {
				merged = TRUE;
			}

			if (wp->nextwind == NULL) {
				break;
			}
		}

		if (merged) {

			wp = (*window)->nextwind;
			(*window)->nextwind = NULL;
			for (; wp != NULL; wp = nextw) {
				insertW (window, wp->opentime, wp->closetime);
				nextw = wp->nextwind;
				free (wp);
			}
			return;
		}
		else {
			wp->nextwind = new_wind ();
			wp = wp->nextwind;
		}
	}

	if (ohour < chour) {
		wp->opentime = ohour;
		wp->closetime = chour;
	}

	return;
}

int mergeW (windows_t * wp, float ohour, float chour)
{

	if ((wp->opentime == -1.0) && (wp->closetime == 25.0)) {
		return TRUE;
	}

  if ((ohour == -1.0) && (chour == 25.0))
	{
	  wp->opentime = -1.0;
	  wp->closetime = 25.0;
	  return TRUE;
	}

  if ((ohour == 25.0) && (chour == 25.0))
	{
	  return TRUE;
	}

  if (ohour >= chour)
	{
	  return TRUE;
	}

  if ((wp->opentime == 25.0) && (wp->closetime == 25.0))
	{
	  wp->opentime = ohour;
	  wp->closetime = chour;
	  return TRUE;
	}
  ohour = (ohour <= 0.0) ? -1.0 : ohour;
  chour = (chour >= 24.0) ? 25.0 : chour;

  if (!(wp->opentime > chour) && !(ohour > wp->closetime))
	{
	  wp->opentime = MIN (ohour, wp->opentime);
	  wp->closetime = MAX (chour, wp->closetime);
	  return TRUE;
	}

  return FALSE;
}

void checkWindow (struct dayhour *dayhour, char *active, time_t * wind_edge, windows_t * wp, time_t now)
{
  time_t tmp_edge = 0.0F;
  float tmp_time = 0.0F;

  if (dayhour->hour >= wp->opentime && dayhour->hour < wp->closetime)
	{
	  *active = TRUE;
	  tmp_edge = now + (wp->closetime - dayhour->hour) * 3600;
		if (tmp_edge < *wind_edge) {
			*wind_edge = tmp_edge;
		}
	}
  else
	{
	  tmp_time = MIN (wp->opentime, 24.0);
	  tmp_edge = now + (tmp_time - dayhour->hour) * 3600;
		if (tmp_edge < *wind_edge) {
			*wind_edge = tmp_edge;
		}
	}

	return;
}

windows_t *new_wind (void)
{
	windows_t *wp = NULL;

	wp = malloc ( sizeof ( *wp ) );
	if (!wp) {
		return NULL;
	}

	wp->nextwind = NULL;
	wp->opentime = 25.0;
	wp->closetime = 25.0;

	return wp;
}


void delWindow (windows_t * wp)
{
	windows_t *nextWp = NULL;

	if (wp == NULL) {
		return;
	}

	while (wp) {
		nextWp = wp->nextwind;
		FREEUP (wp);
		wp = nextWp;
	}
}


int parse_time (char *word, float *hour, unsigned int *day)
{
	float min = 0.0F;
	char *sp  = NULL;

	*day = 0;
	*hour = 0.0;
	min = 0.0;

	sp = strrchr (word, ':');
	if (!sp) {
		if (!isint_ (word) || atoi (word) < 0) {
			return -1;
		}
		*hour = atof (word);
		if (*hour > 23) {
			return -1;
		}
	}
	else {
		*sp = '\0';
		sp++;

		if (!isint_ (sp) || atoi (sp) < 0) {
			return -1;
		}

		min = atoi (sp);
		if (min > 59) {
			return -1;
		}

		sp = strrchr (word, ':');
		if (!sp ) {
			if (!isint_ (word) || atoi (word) < 0) {
				return -1;
			}
			*hour = atof (word);
			if (*hour > 23) {
				return -1;
			}
		}
		else {
			*sp = '\0';
			sp++;
			if (!isint_ (sp) || atoi (sp) < 0) {
				return -1;
			}

			*hour = atof (sp);
			if (*hour > 23) {
				return -1;
			}

			if (!isint_ (word) || atoi (word) < 0) {
				return -1;
			}

			*day = atoi (word);
			if (*day == 0) {
				*day = 7;
			}
			if (*day < 1 || *day > 7) {
				return -1;
			}
		}
	}

	*hour = *hour + min / 60.0;
	return 0;
}

void getDayHour (struct dayhour *dayPtr, time_t nowtime)
{
	char *timep = NULL;

	timep = (char *) ctime (&nowtime); 	// FIXME FIXME FIXME FIXME FIXME what the fuck. ctime to char?
	timep[3] = '\0';

	if (strcmp (timep, "Sun") == 0) {
		dayPtr->day = 7;
	}
	else if (strcmp (timep, "Mon") == 0) {
		dayPtr->day = 1;
	}
	else if (strcmp (timep, "Tue") == 0) {
		dayPtr->day = 2;
	}
	else if (strcmp (timep, "Wed") == 0) {
		dayPtr->day = 3;
	}
	else if (strcmp (timep, "Thu") == 0) {
		dayPtr->day = 4;
	}
	else if (strcmp (timep, "Fri") == 0) {
		dayPtr->day = 5;
	}
	else if (strcmp (timep, "Sat") == 0) {
		dayPtr->day = 6;
	}

	timep += 11;

	timep[2] = '\0';
	dayPtr->hour = atof (timep);

	timep += 3;
	timep[2] = '\0';
	dayPtr->hour += atof (timep) / 60.0;

	return; 
}
