// Copyright (c) 2024-2025 Matteo Benzi <matteo.benzi97@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
// Changelog:
//
// - 2.0.0    Add support for nanoseconds
//            New functions rfc3339time_as/from_ns
//            New functions rfc3339time_as/from_ms
//            Substitute suseconds_t with uint64_t
// - 1.0.1    Fix seconds not being written as double digits when less than 10 (by Ugo Botto)
// - 1.0.0    First implementation

#ifndef RFC3339_H
#define RFC3339_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

typedef struct {
	struct tm datetime;
	int secfrac_ns; // fraction of seconds in nanoseconds, from 0 to 999999999
} rfc3339time;


int rfc3339time_parse(const char* str, rfc3339time* time);
int rfc3339time_fmt(char* str, size_t str_len, const rfc3339time* time);
time_t rfc3339time_as_secs(const rfc3339time* time);
int rfc3339time_from_secs(time_t secs, rfc3339time* time);
uint64_t rfc3339time_as_ms(const rfc3339time* time);
int rfc3339time_from_ms(uint64_t ms, rfc3339time* time);
uint64_t rfc3339time_as_us(const rfc3339time* time);
int rfc3339time_from_us(uint64_t us, rfc3339time* time);
uint64_t rfc3339time_as_ns(const rfc3339time* time);
int rfc3339time_from_ns(uint64_t ns, rfc3339time* time);

#endif // RFC3339_H

#ifdef RFC3339_IMPL

int _ipow(int base, int exp)
{
	int result = 1;
	for (;;) {
		if (exp & 1) result *= base;
		exp >>= 1;
		if (!exp) break;
		base *= base;
	}
	return result;
}

void _ns_to_secfrac(int ns, char* outstr)
{
	int place = 10, prev_digit = 0, zeros = 1;
	for(int i = 9; i > 0; i--) {
		int digit = ns % place;
		if(digit > 9) {
			digit = (digit - prev_digit) / (place / 10);
		}
		place *= 10;
		zeros = zeros && digit == 0;

		if(zeros) {
			outstr[i - 1] = '\0';
		} else {
			outstr[i - 1] = digit + '0';
		}
	}
}

int rfc3339time_parse(const char* str, rfc3339time* time)
{
	int y = 0, M = 0, d = 0, h = 0, m = 0, s = 0, sf = 0;
	char frac_and_tz[10 + 7] = {0}; // up to ".999999999+00:00"
	int h_offset = 0, m_offset = 0;
	char t = 0;

	if(sscanf(str, "%d-%d-%d%c%d:%d:%d%s", &y, &M, &d, &t, &h, &m, &s, frac_and_tz) != 8 || (t != 'T' && t != 't')) {
		return 0; // invalid format
	}

	if(frac_and_tz[0] == '.') { // fraction of seconds present
		// count how many digits the fraction have
		int d = 0;
		while(frac_and_tz[1 + d] >= '0' && frac_and_tz[1 + d] <= '9') {
			d++;
		}
		if(d == 0 || d > 9) return 0; // invalid format

		if(sscanf(frac_and_tz, ".%d%s", &sf, frac_and_tz) != 2) {
			return 0; // invalid format
		}

		sf = sf * _ipow(10, 9 - d);
	}

	if(frac_and_tz[0] != 'Z' && frac_and_tz[0] != 'z') {
		if(sscanf(frac_and_tz, "%d:%d", &h_offset, &m_offset) != 2) {
			return 0; // invalid format
		}
		if(h_offset < 0) m_offset = -m_offset;
	}

	time->datetime.tm_year = y - 1900; // Year since 1900
	time->datetime.tm_mon = M - 1;     // 0-11
	time->datetime.tm_mday = d;        // 1-31
	time->datetime.tm_hour = h;        // 0-23
	time->datetime.tm_min = m;         // 0-59
	time->datetime.tm_sec = s;         // 0-61 (0-60 in C++11)
	time->secfrac_ns = sf;             // 0-999999999
	time->datetime.tm_gmtoff = m_offset * 60 + h_offset * 60 * 60;

	return 1;
}

int rfc3339time_fmt(char* str, size_t str_len, const rfc3339time* time)
{
	size_t fmt_len, total_len = 0;

	fmt_len = strftime(str, str_len, "%Y-%m-%dT%H:%M:%S", &time->datetime);
	if(fmt_len == 0) return 0;
	total_len += fmt_len;
	if(total_len >= str_len) return 0;

	if(time->secfrac_ns > 0) {
		char secfrac[10] = "000000000";
		_ns_to_secfrac(time->secfrac_ns, secfrac);

		fmt_len = snprintf(&str[total_len], str_len - total_len, ".%s", secfrac);
		if(fmt_len == 0) return 0;
		total_len += fmt_len;
		if(total_len >= str_len) return 0;
	}

	str[total_len] = 'Z';
	total_len += 1;

	if(time->datetime.tm_gmtoff != 0) {
		int h = (float)time->datetime.tm_gmtoff / (60 * 60);
		int m = ((float)time->datetime.tm_gmtoff / 60) - h * 60;
		if(m < 0) m = -m;
		// total_len-1 to overwrite the 'Z'
		if(snprintf(&str[total_len - 1], str_len - total_len, "%+.2d:%.2d", h, m) == 0)
			return 0;
	} else {
		str[total_len] = '\0';
	}
	return 1;
}

time_t rfc3339time_as_secs(const rfc3339time* time)
{
	// timegm() may modify time->datetime
	struct tm t = time->datetime;
	return timegm(&t) + time->datetime.tm_gmtoff;
}

int rfc3339time_from_secs(time_t secs, rfc3339time* time)
{
	return gmtime_r(&secs, &time->datetime) != NULL;
}

uint64_t rfc3339time_as_ms(const rfc3339time* time)
{
	return rfc3339time_as_ns(time) / 1000000;
}

int rfc3339time_from_ms(uint64_t ms, rfc3339time* time)
{
	return rfc3339time_from_ns(ms * 1000000, time);
}

uint64_t rfc3339time_as_us(const rfc3339time* time)
{
	return rfc3339time_as_ns(time) / 1000;
}

int rfc3339time_from_us(uint64_t us, rfc3339time* time)
{
	return rfc3339time_from_ns(us * 1000, time);
}

uint64_t rfc3339time_as_ns(const rfc3339time* time)
{
	uint64_t secs = rfc3339time_as_secs(time);
	return secs * 1000000000 + time->secfrac_ns;
}

int rfc3339time_from_ns(uint64_t ns, rfc3339time* time)
{
	time_t secs = ns / 1000000000;
	time->secfrac_ns = ns - secs * 1000000000;
	return rfc3339time_from_secs(secs, time);
}

#endif // RFC3339_IMPL
