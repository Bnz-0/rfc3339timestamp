// Copyright (c) 2024 Matteo Benzi <matteo.benzi97@gmail.com>
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

#ifndef RFC3339_H
#define RFC3339_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>

typedef struct {
	struct tm datetime;
	suseconds_t secfrac_us;
} rfc3339time;


int rfc3339time_parse(const char* str, rfc3339time* time);
int rfc3339time_fmt(char* str, size_t str_len, const rfc3339time* time);
time_t rfc3339time_as_secs(const rfc3339time* time);
int rfc3339time_from_secs(time_t secs, rfc3339time* time);
suseconds_t rfc3339time_as_us(const rfc3339time* time);
int rfc3339time_from_us(suseconds_t us, rfc3339time* time);

#endif // RFC3339_H

#ifdef RFC3339_IMPL

int rfc3339time_parse(const char* str, rfc3339time* time) {
	int y=0,M=0,d=0,h=0,m=0;
	double s = 0.0;
	char time_offset[8] = {0};
	int h_offset=0, m_offset=0;
	char t = 0;

	if(sscanf(str, "%d-%d-%d%c%d:%d:%lf%s", &y, &M, &d, &t, &h, &m, &s, time_offset) != 8 || (t != 'T' && t != 't')) {
		return 0; // invalid format
	}

	if(time_offset[0] != 'Z' && time_offset[0] != 'z') {
		if(sscanf(time_offset, "%d:%d", &h_offset, &m_offset) != 2) {
			return 0; // invalid format
		}
		if(h_offset < 0) m_offset = -m_offset;
	}

	time->datetime.tm_year = y - 1900; // Year since 1900
	time->datetime.tm_mon = M - 1;     // 0-11
	time->datetime.tm_mday = d;        // 1-31
	time->datetime.tm_hour = h;        // 0-23
	time->datetime.tm_min = m;         // 0-59
	time->datetime.tm_sec = (int)s;    // 0-61 (0-60 in C++11)
	time->datetime.tm_gmtoff = m_offset * 60 + h_offset * 60 * 60;
	time->secfrac_us = ((s - (int)s) * 1000000.0) + 0.5; // round

	return 1;
}

int rfc3339time_fmt(char* str, size_t str_len, const rfc3339time* time) {
	size_t fmt_len = strftime(str, str_len, "%Y-%m-%dT%H:%M:", &time->datetime);
	if(fmt_len == 0) return 0;

  // this intially always prints the seconds/fractional part as 9 characters
	size_t fracsec_len_total = snprintf(&str[fmt_len], str_len-fmt_len, "%09.6f", (double)time->datetime.tm_sec + time->secfrac_us / 1000000.0);
	if(fracsec_len_total == 0) return 0;

	size_t digits_to_remove = 0;

	// we start checking if there is a 0 from the end 
	// because it's gonna be much more common to have 
	// trailing digits than trailing zeros.
	// Potentially we can remove up to 7 character including the .

	for (size_t i = 0; i < 7; i++) {
    	char c = str[fmt_len + fracsec_len_total - 1 - i];

    	if (c == '0' || c == '.') {
      		digits_to_remove++;
    	} 
    	else {
      		break;
		}
	}

	fmt_len += fracsec_len_total - (digits_to_remove);
	str[fmt_len] = 'Z';
	fmt_len += 1;

	if(time->datetime.tm_gmtoff != 0) {
		int h = (float)time->datetime.tm_gmtoff / (60*60);
		int m = ((float)time->datetime.tm_gmtoff / 60) - h*60;
		if(m < 0) m = -m;
		// fmt_len-1 to overwrite the 'Z'
		if(snprintf(&str[fmt_len-1], str_len-fmt_len, "%+.2d:%.2d", h, m) == 0)
			return 0;
	} else {
		str[fmt_len] = '\0';
 	}
	return 1;
}

time_t rfc3339time_as_secs(const rfc3339time* time) {
	// timegm() may modify time->datetime
	struct tm t = time->datetime;
	return timegm(&t) + time->datetime.tm_gmtoff;
}

int rfc3339time_from_secs(time_t secs, rfc3339time* time) {
	return gmtime_r(&secs, &time->datetime) != NULL;
}

suseconds_t rfc3339time_as_us(const rfc3339time* time) {
	suseconds_t secs = rfc3339time_as_secs(time);
	return secs * 1000000 + time->secfrac_us;
}

int rfc3339time_from_us(suseconds_t us, rfc3339time* time) {
	time_t secs = us / 1000000;
	time->secfrac_us = us - secs * 1000000;
	return rfc3339time_from_secs(secs, time);
}

#endif // RFC3339_IMPL
