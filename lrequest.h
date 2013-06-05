#ifndef _L_REQUEST_H
#define _L_REQUEST_H


struct request_data {
	char method[10];
	char *url;
	char magic[10];
	int major, minor;
};


void parse_request(struct request_data *);
int check_request();
void head(void);
void get(void);


#endif // _L_REQUEST_H
