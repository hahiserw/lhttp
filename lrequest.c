#include "main.h"
#include "lrequest.h"
#include "lresponse.h"


void parse_request(struct request_data *data)
{
	// if (version == 1.1 && no_host_header)
	// 	return 400;
	// log_message(INFO, "LOOL %i", *(data->method));
	switch (*(data->method)) {
	case 'G': // GET
	case 'g':
		serve(data->url);
		// connection_die(200, "Gicior xDD");
		break;
	case 'H': // HEAD
	case 'h':
		break;
	default:
		connection_die(501, "U mad?");
	}
}

void parse_headers() {
	// dekoduj uri i ogarniaj nagłówek host
	// accept, accept-charset, accept-encoding, accept-language,
	// expect, if-modified-since
	// authentication -> odpowiedz, że nie, czy coś
	// from -> tehe
	log_message(INFO, "Header %s not implemented", "");
}

void head()
{
	//
}

void get()
{
}

char *decode_uri(char *uri)
{
	// Wyjeb http://ja(:(port)) <- tu wielkość liter nie ma znacznia

	// Zamień %coś na znaczki,
	// ALE uważaj na brzydkich ludzi, którzy psują serwery!

}
