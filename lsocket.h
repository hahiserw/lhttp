#ifndef _L_SOCKET_H
#define _L_SOCKET_H


#include "lrequest.h"

#include <netinet/in.h>
#include <sys/socket.h>


// struct client_info {
// 	int sockfd;
// 	struct sockaddr_in address;
// 	socklen_t address_length;
// };

enum line_type {
	REQUEST,
	HEADER,
	BODY,
	END
};


// Zaczynamy zabawę
void create_server(char *, char *);
void accept_connectins();

void handle_connection(/*struct client_info **/);

int another_line_from_buffer(char *, char **);
int clear_buffer_to_eol(char *);
int parse_line(enum line_type *, char *, struct request_data *);


#endif // _L_SOCKET_H
