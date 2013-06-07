// Program napisany na zaliczenie przedmiotu SO2
// WI ZUT
// PH

// http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
// http://www.itp.uzh.ch/~dpotter/howto/daemonize

#include <stdio.h>
#include <stdlib.h> // error_failure

#include <unistd.h>
#include <stdarg.h> // va list
#include <signal.h>
#include <time.h> // strftime, localtime

#include <string.h>

#include "main.h"
#include "lsocket.h"

#include <errno.h>



/* Zmienne globalne **********************************************************/

char *program_name;

char *temp_dir = "/tmp";
char *log_file = "/tmp/lhttpd.log";
char *base_path = ".";

enum log_level verbosity = INFO;
int foreground = 0;
int leon = 0;

int server; // Deskryptor pliku dla gniazda
/*__thread*/ int client;

char *address = "0.0.0.0";
char port[10] = "80";



/* Mejn **********************************************************************/

int main(int argc, char *argv[])
{
	program_name = argv[0];

	// Przetworzenie argumentów
	parse_args(argc, argv);

	/* Demonizowanie */
	// Wrzuć w tło, chyba, że ma działać jako zwykły program
	if (!foreground)
		daemonize();

	// Komunikat w tym miejscu, bo gdyby wystąpił problem w otwieraniu
	// pliku log demon nie mógłby o tym powiedzieć
	log_message(NOTICE, "*");
	log_message(NOTICE, "* lhttpd started");

	if (!foreground)
		close_stds();

	// Przechwyć sygnały
	prepare_signals();

	// Utworzenie soketu
	log_message(DEBUG1, "Opening socket");
	// if (strcmp(port, "1024") < 0)
	// 	log_message(WARN, "Using priviledged port");
	create_server(address, port);

	// Czekamy w nieskończoność na zapytania
	accept_connections();
}



/* Funkcje podstawowe ********************************************************/

void daemonize()
{
	pid_t child, session;

	child = fork();

	if (child < 0) {
		fprintf(stderr, "%s: cannot fork\n", program_name);
		exit(EXIT_FAILURE);
	}

	if (child > 0)
		exit(EXIT_SUCCESS);

	// Pełna kontrola nad swoimi plikami
	umask(0);

	// Jakaś sesja
	session = setsid();
	if (session < 0) {
		fprintf(stderr, "%s: cannot set session\n", program_name);
		exit(EXIT_FAILURE);
	}

	// Zmiana katalogu roboczego na bezpieczny
	if (chdir("/") < 0) {
		fprintf(stderr, "%s: cannot change working directory\n",
			program_name);
		exit(EXIT_FAILURE);
	}
}

void close_stds() {
	// Zamykanie niepotrzebnych strumieni
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

void prepare_signals()
{
	// TERM, INT -> zamknij sokety i rzeczy
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

}

void handle_signal(int number)
{
	switch (number) {
	case SIGINT:
	case SIGTERM:
		// Zamknij ładnie serwer przy tych sygnałach
		close(server);
		log_message(WARN, "* Server terminated");
		exit(EXIT_FAILURE);
	default:
		log_message(WARN, "Got signal nr %i, ignoring", number);
	}
}


void parse_args(int argc, char *argv[])
{
	int option;
	while ((option = getopt(argc, argv, "hvft:l:d:3")) != -1) {
		switch (option) {
		case 'v':
			verbosity++; // Nie podasz więcej niż int? ;p
			break;
		case 'f':
			foreground = 1;
			break;
		case 't':
			temp_dir = optarg;
			break;
		case 'l':
			log_file = optarg;
			break;
		case 'd':
			base_path = optarg;
			break;
		case '3':
			leon = 1;
			break;
		case 'h':
		default:
			usage();
		}
	}

	if (optind < argc) {
		address = (char *)malloc(strlen(argv[optind]) + 1);
		sscanf(argv[optind], "%[^:]:%s", address, port);
	}
}

void usage()
{
	fprintf(stderr, "Usage: %s [-h] [-v] [-f] [-t temp_dir] [-l log_file]"
		" [-d working_directory] [ip[:port]]\n", program_name);
	fprintf(stderr, "\n");
	fprintf(stderr, "Listen for http requests at ip[:port] and serve files"
		" from working_directory\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -h  Display this helpful text and exit\n");
	fprintf(stderr, "  -v  Verbosity level, more `v', more verbose\n");
	fprintf(stderr, "  -f  Stay in foreground"
		" (don't demonize, disables -l)\n");
	fprintf(stderr, "  -t  Temporary directory for lhttpd files"
		" (default: /tmp)\n");
	fprintf(stderr, "  -l  File where all messages are written to"
		" (default: /tmp/lhttp.log)\n");
	fprintf(stderr, "  -d  Root directory for http files (default: .)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  If no IP is specified 0.0.0.0 is used\n");
	fprintf(stderr, "  Default port is 80\n");

	exit(EXIT_FAILURE);
}

void log_message(enum log_level level, const char *message, ...)
{
	if (level > verbosity)
		return;

	// Otworzenie pliku do logów
	FILE *logfd;
	if (foreground) {
		logfd = stderr;
	} else {
		logfd = fopen(log_file, "a");
		if (!logfd) {
			// perror("Cannot create log");
			fprintf(stderr, "Cannot create log\n");
			exit(EXIT_FAILURE);
		}
	}

	time_t now = time(NULL);
	if (now < 0) {
		fprintf(logfd, "Cannot get current time\n");
		exit(EXIT_FAILURE);
	}

	struct tm *local_now = localtime(&now);
	if (!local_now) {
		fprintf(logfd, "Cannot convert current time\n");
		exit(EXIT_FAILURE);
	}

	char timestamp[16];
	if (strftime(timestamp, sizeof(timestamp)
		, "%d %b %H:%M:%S", local_now) == 0)
		sprintf(timestamp, "timestamp error");

	char *level_string;
	switch (level) {
	case ERROR:
		level_string =  "error";
		break;
	case WARN:
		level_string =   "warn";
		break;
	case NOTICE:
		level_string = "notice";
		break;
	case INFO:
		level_string =   "info";
		break;
	case CONNECTION:
		level_string = "  conn";
		break;
	case DEBUG1:
		level_string =  "debug"; // Bardziej się wyróżnia bez cyfry
		break;
	case DEBUG2:
		level_string = "debug2";
		break;
	}

	if (leon)
		fprintf(logfd, "[%15s %6s] :33 < ", timestamp, level_string);
	else
		fprintf(logfd, "[%15s %6s] ", timestamp, level_string);

	// Pozwól na przekazywanie argumentów jak do efprintefa.
	va_list formats;
	va_start(formats, message);
	vfprintf(logfd, message, formats);
	va_end(formats);
	fprintf(logfd, "\n");

	if (!foreground)
		fclose(logfd);
}

void die(const char *reason)
{
	int errno_temp = errno;
	log_message(ERROR, "* %s (errno: %i) Exiting", reason, errno);

	errno = errno_temp;

	if (foreground)
		perror("die");

	exit(EXIT_FAILURE);
}
