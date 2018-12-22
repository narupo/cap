/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "string.h"

/********
* utils *
********/

char *
strapp(char *dst, int32_t dstsz, const char *src) {
	if (!dst || dstsz <= 0 || !src) {
		return NULL;
	}

	const char *dend = dst+dstsz-1; // -1 for final nul
	char *dp = dst + strlen(dst);

	for (const char *sp = src; *sp && dp < dend; *dp++ = *sp++) {
	}	
	*dp = '\0';
	
	return dst;
}

char *
strcpywithout(char *dst, int32_t dstsz, const char *src, const char *without) {
	if (!dst || dstsz <= 0 || !src || !without) {
		return NULL;
	}
	
	int32_t di = 0;
	for (const char *p = src; *p; ++p) {
		if (strchr(without, *p)) {
			continue;
		}
		if (di >= dstsz-1) {
			dst[di] = '\0';
			return NULL;
		}
		dst[di++] = *p;
	}
	dst[di] = '\0';
	return dst;
}

/*******************
* string structure *
*******************/

struct string {
	int length;
	int capacity;
	string_type_t *buffer;
};

/*************
* str macros *
*************/

#define NCHAR (sizeof(string_type_t))
#define NIL ('\0')

/**************************
* str constant variabless *
**************************/

enum {
	NCAPACITY = 4,
};

/*********************
* str delete and new *
*********************/

void
str_del(string *self) {
	if (self) {
		free(self->buffer);
		free(self);
	}
}

string_type_t *
str_escdel(string *self) {
	if (!self) {
		return NULL;
	}
	
	char *buf = self->buffer;
	free(self);
	return buf;
}

string *
str_new(void) {
	string *self = calloc(1, sizeof(string));
	if (!self) {
		return NULL;
	}

	self->length = 0;
	self->capacity = NCAPACITY;
	self->buffer = calloc(self->capacity + 1, NCHAR);
	if (!self->buffer) {
		free(self);
		return NULL;
	}

	self->buffer[self->length] = NIL;

	return self;
}

string *
str_newother(const string *other) {
	if (!other) {
		return NULL;
	}

	string *self = calloc(1, sizeof(string));
	if (!self) {
		return NULL;
	}

	self->length = other->length;
	self->capacity = other->capacity;
	self->buffer = calloc(self->capacity + 1, NCHAR);
	if (!self->buffer) {
		free(self);
		return NULL;
	}

	for (int i = 0; i < self->length; ++i) {
		self->buffer[i] = other->buffer[i];
	}
	self->buffer[self->length] = NIL;

	return self;
}

/*************
* str getter *
*************/

int32_t
str_len(const string *self) {
	if (!self) {
		return -1;
	}
	return self->length;
}

int32_t
str_capa(const string *self) {
	if (!self) {
		return -1;
	}
	return self->capacity;
}

const string_type_t *
str_getc(const string *self) {
	if (!self) {
		return NULL;
	}
	return self->buffer;
}

int32_t
str_empty(const string *self) {
	if (!self) {
		return 0;
	}
	return self->length == 0;
}

/*************
* str setter *
*************/

void
str_clear(string *self) {
	if (!self) {
		return;
	}

	self->length = 0;
	self->buffer[self->length] = NIL;
}

string *
str_set(string *self, const string_type_t *src) {
	if (!self || !src) {
		return NULL;
	}

	int srclen = strlen(src);
	if (srclen >= self->length) {
		if (!str_resize(self, srclen)) {
			return NULL;
		}
	}
	self->length = srclen;

	for (int i = 0; i < srclen; ++i) {
		self->buffer[i] = src[i];
	}
	self->buffer[srclen] = NIL;

	return self;
}

string *
str_resize(string *self, int32_t newcapa) {
	if (!self) {
		return NULL;
	}

	if (newcapa < 0) {
		newcapa = 0;
	}

	string_type_t *tmp = realloc(self->buffer, newcapa*NCHAR + NCHAR); // +NCHAR for final nil
	if (!tmp) {
		str_del(self);
		return NULL;
	}

	self->buffer = tmp;
	self->capacity = newcapa;
	if (newcapa < self->length) {
		self->length = newcapa;
		self->buffer[self->length] = NIL;
	}

	return self;
}

string *
str_pushb(string *self, string_type_t ch) {
	if (!self || ch == NIL) {
		return NULL;
	}

	if (self->length >= self->capacity-1) {
		if (!str_resize(self, self->length*2)) {
			return NULL;
		}
	}

	self->buffer[self->length++] = ch;
	self->buffer[self->length] = NIL;

	return self;
}

string_type_t
str_popb(string *self) {
	if (!self) {
		return NIL;
	}

	if (self->length > 0) {
		string_type_t ret = self->buffer[--self->length];
		self->buffer[self->length] = NIL;
		return ret;
	}

	return NIL;
}

