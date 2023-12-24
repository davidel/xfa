/*
 *  XFA by Davide Libenzi ( Finite Automata library )
 *  Copyright (C) 2000  Davide Libenzi
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */

#include "xfa-include.h"


int xfa_is_prime(int n) {
	int i, hn;

	if (n <= 3)
		return 1;
	if (!(n & 1))
		return 0;
	for (i = 3, hn = n / 2; i < hn; i += 2)
		if (!(n % i))
			return 0;

	return 1;
}

void *xfa_memset(void *d, int c, int n) {
	unsigned char *dp;

	for (dp = (unsigned char *) d; n > 0; dp++, n--)
		*dp = (unsigned char) c;

	return d;
}

void *xfa_memcpy(void *d, const void *s, int n) {
	unsigned char *dp;
	unsigned char const *sp;

	dp = (unsigned char *) d;
	sp = (unsigned char const *) s;
	if ((unsigned char const *) dp < sp || (unsigned char const *) dp > sp + n)
		for (; n > 0; dp++, sp++, n--)
			*dp = *sp;
	else {
		dp += n - 1;
		sp += n - 1;
		for (; n > 0; dp--, sp--, n--)
			*dp = *sp;
	}

	return d;
}

int xfa_mapcmp(unsigned long const *m1, unsigned long const *m2, int n) {
	for (; n > 0; n--, m1++, m2++)
		if (*m1 != *m2)
			return *m1 > *m2 ? +1 : -1;

	return 0;
}

void xfa_mapcpy(unsigned long *d, unsigned long const *s, int n) {
	for (; n > 0; n--, d++, s++)
		*d = *s;
}

void xfa_or_map(unsigned long *d, unsigned long const *s, int n) {
	for (; n > 0; n--, d++, s++)
		*d |= *s;
}

void xfa_and_map(unsigned long *d, unsigned long const *s, int n) {
	for (; n > 0; n--, d++, s++)
		*d &= *s;
}

void xfa_not_map(unsigned long *d, int n) {
	for (; n > 0; n--, d++)
		*d = ~*d;
}

int xfa_map_zero(unsigned long const *s, int n) {
	for (; n > 0; n--, s++)
		if (*s != 0)
			return 0;

	return 1;
}

void xfa_map_init(unsigned long *d, int n) {
	for (; n > 0; n--, d++)
		*d = 0;
}

void xfa_map_set_range(unsigned long *d, int n, int from, int to) {
	from = XFA_MIN(from, n * XFA_ULONG_BITS);
	to = XFA_MIN(to, n * XFA_ULONG_BITS);
	for (; from <= to; from++)
		d[from / XFA_ULONG_BITS] |= 1UL << (from % XFA_ULONG_BITS);
}

void xfa_gen_freelist(xfa_system_t *sys, struct ll_list_head *head,
		      void (*fproc) (xfa_system_t *, void *), int foff) {
	struct ll_list_head *pos;

	while ((pos = LL_LIST_FIRST(head)) != XFA_NULL) {
		LL_LIST_DEL(pos);
		(*fproc) (sys, (char *) pos - foff);
	}
}

long xfa_atol(char const *str) {
	long sign = +1, val = 0, mult = 1;
	char const *tmp = str;

	if (*tmp == '+')
		tmp++;
	else if (*tmp == '-')
		sign = -1, tmp++;
	for (; XFA_ISDIGIT(*tmp); tmp++);
	for (tmp--; tmp >= str; tmp--, mult *= 10)
		val += mult * (long) XFA_DVALUE(*tmp);

	return sign > 0 ? val : -val;
}
