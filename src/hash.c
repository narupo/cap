#include "hash.h"

static pthread_mutex_t hash_crypt_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Create integer hash value from string
 *
 * @param[in] src source string
 *
 * @return number of hash value
 */
int
hash_int(const char* src) {
	int n = 0;

	for (const char* p = src; *p; ++p) {
		n += *p;
	}

	return n % HASH_NHASH;
}

/**
 * Create long hash value from string
 *
 * @param[in] src source string
 *
 * @return number of hash value
 */
long
hash_long(const char* src) {
	long n = 0;

	for (const char* p = src; *p; ++p) {
		n += *p;
	}

	return n % HASH_NHASH;
}

int
hash_int_from_path(const char* path) {
	int n = 0;

	for (const char* p = path; *p; ++p) {
		if (strchr("/\\:", *p)) {
			continue; // ignore this character
		}
		n += *p;
	}

	return n % HASH_NHASH;
}

char*
hash_sha2(char* dst, size_t dstsz, const char* src) {
	if (!dst || dstsz == 0 || !src) {
		perror("Invalid arguments");
		return NULL;
	}

	if (pthread_mutex_lock(&hash_crypt_mutex) != 0) {
		perror("Failed to lock");
		return NULL;
	}

	char seed[20];
	snprintf(seed, sizeof seed, "$5$%u", (unsigned) time(NULL));
	const char* p = crypt(src, seed);

	for (int found = 0; *p; ++p) {
		if (*p == '$') {
			++found;
		}
		if (found >= 3) {
			++p;
			break;
		}
	}

	if (!*p) {
		return NULL;
	}

	memset(dst, 0, dstsz);
	snprintf(dst, dstsz, "%s", p);

	if (pthread_mutex_unlock(&hash_crypt_mutex) != 0) {
		perror("Failed to unlock");
		return NULL;
	}

	return dst;
}