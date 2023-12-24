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

#define PTRH_NEEDS_GROW(p) ((p)->stored >= 2 * (p)->size)


int xfa_ptrhash_create(xfa_system_t *sys, xfa_ptrhash_t *hp, int size) {
	int i;

	for (size++; !xfa_is_prime(size); size++);
	if ((hp->bucks = (xfa_ptrhash_node_t **)
	     XFASYS_ALLOC(sys, size * sizeof(xfa_ptrhash_node_t *))) == XFA_NULL) {
		XFASYS_ERROR(sys, XFAE_ALLOC);
		return -1;
	}
	hp->sys = sys;
	for (i = 0; i < size; i++)
		hp->bucks[i] = XFA_NULL;
	hp->size = size;
	hp->stored = 0;

	return 0;
}

void xfa_ptrhash_free(xfa_ptrhash_t *hp) {
	int i;
	xfa_ptrhash_node_t *hn, *tmp;
	xfa_system_t *sys = hp->sys;

	for (i = 0; i < hp->size; i++)
		for (hn = hp->bucks[i]; (tmp = hn) != XFA_NULL;) {
			hn = hn->next;
			XFASYS_FREE(sys, tmp);
		}
	XFASYS_FREE(sys, hp->bucks);
}

void xfa_ptrhash_clean(xfa_ptrhash_t *hp) {
	int i;
	xfa_ptrhash_node_t *hn, *tmp;
	xfa_system_t *sys = hp->sys;

	for (i = 0; i < hp->size; i++) {
		for (hn = hp->bucks[i]; (tmp = hn) != XFA_NULL;) {
			hn = hn->next;
			XFASYS_FREE(sys, tmp);
		}
		hp->bucks[i] = XFA_NULL;
	}
	hp->stored = 0;
}

int xfa_ptrhash_stored(xfa_ptrhash_t const *hp) {
	return hp->stored;
}

xfa_ptrhash_node_t *xfa_ptrhash_get(xfa_ptrhash_t *hp, void *ptr, xfa_ptrhash_node_t *** prev) {
	unsigned long idx;
	xfa_ptrhash_node_t *hn, **tmp;

	idx = (unsigned long) ptr % hp->size;
	for (hn = hp->bucks[idx], tmp = &hp->bucks[idx]; hn != XFA_NULL;
	     tmp = &hn->next, hn = hn->next)
		if (hn->ptr == ptr) {
			if (prev != XFA_NULL)
				*prev = tmp;
			return hn;
		}

	return XFA_NULL;
}

static int xfa_ptrhash_grow(xfa_ptrhash_t *hp) {
	int i, size;
	unsigned long idx;
	xfa_ptrhash_node_t **bucks, *hn, *tmp;
	xfa_system_t *sys = hp->sys;

	size = 2 * hp->size;
	for (size++; !xfa_is_prime(size); size++);
	if ((bucks = (xfa_ptrhash_node_t **)
	     XFASYS_ALLOC(sys, size * sizeof(xfa_ptrhash_node_t *))) == XFA_NULL) {
		XFASYS_ERROR(sys, XFAE_ALLOC);
		return -1;
	}
	for (i = 0; i < size; i++)
		bucks[i] = XFA_NULL;
	for (i = 0; i < hp->size; i++)
		for (hn = hp->bucks[i]; (tmp = hn) != XFA_NULL;) {
			hn = hn->next;
			idx = (unsigned long) tmp->ptr % size;
			tmp->next = bucks[idx];
			bucks[idx] = tmp;
		}
	XFASYS_FREE(sys, hp->bucks);
	hp->bucks = bucks;
	hp->size = size;

	return 0;
}

