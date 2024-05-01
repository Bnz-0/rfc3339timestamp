#include <stdio.h>
#include "../rfc3339.h"

int main() {
	rfc3339time t = {0};
	char now_str[50] = {0};
	rfc3339time_from_secs(time(0), &t);
	rfc3339time_fmt(now_str, sizeof(now_str), &t);
	printf("Current time: %s\n", now_str);
	return 0;
}
