#include <stdio.h>
#define RFC3339_IMPL
#include "../rfc3339.h"

int main() {
	const char* str = "2024-05-01T10:47:40.123456+01:00";
	rfc3339time t = {0};
	rfc3339time_parse(str, &t);
	printf("Unix timestamp: %ld\n", rfc3339time_as_secs(&t));
	printf("Unix timestamp (microseconds): %ld\n", rfc3339time_as_us(&t));
	return 0;
}
