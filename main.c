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

#include "main.h"
#include "lsocket.h"

#include <errno.h>



/* Zmienne globalne **********************************************************/

char *program_name;

const char DEFAULT_LOG_FILE[] = "/tmp/lhttpd.log";
const char DEFAULT_CONF_FILE[] = "/etc/lhttpd.conf";

enum log_level verbosity = DEBUG2; //INFO;
int foreground = 1; //// Tymczasowo ******************
int leon = 0;

int server; // Deskryptor pliku dla gniazda
/*__thread*/ int client;

char *base_path = ".";


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
	// TODO WARN/NOTICE jeżeli port < 1024
	create_server("localhost", "8080");

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
		fprintf(stderr, "%s: cannot change working directory\n", program_name);
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
	while ((option = getopt(argc, argv, "hvf3")) != -1) {
		switch (option) {
		case 'v':
			verbosity++; // Nie podasz więcej niż int? ;p
			break;
		case 'f':
			foreground = 1;
			break;
		case 'd':
			base_path = optarg;
		case '3':
			leon = 1;
			break;
		case 'h':
		default:
			usage();
		}
	}
}

void usage()
{
	fprintf(stderr, "Usage: %s [-h] [-v] [-f] [-d working_directory]\n", program_name);
	fprintf(stderr, "  -h  Display this helpful text\n");
	fprintf(stderr, "  -v  Verbosity level, more `v', more verbose\n");
	fprintf(stderr, "  -f  Stay in foreground (don't demonize)\n");
	fprintf(stderr, "  -d  Root directory for http files (. default)\n");
	exit(EXIT_FAILURE);
}

void log_message(enum log_level level, const char *message, ...)
{
	if (level > verbosity)
		return;

	// Otworzenie pliku do logów
	FILE *log_file;
	if (foreground) {
		log_file = stderr;
	} else {
		log_file = fopen(DEFAULT_LOG_FILE, "a");
		if (!log_file) {
			// fprintf(stderr, "%s: cannot create log file\n", program_name);
			// perror("Cannot create log");
			fprintf(stderr, "Cannot create log\n");
			exit(EXIT_FAILURE);
		}
	}

	time_t now = time(NULL);
	if (now < 0) {
		fprintf(log_file, "Cannot get current time\n");
		exit(EXIT_FAILURE);
	}

	struct tm *local_now = localtime(&now);
	if (!local_now) {
		fprintf(log_file, "Cannot convert current time\n");
		exit(EXIT_FAILURE);
	}

	char timestamp[16];
	if (strftime(timestamp, sizeof(timestamp), "%d %b %H:%M:%S", local_now) == 0)
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
	case DEBUG1:
		level_string =  "debug"; // Bardziej się wyróżnia bez cyfry
		break;
	case DEBUG2:
		level_string = "debug2";
		break;
	}

	if (leon)
		fprintf(log_file, "[%15s %6s] :33 < ", timestamp, level_string);
	else
		fprintf(log_file, "[%15s %6s] ", timestamp, level_string);

	// Pozwól na przekazywanie argumentów jak do efprintefa.
	va_list formats;
	va_start(formats, message);
	vfprintf(log_file, message, formats);
	va_end(formats);
	fprintf(log_file, "\n");

	if (!foreground)
		fclose(log_file);
}

void die(const char *reason)
{
	int temp = errno;
	log_message(ERROR, "* %s (errno: %i) Exiting", reason, errno);

	errno = temp;

	if (foreground)
		perror("die");

	// switch (errno) {
	// case EACCES:
	// 	log_message(ERROR,
	// 		"Access denied, maybe you need root priviledges");
	// 	break;
	// default:
	// 	log_message(ERROR, "The programmer was lazy and he didn't"
	// 		" specify a message for this error code :(");
	// }

	exit(EXIT_FAILURE);
}
