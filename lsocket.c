#include "main.h"
#include "lsocket.h"
#include "lrequest.h"
#include "lresponse.h"

#include <stdlib.h>
#include <stdio.h> // stderr

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // pointer dereference to incomplete type

#include <string.h> // strlen

#include <pthread.h>


void create_server(char *address_string, char *port)
{
	struct addrinfo *addr;
	if (getaddrinfo(address_string, port, NULL, &addr))
		die("Wrong ip address");

	log_message(DEBUG1, "Server listening on %s:%s", address_string, port);

	server = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

	if (server < 0)
		die("Error opening socket");

	// Pozwala na ponowne użycie adresu.
	// if (verbosity >= DEBUG1) {
		int sockoptval = 1;
		setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &sockoptval,
			sizeof(int));
	// }

	if (bind(server, addr->ai_addr, addr->ai_addrlen) < 0)
		die("Error binding server to address");

	listen(server, 5);
}

void accept_connections()
{
	pthread_t client_thread;
	// int client;

	struct sockaddr_in client_addr;
	socklen_t client_addr_length = sizeof(client_addr);
	char client_ip[INET_ADDRSTRLEN];

	while (1) {
		client = accept(server,
			(struct sockaddr *)&client_addr, &client_addr_length);
			// &client.address, &client.address_length);

		if (client < 0) {
			// log_message(NOTICE, "Client connection failed");
			continue;
		}

		if (!inet_ntop(client_addr.sin_family,
			&client_addr.sin_addr,
			client_ip, INET_ADDRSTRLEN))
			strcpy(client_ip, "[ip error]");
		log_message(CONNECTION, "Client %s connected", client_ip);

		// Nowy wątek tu
		if (pthread_create(&client_thread, NULL,
			(void *)handle_connection, NULL/*, &client*/))
			die("Error creating thread");
	}
}

// Obsłuż połączenie
void handle_connection()
{
	int buffer_size = 1500;
	int chunk_size = 150;

	char *buffer = (char *)malloc(sizeof(char) * buffer_size);
	*buffer = 0; // wtf?

	char chunk[chunk_size];
	int chunks = 0;

	int read_length = 0;

	enum line_type type = REQUEST;

	struct request_data request;

	char *line_start = buffer;

	int chars;
	char *line;

	// Pierw czytaj trochę socketu, a potem szukaj w tym znaku nowej linii.
	// Jak znajdzie, to przesuwa się dalej i tak póki są dane.
	// Albo powiększaj bufor jeśli braknie miejsca.

	while (1) {
		read_length = read(client,
			chunk, chunk_size);

		// Jeśli wciąż nie ma końca linii...
		if (chunks * chunk_size > buffer_size) {
			log_message(DEBUG1,
				"Reallocating buffer for %i*%i bytes",
				chunks, chunk_size);
			if (realloc(buffer,
				sizeof(char) * chunks * chunk_size) == NULL)
				connection_die(500, "Cannot realloc."
					" Too long request line?");
		}

		// log_message(DEBUG2, "Chunk contents:\n%s", chunk);
		strcat(buffer, chunk);
		++chunks;

		// log_message(DEBUG2, "Buffer contents:\n%s", buffer);
		line_start = buffer;

		if (strstr(buffer, "\n")) {
			line = (char *)malloc(strlen(buffer) + 1);
			// Bufor może zawierać kilka linii
			while (1) {
				chars = another_line_from_buffer(
					line, &line_start);
				if (chars < 0)
					break;
				parse_line(&type, line, &request);
			}

			// Usuń wszytko przed \n
			clear_buffer_to_eol(buffer);
			chunks = 0;
			free(line);
		}

		// Koniec rekłesta
		if (read_length != chunk_size)
			break;
	}

	log_message(DEBUG1, "End of request");

	free(buffer);

	// Zabawa!
	parse_request(&request);

	close(client);

	// Sprzątamy
	free(request.url);
}

// Wrzuca linię z pointer do line (bez \r\n). Line musi być zaalokowaną tablicą
// znaków. Zwraca ilość skopiowanych znaków. -1 gdy wskaźnik wychodzi poza
// bufor.
int another_line_from_buffer(char *line, char **pointer)
{
	int chars = 0;

	while (1) {
		if (**pointer == '\0') {
			return -1;
		} else if (**pointer == '\r' && *(*pointer+1) == '\n') {
			*pointer += 2;
			break;
		// Bez CR też przyjmujemy rekłesty
		} else if (**pointer == '\n') {
			++*pointer;
			break;
		}

		*line++ = *(*pointer)++;
		++chars;
	}
	*line = '\0';
	return chars;
}

int parse_line(enum line_type *type, char *line, struct request_data *request)
{
	int size_temp = strlen(line) + 1;
	char temp[size_temp];
	char temp2[size_temp];
	// log_message(DEBUG2, "Line type: %i", *type);
	switch (*type) {
	case REQUEST:
		request->url = (char *)malloc(strlen(line) + 1);
		// Trochę brzydko. :c
		sscanf(line, "%s %s %[^/]/%i.%i",
			&request->method, request->url, &request->magic,
			&request->major, &request->minor);
		// Sprawdzanie jakieś przydałoby się jeszcze tu
		log_message(CONNECTION, "%s %s %s/%i.%i",
			request->method, request->url, request->magic,
			request->major, request->minor);
		++*type;
		break;
	case HEADER:
		if (strlen(line) == 0)
			++*type;
		// I tak w linii nie ma \r lub \n, a jakoś trza zrobić by
		// spacje też łapał
		else if (sscanf(line, "%[^:]: %[^\r\n]", &temp, &temp2) == 2)
			log_message(DEBUG2, "Header: %s: %s", temp, temp2);
		else
			log_message(DEBUG1, "Wrong header?\n> %s", line);
		break;
	case BODY:
		// log_message(DEBUG1, "Request body?\n> %s", line);
		// pusta linia? ++*type
		// break;
	case END:
		break;
	}
}

// Nadpisuje bufor danymi znajdującymi się za ostatnim znakiem nowej linii
// Zwraca ilość przeipsanych znaków; -1 gdy bufor nie zawierał \n
int clear_buffer_to_eol(char *buffer)
{
	int characters = 0;

	char *data = buffer;
	char *rest_start = NULL;

	// Szukamy ostatniego \n
	char *temp = data;
	while (temp = strstr(++temp, "\n")) {
		rest_start = temp;
	}

	if (rest_start == NULL)
		return -1;

	// Wskazuje pierwszą literę za ostatnim \n
	++rest_start;

	while (*data++ = *rest_start++)
		++characters;
	*data = '\0';

	return characters;
}
