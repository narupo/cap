/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "env.h"

/********
* mutex *
********/

static pthread_mutex_t
envmtx = PTHREAD_MUTEX_INITIALIZER;

static bool
dounlock(void) {
	if (pthread_mutex_unlock(&envmtx) != 0) {
		return false;
	}
	return true;
}

static bool
dolock(void) {
	if (pthread_mutex_lock(&envmtx) != 0) {
		return false;
	}
	return true;
}

/************
* functions *
************/

char *
cap_envget(char *dst, size_t dstsz, const char *name) {
	if (!dolock()) {
		return NULL;
	}

	const char *val = getenv(name);
	if (!val) {
		return NULL;
	}

	snprintf(dst, dstsz, "%s", val);

	dounlock();
	return dst;
}

int
cap_envset(const char *name, const char *value, int overwrite) {
	if (!dolock()) {
		return -1;
	}

	int ret = setenv(name, value, overwrite);

	dounlock();
	return ret;
}

int
cap_envsetf(const char *name, const char *value) {
	// 'f' is head characater of 'force' 
	return cap_envset(name, value, 1);
}
