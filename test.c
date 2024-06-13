#include <assert.h>
#include <string.h>
#define RFC3339_IMPL
#include "rfc3339.h"

void test_from_str(const char* str, const char* expected, time_t secs_expected, suseconds_t us_expected) {
	rfc3339time t = {0};
	char outstr[50] = {0};
	assert(rfc3339time_parse(str, &t));

	time_t secs = rfc3339time_as_secs(&t);
	suseconds_t us = rfc3339time_as_us(&t);
	rfc3339time_fmt(outstr, sizeof(outstr), &t);

	printf("%s --> %s (%lds, %ldus)\n", str, outstr, secs, us);
	assert(strcmp(outstr, expected) == 0);
	assert(secs == secs_expected);
	assert(us == us_expected);
}

void test_invalid(const char* str) {
	rfc3339time t = {0};
	assert(!rfc3339time_parse(str, &t));
}

void test_from_secs(time_t secs, const char* expected, time_t secs_expected, suseconds_t us_expected) {
	rfc3339time t = {0};
	char outstr[50] = {0};
	assert(rfc3339time_from_secs(secs, &t));

	time_t secs_out = rfc3339time_as_secs(&t);
	suseconds_t us = rfc3339time_as_us(&t);
	rfc3339time_fmt(outstr, sizeof(outstr), &t);

	printf("%lds --> %s (%lds, %ldus)\n", secs, outstr, secs_out, us);
	assert(strcmp(outstr, expected) == 0);
	assert(secs_out == secs_expected);
	assert(us == us_expected);
}

void test_from_us(suseconds_t us, const char* expected, time_t secs_expected, suseconds_t us_expected) {
	rfc3339time t = {0};
	char outstr[50] = {0};
	assert(rfc3339time_from_us(us, &t));

	time_t secs = rfc3339time_as_secs(&t);
	suseconds_t us_out = rfc3339time_as_us(&t);
	rfc3339time_fmt(outstr, sizeof(outstr), &t);

	printf("%ldus --> %s (%lds, %ldus)\n", us, outstr, secs, us_out);
	assert(strcmp(outstr, expected) == 0);
	assert(secs == secs_expected);
	assert(us_out == us_expected);
}

int main(void) {
	test_from_str("2014-11-12T19:12:14.125-06:30", "2014-11-12T19:12:14.125-06:30", 1415796134, 1415796134125000L);
	test_from_str("2014-11-12T19:12:14.125z", "2014-11-12T19:12:14.125Z", 1415819534, 1415819534125000L);
	test_from_str("1980-01-01t01:01:60+01:00", "1980-01-01T01:01:60+01:00", 315540120, 315540120000000L);
	test_from_secs(1714560460, "2024-05-01T10:47:40Z", 1714560460, 1714560460L*1000000);
	test_from_secs(0, "1970-01-01T00:00:00Z", 0, 0);
	test_from_us(0, "1970-01-01T00:00:00Z", 0, 0);
	test_from_us(1714560460L * 1000000L, "2024-05-01T10:47:40Z", 1714560460, 1714560460L * 1000000L);
	test_from_us(1714560460608856L, "2024-05-01T10:47:40.608856Z", 1714560460, 1714560460608856L);
	test_from_us(1714560460608000L, "2024-05-01T10:47:40.608Z", 1714560460, 1714560460608000L);
	test_invalid("not a date");
	test_invalid("2014-11-12T19:12:14.125");
	test_invalid("2014-11-12 19:12:14.125Z");
	test_invalid("2014-11-12T19:12");

	time_t now = time(0);
	rfc3339time t = {0};
	char outstr[50] = {0};
	assert(rfc3339time_from_secs(now, &t));
	assert(rfc3339time_fmt(outstr, sizeof(outstr), &t));
	printf("now: %s\n", outstr);
	assert(rfc3339time_as_secs(&t) == now);

	return 0;
}
