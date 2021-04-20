#include "vutils.h"

#include <stdio.h>

char* vitoa(int i, char* s, size_t len) {
	snprintf(s, len, "%d", i);
	return s;
}