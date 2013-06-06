#ifndef _L_REQUEST_H
#define _L_REQUEST_H


extern int head_request;

struct request_data {
	char method[10];
	char *url;
	char magic[10];
	int major, minor;
};


void parse_request(struct request_data *);
void decode_url(char *);

void remove_dots(char *);

#endif // _L_REQUEST_H
