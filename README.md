# RFC 3339 time parser and formatter

A single C header utility to parse and format [RFC 3339 timestamps](https://www.rfc-editor.org/rfc/rfc3339.html).

This small library uses `struct tm` and some function in `time.h` to store and manipuate the timestamp, so it should be easy to extend and integrate it.

## Dependencies
- libc

## Examples

Print the current timestamp in rfc3339 format:
```c
#include <stdio.h>
#include "rfc3339.h"

int main() {
	rfc3339time t = {0};
	char now_str[50] = {0};
	rfc3339time_from_secs(time(0), &t);
	rfc3339time_fmt(now_str, sizeof(now_str), &t);
	printf("Current time: %s\n", now_str);
	return 0;
}
```

Parse a rfc3339 formatted timestamp:
```c
#include <stdio.h>
#include "rfc3339.h"

int main() {
	const char* str = "2024-05-01T10:47:40Z";
	rfc3339time t = {0};
	rfc3339time_parse(str, &t);
	printf("Parsed date: %d/%d/%d\n", t.datetime.tm_mday, t.datetime.tm_mon + 1, t.datetime.tm_year + 1900);
	return 0;
}
```

Parse and convert the timestamp in unix timestamp (with fracsec and timezone support):
```c
#include <stdio.h>
#include "rfc3339.h"

int main() {
	const char* str = "2024-05-01T10:47:40.123456+01:00";
	rfc3339time t = {0};
	rfc3339time_parse(str, &t);
	printf("Unix timestamp: %ld\n", rfc3339time_as_secs(&t));
	printf("Unix timestamp (microseconds): %ld\n", rfc3339time_as_us(&t));
	return 0;
}
```
