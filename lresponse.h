#ifndef _L_RESPONSE_H
#define _L_RESPONSE_H


int write_line(char *, ...);

void connection_die(int, char *);
void examine_die(int, char *);

void basic_headers(void);

void serve(char *);
void list_dir(char *);

int content_type(char *, char *);


#endif // _L_RESPONSE_H