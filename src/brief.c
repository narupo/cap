#include "brief.h"

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	int optind;
	char** argv;

	StringArray* briefs;
	StringArray* fnames;

	bool optis_help;
	bool optis_disp_all;
};

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self) {
	if (self) {
		strarray_delete(self->briefs);
		strarray_delete(self->fnames);
		free(self);
	}
}

static Command*
command_new(int argc, char* argv[]) {
	// Construct
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		perror("Failed to construct");
		return NULL;
	}

	// Set values
	self->name = "cap brief";
	self->argc = argc;
	self->argv = argv;

	self->briefs = strarray_new();
	if (!self->briefs) {
		WARN("Failed to construct StringArray");
		goto fail_briefs;
	}

	self->fnames = strarray_new();
	if (!self->fnames) {
		WARN("Failed to construct StringArray");
		goto fail_fnames;
	}

	// Parse command options
	if (!command_parse_options(self)) {
		perror("Failed to parse options");
		goto fail_parse_options;
	}

	// Done
	return self;

fail_briefs:
	free(self);

fail_fnames:
	strarray_delete(self->briefs);

fail_parse_options:
	strarray_delete(self->fnames);
	
	return NULL;
}

static bool
command_parse_options(Command* self) {
	// Parse options
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{"all", no_argument, 0, 'a'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "ha", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->optis_help = true; break;
		case 'a': self->optis_disp_all = true; break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		perror("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

static FILE*
command_open_stream(Command const* self, char const* fname) {
	// Ready
	Config* config = config_instance();
	if (!config) {
		WARN("Failed to construct config");
		return NULL;
	}

	// Make path
	char path[NFILE_PATH];
	snprintf(path, NUMOF(path), "%s/%s", config_path(config, "cd"), fname);

	if (file_is_dir(path)) {
		term_eputsf("%s: Can't open file. \"%s\" is a directory.", self->name, path);
		return NULL;
	}

	if (!file_is_exists(path)) {
		term_eputsf("%s: Not found file \"%s\".", self->name, path);
		return NULL;
	}

	return file_open(path, "rb");
}

static int
command_read_from_stream(Command* self, FILE* fin, char const* fname) {
	// Ready
	Buffer* buf = buffer_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		goto fail_buffer;
	}

	CapParser* parser = capparser_new();
	if (!parser) {
		WARN("Failed to construct parser");
		goto fail_parser;
	}

	// Read briefs in file
	for (; buffer_getline(buf, fin); ) {
		char const* line = buffer_get_const(buf);
		CapRow* row = capparser_parse_line(parser, line);
		if (!row) {
			continue;
		}

		CapColType type = caprow_front_type(row);
		if (type == CapColBrief) {
			// Save
			char const* val = capcol_value_const(caprow_front(row));
			strarray_push_copy(self->briefs, val);
			strarray_push_copy(self->fnames, fname);
	
			if (!self->optis_disp_all) {
				caprow_delete(row);
				break;
			}
		}

		caprow_delete(row);
	}

	capparser_delete(parser);
	buffer_delete(buf);
	return 0;

fail_parser:
	buffer_delete(buf);

fail_buffer:
	return 1;
}

static int
command_run(Command* self) {
	// Check argument
	if (self->optis_help) {
		brief_usage();
		return 0;
	}

	// Read from streams
	int maxfnamelen = 0;

	if (self->argc == self->optind) {
		command_read_from_stream(self, stdin, "");

	} else {
		for (int i = self->optind; i < self->argc; ++i) {
			char const* fname = self->argv[i];
			size_t fnamelen = strlen(fname);
			maxfnamelen = (fnamelen > maxfnamelen ? fnamelen : maxfnamelen);

			FILE* fin = command_open_stream(self, fname);
			if (!fin) {
				WARN("Failed to open file \"%s\"", fname);
				goto fail_open_file;
			}

			command_read_from_stream(self, fin, fname);

			file_close(fin);
		}
	}

	// Display
	char const* prevfname = NULL;

	for (int i = 0; i < strarray_length(self->fnames); ++i) {
		char const* fname = strarray_get_const(self->fnames, i);
		char const* brief = strarray_get_const(self->briefs, i);
		size_t brieflen = strlen(brief);
		size_t fnamelen = strlen(fname);
		
		if (prevfname && strcmp(prevfname, fname) == 0) {
			term_printf("%-*s %-*s%s", fnamelen, "", maxfnamelen-fnamelen, "", brief);
		} else if (fnamelen) {
			term_printf("%s %-*s%s", fname, maxfnamelen-fnamelen, "", brief);
		} else {
			term_printf("%-*s%s", maxfnamelen-fnamelen, "", brief);
		}
		if (brief[brieflen-1] != '.') {
			term_printf(".");
		}
		term_printf("\n");

		prevfname = fname;
	}

	// Done
	return 0;

fail_open_file:
	return 1;
}

/*************************
* Brief public interface *
*************************/

void
brief_usage(void) {
    term_eprintf(
        "cap brief\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap brief [file]... [option]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help display usage\n"
        "\t-a, --all  display all briefs\n"
        "\n"
    );
}

int
brief_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		perror("Failed to construct command");
		return EXIT_FAILURE;
	}

	// Run
	int res = command_run(command);

	// Done
	command_delete(command);
	return res;
}

/*************
* Brief test *
*************/

#if defined(TEST_PROGRAM)
int
main(int argc, char* argv[]) {
	return brief_main(argc, argv);
}
#endif