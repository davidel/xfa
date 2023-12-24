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

#if !defined(_XFA_PHASH_H)
#define _XFA_PHASH_H

typedef struct s_xfa_ptrhash_node {
	struct s_xfa_ptrhash_node *next;
	void *ptr;
	void *val;
} xfa_ptrhash_node_t;

typedef struct s_xfa_ptrhash {
	xfa_system_t *sys;
	int size;
	int stored;
	xfa_ptrhash_node_t **bucks;
} xfa_ptrhash_t;

typedef struct s_xfa_ptrhash_enum {
	xfa_ptrhash_t *hp;
	int cidx;
	xfa_ptrhash_node_t *hn;
	xfa_ptrhash_node_t **prev;
} xfa_ptrhash_enum_t;


int xfa_ptrhash_create(xfa_system_t *sys, xfa_ptrhash_t *hp, int size);
void xfa_ptrhash_free(xfa_ptrhash_t *hp);
void xfa_ptrhash_clean(xfa_ptrhash_t *hp);
int xfa_ptrhash_stored(xfa_ptrhash_t const *hp);
xfa_ptrhash_node_t *xfa_ptrhash_get(xfa_ptrhash_t *hp, void *ptr, xfa_ptrhash_node_t *** prev);
xfa_ptrhash_node_t *xfa_ptrhash_add(xfa_ptrhash_t *hp, void *ptr, void *val, int check);
void xfa_ptrhash_freenode(xfa_system_t *sys, xfa_ptrhash_node_t *hn);
void xfa_ptrhash_remove(xfa_ptrhash_t *hp, xfa_ptrhash_node_t *hn, xfa_ptrhash_node_t ** prev);
xfa_ptrhash_node_t *xfa_ptrhash_first(xfa_ptrhash_t *hp, xfa_ptrhash_enum_t *enh);
xfa_ptrhash_node_t *xfa_ptrhash_next(xfa_ptrhash_enum_t *enh);
int xfa_ptrhash_remove_curr(xfa_ptrhash_enum_t *enh);
int xfa_ptrhash_cmp(xfa_ptrhash_t *hp1, xfa_ptrhash_t *hp2);
int xfa_ptrhash_contain(xfa_ptrhash_t *hp1, xfa_ptrhash_t *hp2);

#endif
