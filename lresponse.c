#include "main.h"
#include "lresponse.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> // va
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


char response_body[] =
		"<!doctype html>\r\n"
		"<html>\r\n"
		"<head>\r\n"
		"	<title>Something went wrong</title>\r\n"
		"	<style>\r\n"
		"		body {\r\n"
		"			font-family: Verdana;\r\n"
		"		}\r\n"
		"	</style>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"	<h1>Something went wrong</h1>\r\n"
		"	<br />\r\n"
		"	<h2>%s</h2>\r\n"
		"	<p>%s</p>\r\n"
		"</body>\r\n"
		"</html>\r\n";


// Tak bezczelnie podzielone, by nie robić tablicy 5 na 17 (przez response4xx)

char *response1xx[] = {
	"Continue",
	"Switching Protocols"
};

char *response2xx[] = {
	"OK",
	"Created",
	"Accepted",
	"Non-Authoritative Information",
	"No Content",
	"Reset Content",
	"Partial Content"
};

char *response3xx[] = {
	"Multiple Choices",
	"Moved Permanently",
	"Found",
	"See Other",
	"Not modified",
	"Use Proxy",
	"",
	"Temporary Redirect"
};

char *response4xx[] = {
	"Bad Request", // jest kontent, ale brak content-length
	"Unauthorized",
	"Payment Required",
	"Forbidden",
	"Not Found",
	"Method Not Allowed",
	"Not Acceptable", // accept-charset mi nie pasuje?
	"Proxy Authentication Required",
	"Request Time-out",
	"Conflict",
	"Gone", // 10
	"Length Required",
	"Precondidtion Failed",
	"Request Entity Too Large",
	"Request-URI Too Large",
	"Unsupported Media Type",
	"Requested range not satisfiable",
	"Expectation failed"
};

char *response5xx[] = {
	"Internal Server Error",
	"Not Implemented",
	"Bad Gateway",
	"Service Unavailable",
	"Gateway Time-out",
	"HTTP Version not supported"
};

char **response[] = {
	NULL,
	response1xx,
	response2xx,
	response3xx,
	response4xx,
	response5xx
};


// Typy mime po rozszerzeniach

char *ext_application[] = {
	"json",  "json",
	"js",    "javascript",
	"pdf",   "pdf",
	"ps",    "postscript",
	"xhtml", "xhtml+xml",
	"xml",   "xml",
	"zip",   "zip",
	"gzip",  "gzip"
};

char *ext_audio[] = {
	"mp4",   "mp4",
	"mp3",   "mpeg",
	"ogg",   "ogg",
	"oga",   "ogg",
	"flac",  "x-flac",
	"ra",    "vnd.rn-realaudio"
	"wav",   "vnd.wave",
	"webm",  "webm"
};

char *ext_image[] = {
	"gif",   "gif",
	"jpeg",  "jpeg",
	"pjpeg", "pjpeg",
	"png",   "png",
	"svg",   "svg+xml",
	"tiff",  "tiff",
	"ico",   "x-icon"
};

char *ext_text[] = {
	"css",   "css",
	"csv",   "csv",
	"html",  "html",
	// "",      "plain", // Tak?
	"txt",   "plain",
	"h",     "plain",
	"c",     "plain",
	"vcf",   "vcard",
	"vcard", "vcard"
	"xml",   "xml"
};

char *ext_video[] = {
	"mpg",   "mpeg",
	"mpeg",  "mpeg",
	"mp1",   "mpeg",
	"mp2",   "mpeg",
	"mp3",   "mpeg",
	"m1v",   "mpeg",
	"m1a",   "mpeg",
	"m2a",   "mpeg",
	"mpa",   "mpeg",
	"mpv",   "mpeg",
	"mp4",   "mp4",
	"ogg",   "ogg",
	"qt",    "quicktime",
	"webm",  "webm",
	"mkv",   "x-matroska",
	"wmv",   "x-ms-wmv",
	"flv",   "x-flv"
};

char **extensions[] = {
	ext_text,
	ext_application,
	ext_image,
	ext_video,
	ext_audio
};

#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))

int extensions_sizes[] = {
	NELEMS(ext_text),
	NELEMS(ext_application),
	NELEMS(ext_image),
	NELEMS(ext_video),
	NELEMS(ext_audio)
};


