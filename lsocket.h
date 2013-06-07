#ifndef _L_SOCKET_H
#define _L_SOCKET_H


#include "lrequest.h"

#include <netinet/in.h>
#include <sys/socket.h>


enum line_type {
	REQUEST,
	HEADER,
	BODY,
	END
};


void create_server(char *, char *);
void accept_connectins(void);

void handle_connection(void);

int another_line_from_buffer(char *, char **);
int clear_buffer_to_eol(char *);
int parse_line(enum line_type *, char *, struct request_data *);


#endif // _L_SOCKET_H
