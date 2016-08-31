/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-run.h"

enum {
	NSCRIPTNAME = 100,
	NCMDLINE = 256
};

static char *
readscriptline(char *dst, size_t dstsz, const char *path) {
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		return NULL;
	}

	char tmp[dstsz];
	cap_fgetline(tmp, sizeof tmp, fin);

	const char *needle = "!";
	char *at = strstr(tmp, needle);
	if (!at) {
		fclose(fin);
		return NULL;
	}

	snprintf(dst, dstsz, "%s", at + strlen(needle));

	if (fclose(fin) < 0) {
		return NULL;
	}

	return dst;
}

int
main(int argc, char *argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap run");

	if (argc < 2) {
		cap_error("need script name");
		return 1;
	}

	char varcd[FILE_NPATH];
	if (!cap_envget(varcd, sizeof varcd, "CAP_VARCD")) {
		cap_log("error", "need environment variable of cd");
		return 1;
	}

	char spath[FILE_NPATH]; // Script path
	cap_fsolvefmt(spath, sizeof spath, "%s/%s", varcd, argv[1]);
	if (isoutofhome(spath)) {
		freeargv(argc, argv);
		cap_die("invalid script '%s'", spath);
	}

	char exesname[NSCRIPTNAME]; // Execute script name in file
	readscriptline(exesname, sizeof exesname, spath);
	// cap_log("debug", "exesname[%s]\n", exesname);

	struct cap_string *cmdline = cap_strnew();
	cap_strapp(cmdline, exesname);
	cap_strapp(cmdline, " ");
	cap_strapp(cmdline, spath);
	cap_strapp(cmdline, " ");
	for (int i = 2; i < argc; ++i) {
		cap_strapp(cmdline, argv[i]);
		cap_strapp(cmdline, " ");
	}
	// cap_log("debug", "exesname[%s] spath[%s] cmdline[%s]\n", exesname, spath, cap_strgetc(cmdline));

	// Start process communication
	safesystem(cap_strgetc(cmdline));
	
	// Done
	cap_strdel(cmdline);
	return 0;
}