// Zwraca typ mime na podstawie rozszerzenia (określone w tablicach wyżej)
int content_type(char *mime, char *extension)
{
	int type, types;
	int i, size;
	char **curr;
	types = NELEMS(extensions);
	for (type = 0; type < types; ++type) {
		curr = extensions[type];
		size = extensions_sizes[type];
		for (i = 0; i < size; i += 2) {
			if (!strcmp(curr[i], extension)) {
				switch (type) {
				case 0:
					strcpy(mime, "text");
					break;
				case 1:
					strcpy(mime, "application");
					break;
				case 2:
					strcpy(mime, "image");
					break;
				case 3:
					strcpy(mime, "video");
					break;
				case 4:
					strcpy(mime, "audio");
					break;
				}
				strcat(mime, "/");
				strcat(mime, extensions[type][i+1]);
				// log_message(INFO, "Stuff: %s", mime);
				return 0;
			}
		}
	}
	strcpy(mime, "text/plain");
	return -1;
}


// Wypisuje do socketu sformatowaną linię ładnie alokując bufor
// Zwraca całkowitą ilość znaków w buforze
int write_line(char *line, ...)
{
	// Trza ładny rozmiar dla bufora ogarnąć
	int buffer_size = strlen(line) + 3;

	// char format;
	char *arg;
	char *position = line;

	// Dodaje bajtów do bufora (np. dla %s)
	va_list args;
	va_start(args, line);
	while (*position) {
		// Sprawdza tylko kolejny znak, bo raczj nie będę się bawił
		// w piękne formatowanie
		if (*position++ == '%') {
			switch (*position) {
			case 's':
				arg = va_arg(args, char *);
				buffer_size += strlen(arg);
				break;
			default:
				va_arg(args, void);
				buffer_size += 10;
			}
		}
	}
	va_end(args);

	char *buffer = (char *)malloc(buffer_size);

	// I wypełniamy te procenty argumentami
	va_start(args, line);
	vsprintf(buffer, line, args);
	va_end(args);

	log_message(DEBUG2, "Response: %s (%i)", buffer, strlen(buffer));
	strcat(buffer, "\r\n");
	write(client, buffer, strlen(buffer));

	int characters = strlen(buffer);

	free(buffer);

	return characters;
}

void basic_headers()
{
	write_line("Server: lhttpd");
	write_line("Connection: close");
	// write_line("Date: ?")
}

// Dla statusów 400+
void connection_die(int status, char *message)
{
	log_message(INFO, "Connection dying with: %s (%i %s)",
		message, status, response[status / 100][status % 100]);

	write_line("HTTP/1.1 %i %s",
		status, response[status / 100][status % 100]);
	basic_headers();
	write_line("");

	write_line(response_body,
		response[status / 100][status % 100], message);
}

// Wysyła podany plik wraz z odpowiednimi nagłółkami do socketu
void send_file(char *filepath)
{
	// Inaczej wyślij plik
	FILE *data;
	data = fopen(filepath, "r");

	if (!data) {
		examine_die(errno, filepath);
		return;
	}

	write_line("HTTP/1.1 200 OK");
	fseek(data, 0, SEEK_END);
	write_line("Content-Length: %i", ftell(data));
	fseek(data, 0, SEEK_SET);

	// Szukamy ostatniej kropki
	char *extension;
	char *temp = filepath;
	while (temp = strstr(++temp, ".")) {
		extension = temp;
	}
	++extension;

	if (extension != '\0') {
		char mime[29];
		if (content_type(mime, extension) == 0)
			write_line("Content-Type: %s", mime);
		// else
		// 	write_line("Content-Type: application/octet-stream");
	}

	basic_headers();
	write_line("");

	// To tylko HEAD
	if (head_request) {
		fclose(data);
		return;
	}

	log_message(DEBUG2, "Response file: %s", filepath);

	// Wysyłamy plik
	int size = 512;
	char buffer[size];

	size_t count;

	while (!feof(data)) {
		count = fread(buffer, 1, size, data);
		write(client, buffer, count);
		// log_message(DEBUG2, "written: %i", count);
	}
	// log_message(DEBUG2, "closing file");

	write_line("");

	fclose(data);
}

