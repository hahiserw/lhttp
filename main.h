#ifndef _MAIN_H
#define _MAIN_H


#include "lsocket.h"


/* Zmienne globalne **********************************************************/

enum log_level {
	ERROR,
	WARN,
	NOTICE, // Ważne wydarzenie
	INFO,   // Informacja
	CONNECTION, // Związane z zapytaniami
	DEBUG1,
	DEBUG2
};

extern enum log_level verbosity;

extern char *temp_dir;
extern char *base_path;

extern int server;
extern /*__thread*/ int client;



/* Deklaracje funkcji ********************************************************/

void usage(void);

void daemonize(void);
void close_stds(void);

void log_message(enum log_level, const char *, ...);
void die(const char *);

void prepare_signals();
void handle_signal(int);

void parse_args(int, char *[]);


#endif // _MAIN_H