xfa_ptrhash_node_t *xfa_ptrhash_add(xfa_ptrhash_t *hp, void *ptr, void *val, int check) {
	unsigned long idx;
	xfa_ptrhash_node_t *hn;
	xfa_system_t *sys = hp->sys;

	if (check && (hn = xfa_ptrhash_get(hp, ptr, XFA_NULL)) != XFA_NULL)
		return hn;
	if (PTRH_NEEDS_GROW(hp) && xfa_ptrhash_grow(hp) < 0)
		return XFA_NULL;
	if ((hn = (xfa_ptrhash_node_t *)
	     XFASYS_ALLOC(sys, sizeof(xfa_ptrhash_node_t))) == XFA_NULL) {
		XFASYS_ERROR(sys, XFAE_ALLOC);
		return XFA_NULL;
	}
	hn->ptr = ptr;
	hn->val = val;
	idx = (unsigned long) ptr % hp->size;
	hn->next = hp->bucks[idx];
	hp->bucks[idx] = hn;
	hp->stored++;

	return hn;
}

void xfa_ptrhash_freenode(xfa_system_t *sys, xfa_ptrhash_node_t *hn) {
	XFASYS_FREE(sys, hn);
}

void xfa_ptrhash_remove(xfa_ptrhash_t *hp, xfa_ptrhash_node_t *hn, xfa_ptrhash_node_t ** prev) {
	*prev = hn->next;
	hp->stored--;
}

xfa_ptrhash_node_t *xfa_ptrhash_first(xfa_ptrhash_t *hp, xfa_ptrhash_enum_t *enh) {
	int i;
	xfa_ptrhash_node_t *hn;

	enh->prev = XFA_NULL;
	for (i = 0; i < hp->size; i++)
		if (hp->bucks[i] != XFA_NULL)
			break;
	if (i == hp->size)
		return XFA_NULL;
	hn = hp->bucks[i];
	enh->hp = hp;
	enh->hn = hn->next;
	enh->prev = &hp->bucks[i];
	enh->cidx = i;

	return hn;
}

xfa_ptrhash_node_t *xfa_ptrhash_next(xfa_ptrhash_enum_t *enh) {
	int i;
	xfa_ptrhash_t *hp;
	xfa_ptrhash_node_t *hn;

	if (enh->prev == XFA_NULL)
		return XFA_NULL;
	if ((hn = enh->hn) != XFA_NULL) {
		if (*enh->prev != hn)
			enh->prev = &(*enh->prev)->next;
		enh->hn = hn->next;
		return hn;
	}
	enh->prev = XFA_NULL;
	for (i = enh->cidx + 1, hp = enh->hp; i < hp->size; i++)
		if (hp->bucks[i] != XFA_NULL)
			break;
	if (i >= hp->size)
		return XFA_NULL;
	hn = hp->bucks[i];
	enh->hn = hn->next;
	enh->prev = &hp->bucks[i];
	enh->cidx = i;

	return hn;
}

int xfa_ptrhash_remove_curr(xfa_ptrhash_enum_t *enh) {
	if (enh->prev == XFA_NULL || *enh->prev == XFA_NULL || *enh->prev == enh->hn) {
		XFASYS_ERROR(enh->hp->sys, XFAE_PTRH_NOTFOUND);
		return -1;
	}
	xfa_ptrhash_remove(enh->hp, *enh->prev, enh->prev);

	return 0;
}

int xfa_ptrhash_cmp(xfa_ptrhash_t *hp1, xfa_ptrhash_t *hp2) {
	int i;
	xfa_ptrhash_node_t *hn;

	if (hp1->stored > hp2->stored)
		return +2;
	else if (hp1->stored < hp2->stored)
		return -2;
	for (i = 0; i < hp1->size; i++)
		for (hn = hp1->bucks[i]; hn != XFA_NULL; hn = hn->next)
			if (xfa_ptrhash_get(hp2, hn->ptr, XFA_NULL) == XFA_NULL)
				return +1;

	return 0;
}

int xfa_ptrhash_contain(xfa_ptrhash_t *hp1, xfa_ptrhash_t *hp2) {
	int i;
	xfa_ptrhash_node_t *hn;

	if (hp1->stored < hp2->stored)
		return 0;
	for (i = 0; i < hp2->size; i++)
		for (hn = hp2->bucks[i]; hn != XFA_NULL; hn = hn->next)
			if (xfa_ptrhash_get(hp1, hn->ptr, XFA_NULL) == XFA_NULL)
				return 0;

	return 1;
}