string *
str_pushf(string *self, string_type_t ch) {
	if (!self || ch == NIL) {
		return NULL;
	}

	if (self->length >= self->capacity-1) {
		if (!str_resize(self, self->length*2)) {
			return NULL;
		}
	}

	for (int32_t i = self->length; i > 0; --i) {
		self->buffer[i] = self->buffer[i-1];
	}

	self->buffer[0] = ch;
	self->buffer[++self->length] = NIL;
	return self;
}

string_type_t
str_popf(string *self) {
	if (!self || self->length == 0) {
		return NIL;
	}

	string_type_t ret = self->buffer[0];

	for (int32_t i = 0; i < self->length-1; ++i) {
		self->buffer[i] = self->buffer[i+1];
	}

	--self->length;
	self->buffer[self->length] = NIL;

	return ret;
}

string *
str_app(string *self, const string_type_t *src) {
	if (!self || !src) {
		return NULL;
	}

	int32_t srclen = strlen(src);

	if (self->length + srclen >= self->capacity-1) {
		if (!str_resize(self, (self->length + srclen) * 2)) {
			return NULL;
		}
	}

	for (const string_type_t *sp = src; *sp; ++sp) {
		self->buffer[self->length++] = *sp;
	}
	self->buffer[self->length] = NIL;

	return self;
}

string *
str_appstream(string *self, FILE *fin) {
	if (!self || !fin) {
		return NULL;
	}
	
	for (int32_t ch; (ch = fgetc(fin)) != EOF; ) {
		if (!str_pushb(self, ch)) {
			return NULL;
		}
	}

	return self;
}

string *
str_appother(string *self, const string *other) {

	if (!self || !other) {
		return NULL;
	}

	string *ret = NULL;

	if (self == other) {
		string_type_t *buf = (string_type_t *) strdup(self->buffer);
		if (!buf) {
			return ret;
		}
		ret = str_app(self, buf);
		free(buf);
	} else {
		ret = str_app(self, other->buffer);
	}

	return ret;
}

string *
str_appfmt(string *self, string_type_t *buf, int32_t nbuf, const string_type_t *fmt, ...) {
	if (!self || !buf || !fmt || nbuf == 0) {
		return NULL;
	}

	va_list args;
	va_start(args, fmt);
	int32_t buflen = vsnprintf(buf, nbuf, fmt, args);
	va_end(args);

	for (int32_t i = 0; i < buflen; ++i) {
		if (!str_pushb(self, buf[i])) {
			return NULL;
		}
	}

	return self;
}

string *
str_rstrip(string *self, const string_type_t *rems) {
	if (!self || !rems) {
		return NULL;
	}

	for (int32_t i = self->length-1; i > 0; --i) {
		if (strchr(rems, self->buffer[i])) {
			self->buffer[i] = NIL;
		} else {
			break;
		}
	}

	return self;
}

string *
str_lstrip(string *self, const string_type_t *rems) {
	if (!self || !rems) {
		return NULL;
	}

	for (; self->length; ) {
		if (strchr(rems, self->buffer[0])) {
			str_popf(self);
		} else {
			break;
		}
	}

	return self;
}

string *
str_strip(string *self, const string_type_t *rems) {
	if (!str_rstrip(self, rems)) {
		return NULL;
	}
	if (!str_lstrip(self, rems)) {
		return NULL;
	}
	return self;
}

/****************
* str algorithm *
****************/

#define MAX(a, b) (a > b ? a : b)
/**
 * Boyer-Moore search at first
 *
 * @param[in]  tex	  Target string
 * @param[in]  texlen Target length
 * @param[in]  pat	  Pattern string
 * @param[in]  patlen Pattern length
 * @return		  Success to pointer to found position in target string
 * @return		  Failed to NULL
 */
static const char *
bmfind(
	const char *restrict tex,
	int32_t texlen,
	const char *restrict pat,
	int32_t patlen
) {
	int32_t const max = CHAR_MAX+1;
	ssize_t texpos = 0;
	ssize_t patpos = 0;
	int32_t table[max];

	if (texlen < patlen || patlen <= 0) {
		return NULL;
	}

	for (int32_t i = 0; i < max; ++i) {
		table[i] = patlen;
	}

	for (int32_t i = 0; i < patlen; ++i) {
		table[ (int32_t)pat[i] ] = patlen-i-1;
	}

	texpos = patlen-1;

	while (texpos <= texlen) {
		int32_t curpos = texpos;
		patpos = patlen-1;
		while (tex[texpos] == pat[patpos]) {
			if (patpos <= 0) {
				return tex + texpos;
			}
			--patpos;
			--texpos;
		}
		int32_t index = (int32_t)tex[texpos];
		texpos = MAX(curpos+1, texpos + table[ index ]);
	}
	return NULL;
}
#undef MAX

