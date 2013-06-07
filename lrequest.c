#include "main.h"
#include "lrequest.h"
#include "lresponse.h"

#include <stdlib.h>
#include <string.h>


/*__thread*/ int head_request = 0;


void parse_request(struct request_data *data)
{
	switch (*(data->method)) {
	case 'H': // HEAD
	case 'h':
		head_request = 1;
	case 'G': // GET
	case 'g':
		decode_url(data->url);
		serve(data->url);
		head_request = 0;
		break;
	default:
		connection_die(501, "It's just for zaliczenie,"
			" don't expect too much");
	}
}

// void parse_headers() {
// 	// dekoduj url i ogarniaj nagłówek host
// 	// accept, accept-charset, accept-encoding, accept-language,
// 	// expect, if-modified-since
// 	// authentication -> odpowiedz, że nie, czy coś
// }

void decode_url(char *url)
{
	// Pozbądź się http://ja(:(port)) <- tu wielkość liter nie ma znacznia
	// To jest tylko potrzebne jeśli ruch idzie przez proxy http

	// Usuń wszystkie "../"
	if (strstr(url, "../")) {
		// Dwa razy, bo ../../../ za pierwszym razem zmieni się na ../
		remove_dots(url);
		remove_dots(url);
	}

	// Zamień %coś na znaczki
	if (strstr(url, "%"))
		unescape(url);
}

void remove_dots(char *url)
{
	char *curr = url + 2; // By nie wyjść za \0
	while (*curr) {
		if (*(curr - 2) == '.' && *(curr - 1) == '.' && *curr == '/') {
			char *curr_to = curr - 2;
			char *curr_from = curr + 1;
			while (*curr_to++ = *curr_from++);
			*curr_to = '\0';
		}
		++curr;
	}

}

// Przydałoby się zabezpieczyć przed złymi ludźmi jakoś
void unescape(char *url)
{
	char *src = url;
	char *dest = url;

	char buffer[] = "0x00";

	while (*dest) {
		while (*src == '%') {
			++src;
			buffer[2] = *src++;
			buffer[3] = *src++;
			*dest++ = strtol(buffer, NULL, 16);
		}
		*dest++ = *src++;
	}

	*dest = '\0';
}
