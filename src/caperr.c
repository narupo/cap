#include "caperr.h"

enum {
	NSTACK = 20,
	NFNAME = 32,
	NFUNCNAME = 32,
	NHEADER = 32,
	NMESSAGE = 64,
};

typedef struct {
	int saveerrno;
	int number;
	int lineno;
	char fname[NFNAME];
	char funcname[NFUNCNAME];
	char header[NHEADER];
	char message[NMESSAGE];
} CapErr;

static struct {
	int stack_top;
	CapErr stack[NSTACK];
} caperrs;

static pthread_mutex_t
caperrs_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool
caperrs_lock(void) {
	if (pthread_mutex_lock(&caperrs_mutex) == 0) {
		return true;
	}

	perror("pthread_mutex_lock");
	return false;
}

static bool
caperrs_unlock(void) {
	if (pthread_mutex_unlock(&caperrs_mutex) == 0) {
		return true;
	}

	perror("pthread_mutex_unlock");
	return false;
}

static void
caperrs_push(
	int saveerrno,
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	va_list args) {

#if !defined(DEBUG)
	if (number == CAPERR_DEBUG) {
		return;
	}
#endif

	if (!caperrs_lock()) {
		return;
	}

	if (caperrs.stack_top >= NSTACK) {
		term_eputsf("caperr: Stack overflow");

	} else {
		CapErr* s = &caperrs.stack[caperrs.stack_top++];
		s->saveerrno = saveerrno;
		s->number = number;
		s->lineno = lineno;
		snprintf(s->fname, sizeof(s->fname), "%s", fname);
		snprintf(s->funcname, sizeof(s->funcname), "%s", funcname);
		snprintf(s->header, sizeof(s->header), "%s", header);
		vsnprintf(s->message, sizeof(s->message), fmt, args);
	}

	caperrs_unlock();
}

static CapErr const*
caperrs_pop(void) {
	if (!caperrs_lock()) {
		return NULL;
	}

	if (caperrs.stack_top <= 0) {
		caperrs_unlock();
		return NULL;
	}

	caperrs_unlock();
	return &caperrs.stack[--caperrs.stack_top];
}

int
_caperr(
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	...) {

	va_list args;

	va_start(args, fmt);
	caperrs_push(errno, fname, funcname, lineno, header, number, fmt, args);
	va_end(args);

	return number;
}

char const*
caperr_to_string(int number) {
	switch (number) {
		case CAPERR_DEBUG: return "debug: "; break;
		case CAPERR_ERROR: return ""; break;
		case CAPERR_NOTFOUND: return "Not found"; break;
		case CAPERR_CONSTRUCT: return "Failed to construct"; break;
		
		case CAPERR_FOPEN: return "Failed to open file"; break;
		case CAPERR_OPEN: return "Failed to open"; break;
		case CAPERR_OPENDIR: return "Failed to open directory"; break;

		case CAPERR_SYNTAX: return "Syntax error"; break;

		case CAPERR_PARSE: return "Failed to parse"; break;
		case CAPERR_PARSE_OPTIONS: return "Failed to parse options"; break;

		case CAPERR_INVALID: return "Invalid"; break;
		case CAPERR_INVALID_ARGUMENTS: return "Invalid arguments"; break;
		
		case CAPERR_READ: return "Failed to read"; break;
		case CAPERR_READDIR: return "Failed to read directory"; break;

		case CAPERR_WRITE: return "Failed to write"; break;
		case CAPERR_EXECUTE: return "Failed to execute"; break;

		default: return "Unknown errors"; break;
	}
}

static void
caperr_display_record(FILE* stream, CapErr const* e) {

#ifdef DEBUG
	fprintf(stream, "%s: %s: %d: ", e->fname, e->funcname, e->lineno);
#endif

	// Display header and number
	fprintf(stream, "%s: %s", e->header, caperr_to_string(e->number));

	// Display user's message
	size_t msglen = strlen(e->message);

	if (!msglen) {
		fprintf(stream, ".");

	} else {

		if (e->number != CAPERR_ERROR) {
			fputc(' ', stream);
		}

		// Display
		fprintf(stream, "%s", e->message);
		
		// Fix tail format of string
		if (e->message[msglen-1] != '.') {
			fprintf(stream, ".");
		}
	}
	
	// Display by errno
	if (e->saveerrno != 0) {
		fprintf(stream, " %s.", strerror(e->saveerrno));
	}
	
	// Done
	fprintf(stream, "\n");
}

void
caperr_display(FILE* stream) {
	// Stack trace
	for (CapErr const* e = caperrs_pop(); e; e = caperrs_pop()) {
		caperr_display_record(stream, e);
	}
}

void
caperr_display_first(FILE* stream) {
	if (caperrs.stack_top != 0) {
		caperr_display_record(stream, &caperrs.stack[0]);		
	}
}

void
caperr_display_last(FILE* stream) {
	if (caperrs.stack_top != 0) {
		caperr_display_record(stream, &caperrs.stack[caperrs.stack_top-1]);
	}
}

#if defined(TEST_CAPERR)
int
main(int argc, char* argv[]) {
	caperr(argv[0], CAPERR_FOPEN, "not found file \"%s\"", argv[0]);
	caperr_display();
    return 0;
}
#endif