const char *
str_findc(const string *self, const char *target) {
	if (!self || !target) {
		return NULL;
	}

	return bmfind(self->buffer, self->length, target, strlen(target));
}

/**************
* str cleanup *
**************/

#undef NCHAR
#undef NIL

/***********
* str test *
***********/

#if defined(_TEST_STRING)
#include <ctype.h>

/************
* test args *
************/

struct args {
	int capa;
	int len;
	char **args;
};

static void
argsdel(struct args *self) {
	if (!self) {
		return;
	}

	for (int i = 0; i < self->len; ++i) {
		free(self->args[i]);
	}
	free(self->args);
	free(self);
}

static char **
argsescdel(struct args *self) {
	if (!self) {
		return NULL;
	}

	char **args = self->args;
	free(self);

	return args;
}

static struct args *
argsnew(void) {
	struct args *self = calloc(1, sizeof(struct args));
	if (!self) {
		return NULL;
	}

	self->capa = 4;
	self->args = calloc(self->capa+1, sizeof(char *));
	if (!self) {
		return NULL;
	}

	return self;
}

static struct args *
argsresize(struct args *self, int newcapa) {
	if (!self || newcapa <= self->capa) {
		return NULL;
	}

	char **tmp = realloc(self->args, sizeof(char *) * newcapa + sizeof(char *));
	if (!tmp) {
		return NULL;
	}

	self->capa = newcapa;
	self->args = tmp;

	return self;
}

static struct args *
argspush(struct args *self, const char *arg) {
	if (!self || !arg) {
		return NULL;
	}

	if (self->len >= self->capa) {
		if (!argsresize(self, self->capa*2)) {
			return NULL;
		}
	}

	char *cp = strdup(arg);
	if (!cp) {
		return NULL;
	}

	self->args[self->len++] = cp;
	self->args[self->len] = NULL;

	return self;
}

static void
argsclear(struct args *self) {
	if (!self) {
		return;
	}

	for (int i = 0; i < self->len; ++i) {
		free(self->args[i]);
		self->args[i] = NULL;
	}	

	self->len = 0;
}

static const char *
argsgetc(const struct args *self, int idx) {
	if (idx >= self->len) {
		return NULL;
	}
	return self->args[idx];
}

static int 
argslen(const struct args *self) {
	return self->len;
}

static void
freeargv(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i) {
		free(argv[i]);
	}
	free(argv);
}

static void
showargv(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i) {
		printf("[%d] = '%s'\n", i, argv[i]);
	}
}

static void
push(struct args *args, string *str) {
	if (str_empty(str)) {
		return;
	}

	if (!argspush(args, str_getc(str))) {
		perror("argspush");
		exit(1);
	}

	str_clear(str);
}

static struct args *
makeargsby(const char *line) {
	struct args *args = argsnew();
	if (!args) {
		perror("argsnew");
		return NULL;
	}

	string *tmp = str_new();
	if (!tmp) {
		argsdel(args);
		perror("str_new");
		return NULL;
	}

	const char *p = line;
	int m = 0;
	do {
		int c = *p;
		if (c == '\0') {
			c = ' ';
		}

		switch (m) {
		case 0: // First
			if (isspace(c)) {
				push(args, tmp);
			} else {
				str_pushb(tmp, c);
			}
		break;
		}
	} while (*p++);

	str_del(tmp);
	return args;
}

/************
* test main *
************/

static string *kstr;

static int
test_del(int argc, char *argv[]) {
	str_del(kstr);
	kstr = str_new();
	if (!kstr) {
		return 1;
	}

	return 0;
}

static int
test_escdel(int argc, char *argv[]) {
	string_type_t *buf = str_escdel(kstr);
	if (!buf) {
		return 1;
	}

	free(buf);

	kstr = str_new();
	if (!kstr) {
		return 2;
	}

	return 0;
}

static int
test_new(int argc, char *argv[]) {
	str_del(kstr);
	kstr = str_new();
	if (!kstr) {
		return 1;
	}

	if (argc >= 2) {
		str_set(kstr, argv[1]);
	}

	return 0;
}

static int
test_newother(int argc, char *argv[]) {
	if (!kstr) {
		return 1;
	}

	string *dst = str_newother(kstr);
	if (!dst) {
		return 2;
	}

	if (strcmp(kstr->buffer, dst->buffer) != 0) {
		str_del(dst);
		return 3;
	}

	str_del(dst);
	return 0;
}

static int
test_len(int argc, char *argv[]) {
	printf("len: %d\n", str_len(kstr));
	return 0;
}

static int
test_capa(int argc, char *argv[]) {
	printf("capa: %d\n", str_capa(kstr));
	return 0;
}

static int
test_getc(int argc, char *argv[]) {
	const char *s = str_getc(kstr);
	if (!s) {
		return 1;
	}

	return 0;
}

