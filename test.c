#include <assert.h>
#include <string.h>
#define RFC3339_IMPL
#include "rfc3339.h"

void test_from_str(const char* str, const char* expected, time_t secs_expected, uint64_t ns_expected)
{
	rfc3339time t = {0};
	char outstr[50] = {0};
	assert(rfc3339time_parse(str, &t));

	time_t secs = rfc3339time_as_secs(&t);
	uint64_t ns = rfc3339time_as_ns(&t);
	rfc3339time_fmt(outstr, sizeof(outstr), &t);

	printf("%s --> %s (%lds, %ldns)\n", str, outstr, secs, ns);
	assert(strcmp(outstr, expected) == 0);
	assert(secs == secs_expected);
	assert(ns == ns_expected);
	assert(ns / 1000 == rfc3339time_as_us(&t));
	assert(ns / 1000000 == rfc3339time_as_ms(&t));
}

void test_invalid(const char* str)
{
	rfc3339time t = {0};
	assert(!rfc3339time_parse(str, &t));
}

void test_from_secs(time_t secs, const char* expected, time_t secs_expected, uint64_t ns_expected)
{
	rfc3339time t = {0};
	char outstr[50] = {0};
	assert(rfc3339time_from_secs(secs, &t));

	time_t secs_out = rfc3339time_as_secs(&t);
	uint64_t ns = rfc3339time_as_ns(&t);
	rfc3339time_fmt(outstr, sizeof(outstr), &t);

	printf("%lds --> %s (%lds, %ldns)\n", secs, outstr, secs_out, ns);
	assert(strcmp(outstr, expected) == 0);
	assert(secs_out == secs_expected);
	assert(ns == ns_expected);
	assert(ns / 1000 == rfc3339time_as_us(&t));
	assert(ns / 1000000 == rfc3339time_as_ms(&t));
}

void test_from_ns(uint64_t ns, const char* expected, time_t secs_expected, uint64_t ns_expected)
{
	rfc3339time t = {0};
	char outstr[50] = {0};
	assert(rfc3339time_from_ns(ns, &t));

	time_t secs = rfc3339time_as_secs(&t);
	uint64_t ns_out = rfc3339time_as_ns(&t);
	rfc3339time_fmt(outstr, sizeof(outstr), &t);

	printf("%ldns --> %s (%lds, %ldns)\n", ns, outstr, secs, ns_out);
	assert(strcmp(outstr, expected) == 0);
	assert(secs == secs_expected);
	assert(ns_out == ns_expected);
	assert(ns_out / 1000 == rfc3339time_as_us(&t));
	assert(ns_out / 1000000 == rfc3339time_as_ms(&t));
}

int main(void)
{
	test_from_str("2014-11-12T19:12:14.125-06:30", "2014-11-12T19:12:14.125-06:30", 1415796134, 1415796134125000000L);
	test_from_str("2024-12-13T16:27:07.60133Z", "2024-12-13T16:27:07.60133Z", 1734107227, 1734107227601330000L);
	test_from_str("2014-11-12T19:12:14.125z", "2014-11-12T19:12:14.125Z", 1415819534, 1415819534125000000L);
	test_from_str("2014-11-12T19:12:14.0500Z", "2014-11-12T19:12:14.05Z", 1415819534, 1415819534050000000L);
	test_from_str("2014-11-12T19:12:14.050010Z", "2014-11-12T19:12:14.05001Z", 1415819534, 1415819534050010000L);
	test_from_str("1980-01-01t01:01:60+01:00", "1980-01-01T01:01:60+01:00", 315540120, 315540120000000000L);
	test_from_str("2025-07-18T20:44:07Z", "2025-07-18T20:44:07Z", 1752871447, 1752871447000000000L);
	test_from_str("2025-07-18T20:44:07.999999999Z", "2025-07-18T20:44:07.999999999Z", 1752871447, 1752871447999999999L);
	test_from_str("2025-07-18T20:44:07.999999999+10:00", "2025-07-18T20:44:07.999999999+10:00", 1752871447 + 10 * 60 * 60, 1752871447999999999L + 10L * 60L * 60L * 1000000000L);
	test_from_secs(1714560460, "2024-05-01T10:47:40Z", 1714560460, 1714560460L * 1000000000L);
	test_from_secs(0, "1970-01-01T00:00:00Z", 0, 0);
	test_from_ns(0, "1970-01-01T00:00:00Z", 0, 0);
	test_from_ns(1714560460L * 1000000000L, "2024-05-01T10:47:40Z", 1714560460, 1714560460L * 1000000000L);
	test_from_ns(1714560460608856000L, "2024-05-01T10:47:40.608856Z", 1714560460, 1714560460608856000L);
	test_from_ns(1714560460608000000L, "2024-05-01T10:47:40.608Z", 1714560460, 1714560460608000000L);
	test_from_ns(1714560460608800429L, "2024-05-01T10:47:40.608800429Z", 1714560460, 1714560460608800429L);
	test_invalid("not a date");
	test_invalid("2014-11-12T19:12:14.125");
	test_invalid("2014-11-12 19:12:14.125Z");
	test_invalid("2014-11-12T19:12");
	test_invalid("2014-11-12T19:12:14.Z");
	test_invalid("2014-11-12T19:12:14.0123456789Z");

	time_t now = time(0);
	rfc3339time t = {0};
	char outstr[50] = {0};
	assert(rfc3339time_from_secs(now, &t));
	assert(rfc3339time_fmt(outstr, sizeof(outstr), &t));
	printf("now: %s\n", outstr);
	assert(rfc3339time_as_secs(&t) == now);

	return 0;
}
