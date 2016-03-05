#include "term.h"

void
term_destroy(void) {
}

int
term_init(void) {
	return 0;
}

void
term_flush(void) {
	fflush(stdout);
}

void
term_eflush(void) {
	fflush(stderr);
}

int
term_putc(int ch) {
	return fputc(ch, stdout);
}

int
term_eputc(int ch) {
	return fputc(ch, stderr);
}

int
term_printf(char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int len = vfprintf(stdout, fmt, args);

	va_end(args);

	return len;
}

int
term_putsf(char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int len = vfprintf(stdout, fmt, args);
	len += fprintf(stdout, "\n");

	va_end(args);

	return len;
}

int
term_eprintf(char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int len = vfprintf(stderr, fmt, args);
	fflush(stderr);

	va_end(args);

	return len;
}

int
term_eputsf(char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int len = vfprintf(stderr, fmt, args);
	len += fprintf(stderr, "\n");
	fflush(stderr);

	va_end(args);

	return len;
}

/********************
* term color family *
********************/

#if defined(_WIN32) || defined(_WIN64)
static WORD
attrtowinattrflag(TermAttr termattr) {
	switch (termattr) {
	default:
	case TA_RESET:
	case TA_DIM:
	case TA_BLINK:
	case TA_UNDERLINE:
	case TA_REVERSE:
	case TA_HIDDEN: return 0; break;
	case TA_BRIGHT: return FOREGROUND_INTENSITY; break;
	}
}

static WORD
fgtowincolorflag(TermColor termcolor) {
	switch (termcolor) {
	default:
	case TC_BLACK: return 0; break;
 	case TC_BLUE: return FOREGROUND_INTENSITY | FOREGROUND_BLUE; break;
    case TC_GREEN: return FOREGROUND_INTENSITY | FOREGROUND_GREEN; break;
    case TC_CYAN: return FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
    case TC_RED: return FOREGROUND_INTENSITY | FOREGROUND_RED; break;
    case TC_MAGENTA: return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE; break;
    case TC_YELLOW: return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN; break;
    case TC_WHITE: return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
	}
}

static WORD
bgtowincolorflag(TermColor termcolor) {
	switch (termcolor) {
	default:
	case TC_BLACK: return 0; break;
 	case TC_BLUE: return BACKGROUND_INTENSITY | BACKGROUND_BLUE; break;
    case TC_GREEN: return BACKGROUND_INTENSITY | BACKGROUND_GREEN; break;
    case TC_CYAN: return BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE; break;
    case TC_RED: return BACKGROUND_INTENSITY | BACKGROUND_RED; break;
    case TC_MAGENTA: return BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE; break;
    case TC_YELLOW: return BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN; break;
    case TC_WHITE: return BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE; break;
	}
}
#endif /* For Windows */

/** 
 * From http://www.linuxjournal.com/article/8603
 */
void
term_ftextcolor(FILE* fout, TermAttr attr, TermColor fg, TermColor bg) {
	char cmd[13];
	snprintf(cmd, sizeof cmd, "%c[%d;%d;%dm", 0x1b, attr, fg+30, bg+40);
	fprintf(fout, "%s", cmd);
}

static int
_acfprintf(FILE* fout, TermAttr attr, TermColor fg, TermColor bg, char const* fmt, va_list args) {
	// Set state
#if defined(_WIN32) || defined(_WIN64)
	DWORD nhandle = STD_OUTPUT_HANDLE;
	if (fout == stderr) {
		nhandle = STD_ERROR_HANDLE;
	} else if (fout != stdout) {
		fprintf(stderr, "cap: term: Invalid stream in acfprintf.");
	}

	HANDLE stdh = GetStdHandle(nhandle);
	WORD oldattr = 0;
	WORD setattr = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	// Save state
	GetConsoleScreenBufferInfo(stdh, &csbi);
	oldattr = csbi.wAttributes;
	setattr = attrtowinattrflag(attr) | fgtowincolorflag(fg) | bgtowincolorflag(bg);
	SetConsoleTextAttribute(stdh, setattr);
#else
	term_ftextcolor(fout, attr, fg, bg);
#endif

	// Draw
	int len = vfprintf(fout, fmt, args);

	// Undo state
#if defined(_WIN32) || defined(_WIN64)
	SetConsoleTextAttribute(stdh, oldattr);
#else
	term_ftextcolor(fout, TA_RESET, fg, bg);
#endif

	return len;	
}

int
term_acprintf(TermAttr attr, TermColor fg, TermColor bg, char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int len = _acfprintf(stdout, attr, fg, bg, fmt, args);	
	va_end(args);
	return len;
}

int
term_cprintf(TermColor fg, TermColor bg, char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int len = _acfprintf(stdout, TA_BRIGHT, fg, bg, fmt, args);	
	va_end(args);
	return len;
}

int
term_aceprintf(TermAttr attr, TermColor fg, TermColor bg, char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int len = _acfprintf(stderr, attr, fg, bg, fmt, args);	
	va_end(args);
	return len;
}

int
term_ceprintf(TermColor fg, TermColor bg, char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int len = _acfprintf(stderr, TA_BRIGHT, fg, bg, fmt, args);	
	va_end(args);
	return len;
}

/************
* term test *
************/

#if defined(TEST_TERM)
static int
test_cprintf(int argc, char* argv[]) {
	char const* src = argv[0];
	if (argc >= 2) {
		src = argv[1];
	}

	term_cprintf(TC_GREEN, TC_YELLOW, "%s\n", src);
	term_cprintf(TC_YELLOW, TC_RED, "%s\n", src);
	term_cprintf(TC_BLACK, TC_WHITE, "%s\n", src);

	term_cprintf(TC_BLACK, TC_WHITE, "UNDERLINE %s\n", src);
	term_acprintf(TA_BRIGHT, TC_WHITE, TC_BLACK, "BRIGHT %s\n", src);
	term_acprintf(TA_DIM, TC_WHITE, TC_BLACK, "DIM %s\n", src);
	term_acprintf(TA_UNDERLINE, TC_RED, TC_BLACK, "UNDERLINE %s\n", src);
	term_acprintf(TA_BLINK, TC_GREEN, TC_BLACK, "BLINK %s\n", src);
	term_acprintf(TA_REVERSE, TC_BLUE, TC_BLACK, "REVERSE %s\n", src);
	term_acprintf(TA_HIDDEN, TC_BLACK, TC_YELLOW, "HIDDEN %s\n", src);

	return 0;
}

int
main(int argc, char* argv[]) {
	int ret = 0;
	ret = test_cprintf(argc, argv);
	fflush(stdout);
	fflush(stderr);
    return ret;
}
#endif