// Podejmuje decyzje jak obsłużyć zapytanie
void serve(char *file)
{
	char *filepath = (char *)malloc(strlen(base_path) + strlen(file) + 2);
	strcpy(filepath, base_path);
	strcat(filepath, "/");
	strcat(filepath, file);

	// Ostatni znak '/' lub stat S_IFDIR
	struct stat info;
	if (stat(filepath, &info) != 0) {
		log_message(NOTICE, "Stat error on file: %s", filepath);
		examine_die(errno, filepath);
		return;
	}

	if (filepath[strlen(filepath) - 1] == '/' || info.st_mode & S_IFDIR) {
		// A teraz sprawdź czy jest tu index.html
		char *index = (char *)malloc(strlen(filepath) + 12);
		strcpy(index, filepath);
		strcat(index, "/index.html");

		// index.html istnieje w tym katalogu?
		if (access(index, F_OK) != -1)
			send_file(index);
		else
			list_dir(filepath);

		return;
	}

	send_file(filepath);
}

// Wypisuje listę danego katalogu
void list_dir(char *path)
{
	log_message(INFO, "List: %s", path);

	struct dirent **dir_list;
	int files;

	files = scandir(path, &dir_list, NULL, alphasort);
	if (files < 0) {
		examine_die(errno, path);
		return;
	}

	char *tmp = "/tmp/lhttpd-temp-list.html";

	FILE *list = fopen(tmp, "w");

	if (!list) {
		examine_die(errno, "/");
		// connection_die(501, "Bloody Linux!");
		return;
	}

	// To zmienia path na relatywny katalog względem base_path
	char *path_bad = path;
	// Przesunięcie o base_path
	char *path_good = path + strlen(base_path) + 1;
	while (*path_bad++ = *path_good++);
	*path_bad = '\0';

	fprintf(list,
		"<!doctype html>\r\n"
		"<html>\r\n<head>\r\n"
		"	<title>Index of %s</title>\r\n"
		"	<style>\r\n"
		"		body {\r\n"
		"			font-family: Verdana\r\n"
		"		}\r\n"
		"		td {\r\n"
		"			padding: 0 10px;\r\n"
		"		}\r\n"
		"	</style>\r\n"
		"</head>\r\n<body>\r\n"
		"	<h1>%s</h1>\r\n"
		"	<br />\r\n"
		"	<table>\r\n"
		"		<tr><th>type</th><th>name</th></tr>\r\n",
		path, path);

	struct dirent *entry;
	int i;
	char *type;

	for (i = 0; i < files; ++i) {
		entry = dir_list[i];
		if (strcmp(entry->d_name, ".")) {
			switch (entry->d_type) {
			case DT_BLK:
				type = "block dev";
				break;
			case DT_CHR:
				type = "char dev";
				break;
			case DT_DIR:
				type = "directory";
				break;
			case DT_FIFO:
				type = "FIFO pipe";
				break;
			case DT_LNK:
				type = "symlink";
				break;
			case DT_REG:
				type = "file";
				break;
			case DT_SOCK:
				type = "socket";
				break;
			case DT_UNKNOWN:
				type = "unknown";
				break;
			}

			fprintf(list, "<tr><td>%s</td><td><a href=\"%s",
				type, entry->d_name);
			if (entry->d_type == DT_DIR)
				fprintf(list, "/");
			fprintf(list, "\">%s</a></td></tr>\r\n",
				entry->d_name);
		}
		free(dir_list[i]);
	}
	free(dir_list);
	// if (!entry)
	// 	examine_die(errno, path);

	fprintf(list,
		"	</table>\r\n"
		"</body>\r\n</html>\r\n"
	);

	fflush(list);

	fclose(list);

	send_file(tmp);

	if (unlink(tmp))
		log_message(INFO, "Error unlinkink temporary file");
}

void examine_die(int error, char *path)
{
	switch (error) {
	case ENOENT:
	case ENOTDIR:
		connection_die(404, "There is no such thing like this");
		break;
	case EACCES:
		connection_die(403, "Sorry dude, you can't see this");
		break;
	default:
		log_message(NOTICE, "error (%i) opening  '%s'",
			error, path);
		connection_die(500, "Things happen");
		break;
	}
}