static int
test_empty(int argc, char *argv[]) {
	printf("empty: %s\n", (str_empty(kstr) ? "true" : "false"));
	return 0;
}

static int
test_clear(int argc, char *argv[]) {
	str_clear(kstr);
	return 0;
}

static int
test_set(int argc, char *argv[]) {
	if (argc < 2) {
		str_set(kstr, "");
	} else {
		str_set(kstr, argv[1]);
	}
	return 0;
}

static int
test_resize(int argc, char *argv[]) {
	if (argc < 2) {
		return 0;
	}
	
	int n = atoi(argv[1]);
	if (n <= 0) {
		return 1;
	}

	str_resize(kstr, n);
	return 0;
}

static int
test_pushb(int argc, char *argv[]) {
	if (argc < 2) {
		return 2;
	}

	for (int i = 0, len = strlen(argv[1]); i < len; ++i) {
		str_pushb(kstr, argv[1][i]);
	}

	return 0;
}

static int
test_popb(int argc, char *argv[]) {
	if (!str_empty(kstr)) {
		if (str_popb(kstr) == '\0') {
			return 2;
		}
	}

	return 0;
}

static int
test_pushf(int argc, char *argv[]) {
	return 0;
}

static int
test_popf(int argc, char *argv[]) {
	return 0;
}

static int
test_app(int argc, char *argv[]) {
	return 0;
}

static int
test_appstream(int argc, char *argv[]) {
	return 0;
}

static int
test_appother(int argc, char *argv[]) {
	return 0;
}

static int
test_appfmt(int argc, char *argv[]) {
	return 0;
}

static int
test_rstrip(int argc, char *argv[]) {
	return 0;
}

static int
test_lstrip(int argc, char *argv[]) {
	return 0;
}

static int
test_strip(int argc, char *argv[]) {
	return 0;
}

static int
test_findc(int argc, char *argv[]) {
	return 0;
}

int
main(int argc, char *argv[]) {
	static const struct cmd {
		const char *name;
		int (*run)(int, char**);
	} cmds[] = {
		{"del", test_del}, 
		{"escdel", test_escdel}, 
		{"new", test_new}, 
		{"newother", test_newother}, 
		{"len", test_len}, 
		{"capa", test_capa}, 
		{"getc", test_getc}, 
		{"empty", test_empty}, 
		{"clear", test_clear}, 
		{"set", test_set}, 
		{"resize", test_resize}, 
		{"pushb", test_pushb}, 
		{"popb", test_popb}, 
		{"pushf", test_pushf}, 
		{"popf", test_popf}, 
		{"app", test_app}, 
		{"appstream", test_appstream}, 
		{"appother", test_appother}, 
		{"appfmt", test_appfmt}, 
		{"rstrip", test_rstrip}, 
		{"lstrip", test_lstrip}, 
		{"strip", test_strip}, 
		{"findc", test_findc},
		{},
	};

	kstr = str_new();
	if (!kstr) {
		goto error;
	}

	// Command loop
	char cmdline[256];
	for (; fgets(cmdline, sizeof cmdline, stdin); ) {
		// Remove newline
		int32_t cmdlen = strlen(cmdline);
		if (cmdline[cmdlen-1] == '\n') {
			cmdline[--cmdlen] = '\0';
		}

		if (!strcasecmp(cmdline, "q")) {
			goto done;
		} else if (!strcasecmp(cmdline, "h")) {
			for (const struct cmd *p = cmds; p->name; ++p) {
				printf("%s\n", p->name);
			}
			continue;
		}

		// Command line to args
		struct args *args = makeargsby(cmdline);
		if (!args || argslen(args) <= 0) {
			argsdel(args);
			goto error;
		}
		int cmdargc = argslen(args);
		char **cmdargv = argsescdel(args);

		// Find command by input line. And execute
		int status = -1;
		for (const struct cmd *p = cmds; p->name; ++p) {
			if (!strcmp(p->name, cmdargv[0])) {
				// showargv(cmdargc, cmdargv);
				status = p->run(cmdargc, cmdargv);
				break;
			}
		}

		freeargv(cmdargc, cmdargv);

		// Check status
		switch (status) {
		case 0: fprintf(stderr, "ok: '%s': str buf[%s]\n", cmdline, str_getc(kstr)); break;
		case -1: fprintf(stderr, "failed: not found command '%s': str buf[%s]\n", cmdline, str_getc(kstr)); goto error;
		default: fprintf(stderr, "failed: status %d: str buf[%s]\n", status, str_getc(kstr)); goto error;
		}
	}

done:
	fprintf(stderr, "done: %s\n", str_getc(kstr));
	str_del(kstr);
	return 0;

error:
	perror("error");
	str_del(kstr);
	return 1;
}
#endif