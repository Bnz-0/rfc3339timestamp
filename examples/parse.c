#include <stdio.h>
#include "../rfc3339.h"

int main() {
	const char* str = "2024-05-01T10:47:40Z";
	rfc3339time t = {0};
	rfc3339time_parse(str, &t);
	printf("Parsed date: %d/%d/%d\n", t.datetime.tm_mday, t.datetime.tm_mon + 1, t.datetime.tm_year + 1900);
	return 0;
}
