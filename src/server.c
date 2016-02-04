#include "server.h"

typedef struct Server Server;

struct Server {
	int argc;
	int optind;
	char** argv;

	bool opt_is_help;
};

static char const PROGNAME[] = "cap server";
static char const DEFAULT_HOSTPORT[] = "127.0.0.1:1234";

static bool
server_parse_options(Server* self);

static void
server_delete(Server* self) {
	if (self) {
		free(self);
	}
}

static Server*
server_new(int argc, char* argv[]) {
	// Construct
	Server* self = (Server*) calloc(1, sizeof(Server));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Set values
	self->argc = argc;
	self->argv = argv;

	// Parse server options
	if (!server_parse_options(self)) {
		WARN("Failed to parse options");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

static bool
server_parse_options(Server* self) {
	// Parse options
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->opt_is_help = true; break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		WARN("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

static int
server_run(Server* self) {
	char const* hostport = DEFAULT_HOSTPORT;
	if (self->argc > self->optind) {
		hostport = self->argv[self->optind];
	}

	term_eputsf("CapServer running by \"%s\".", hostport);

	HttpHeader* header = httpheader_new();
	if (!header) {
		return caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "HttpHeader");
	}

	Socket* servsock = socket_open(hostport, "tcp-server");
	if (!servsock) {
		httpheader_delete(header);
		return caperr_printf(PROGNAME, CAPERR_OPEN, "socket");
	}

	term_eputsf("accept...");

	Socket* cliesock = socket_accept(servsock);
	if (!cliesock) {
		httpheader_delete(header);
		socket_close(servsock);
		return caperr_printf(PROGNAME, CAPERR_OPEN, "accept socket");
	}

	for (;;) {
		term_eputsf("recv...");

		char buf[1024];
		int nrecv = socket_recv_string(cliesock, buf, sizeof buf);
		if (nrecv < 0) {
			WARN("nrecv [%d]", nrecv);
			break;
		}

		term_eputsf("****************\n%s", buf);

		httpheader_parse_string(header, buf);

		if (strcasecmp(httpheader_method_name(header), "GET") == 0) {
			char const* reqpath = httpheader_method_value(header);
			Config* config = config_instance();

			char path[FILE_NPATH];
			config_path_with_home(config, path, sizeof path, reqpath);

			if (!file_is_exists(path)) {
				socket_send_string(cliesock,
					"HTTP/1.1 404\r\n"
					"\r\n"\
				);
				continue;
			}

			term_eputsf("open path [%s]", path);

			FILE* fin = file_open(path, "rb");
			if (!fin) {
				caperr_printf(PROGNAME, CAPERR_FOPEN, "\"%s\"", path);
				continue;
			}

			int contlen = 0;
			for (contlen = 0; fgetc(fin) != EOF; ++contlen) {
			}

			if (fseek(fin, 0L, SEEK_SET) == EOF) {
				caperr_printf(PROGNAME, CAPERR_ERROR, "Failed to seek");
				file_close(fin);
				continue;
			}

			char cont[contlen+1];
			fread(cont, sizeof(cont[0]), contlen, fin);
			cont[contlen] = '\0';
			file_close(fin);

			char tmp[128];
			snprintf(tmp, sizeof tmp, "Content-Length: %d\r\n", contlen);

			String* res = str_new();

			str_append_string(res,
				"HTTP/1.1 200 OK\r\n"
				"Server: CapServer (Prototype)\r\n"
				"Date: ^_^\r\n"
				"Content-Type: text/html; charset=utf-8\r\n"
				"Connection: keep-alive\r\n"
			);

			str_append_string(res, tmp);
			str_append_string(res, "\r\n");
			str_append_string(res, "<pre>\n");
			str_append_string(res, cont);
			str_append_string(res, "</pre>\n");

			term_eputsf("send...");
			socket_send_string(cliesock, str_get_const(res));

			str_delete(res);
		}

		if (strncmp(buf, "exit", 4) == 0) {
			break;
		}
	}

	httpheader_delete(header);
	socket_close(cliesock);
	socket_close(servsock);
	return 0;
}

/**************************
* server public interface *
**************************/

void
server_usage(void) {
    term_eprintf( 
        "Usage:\n"
        "\n"
        "\t%s [option]... [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help display usage\n"
        "\n"
    , PROGNAME);
}

int
server_main(int argc, char* argv[]) {
	// Construct
	Server* server = server_new(argc, argv);
	if (!server) {
		WARN("Failed to construct server");
		return EXIT_FAILURE;
	}

	// Run
	int res = server_run(server);

	// Done
	server_delete(server);
	return res;
}

/**************
* server test *
**************/

#if defined(TEST_SERVER)
int
main(int argc, char* argv[]) {
	return server_main(argc, argv);
}
#endif