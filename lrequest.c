#include "main.h"
#include "lrequest.h"
#include "lresponse.h"


/*__thread*/ int head_request = 0;


void parse_request(struct request_data *data)
{
	// if (version == 1.1 && no_host_header)
	// 	return 400;
	// log_message(INFO, "LOOL %i", *(data->method));
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
		connection_die(501, "It's just for zaliczenie, don't expect too much");
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

	// Usunięcie wszystkich "../"
	// Dwa razy, bo ../../../ za pierwszym razem zmieni się na ../
	remove_dots(url);
	remove_dots(url);

	// Zamień %coś na znaczki,
	// ALE uważaj na brzydkich ludzi, którzy psują serwery!

}

void remove_dots(char *url)
{
	char *curr = url + 2; // By nie wyjść za \0
	while (*curr) {
		if (*(curr-2) == '.' && *(curr-1) == '.' && *curr == '/') {
			char *curr_to = curr - 2;
			char *curr_from = curr + 1;
			while (*curr_to++ = *curr_from++);
			*curr_to = '\0';
		}
		++curr;
	}

}
