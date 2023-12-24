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

#if !defined(_XFA_PSTACK_H)
#define _XFA_PSTACK_H

typedef struct s_xfa_stack_cls {
	struct ll_list_head link;
	int size;
	int count;
	void const *ptrs[1];
} xfa_stack_cls_t;

typedef struct s_xfa_ptrstack {
	xfa_system_t *sys;
	struct ll_list_head clist;
} xfa_ptrstack_t;


int xfa_ptrstack_create(xfa_system_t *sys, xfa_ptrstack_t *sp);
void xfa_ptrstack_free(xfa_ptrstack_t *sp);
int xfa_ptrstack_push(xfa_ptrstack_t *sp, void const *ptr);
void const *xfa_ptrstack_pop(xfa_ptrstack_t *sp);

#endif
