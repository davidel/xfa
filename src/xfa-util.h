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

#if !defined(_XFA_UTIL_H)
#define _XFA_UTIL_H


int xfa_is_prime(int n);
void *xfa_memset(void *d, int c, int n);
void *xfa_memcpy(void *d, const void *s, int n);
int xfa_mapcmp(unsigned long const *m1, unsigned long const *m2, int n);
void xfa_mapcpy(unsigned long *d, unsigned long const *s, int n);
void xfa_or_map(unsigned long *d, unsigned long const *s, int n);
void xfa_and_map(unsigned long *d, unsigned long const *s, int n);
void xfa_not_map(unsigned long *d, int n);
int xfa_map_zero(unsigned long const *s, int n);
void xfa_map_init(unsigned long *d, int n);
void xfa_map_set_range(unsigned long *d, int n, int from, int to);
void xfa_gen_freelist(xfa_system_t * sys, struct ll_list_head *head,
		      void (*fproc) (xfa_system_t *, void *), int foff);
long xfa_atol(char const *str);


#endif
