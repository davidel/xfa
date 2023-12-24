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

#if !defined(_XFA_H)
#define _XFA_H

#define XFATF_EPSTRANS (1 << 0)

#define XFASF_START (1 << 0)
#define XFASF_TARGET (1 << 1)

#define XFASF_TRANSFERABLE (XFASF_START | XFASF_TARGET)

#define XFA_MAPSIZE (256 / XFA_CHARBIT)
#define XFA_ULONG_BITS (sizeof(unsigned long) * XFA_CHARBIT)
#define XFA_TRANSMAP_ULONGS (256 / XFA_ULONG_BITS)
#define XFA_DEF_PHASH_SIZE 4
#define XFA_NFASREP_PHASH_SIZE 128


struct s_xfa_state;

typedef struct s_xfa_attr {
	struct ll_list_head link;
	int type;
	void (*free) (xfa_system_t *, void *);
	void *adata;
} xfa_attr_t;

typedef struct s_xfa_trans {
	struct ll_list_head tlink;
	struct ll_list_head flink;
	unsigned long *map;
	unsigned long flags;
	struct s_xfa_state *to;
	struct s_xfa_state *from;
} xfa_trans_t;

typedef struct s_xfa_state {
	struct ll_list_head tlist;
	struct ll_list_head flist;
	xfa_ptrhash_t sh;
	unsigned long flags;
	unsigned long label;
	struct ll_list_head alist;
} xfa_state_t;


xfa_trans_t *xfa_alloc_trans(xfa_system_t *sys, unsigned long const *map,
			     unsigned long flags, xfa_state_t *from, xfa_state_t *to);
void xfa_free_trans(xfa_system_t *sys, xfa_trans_t *trn);
xfa_state_t *xfa_alloc_state(xfa_system_t *sys, int phsize);
void xfa_free_state(xfa_system_t *sys, xfa_state_t *stt, int recurse);
void xfa_free_attr(xfa_system_t *sys, xfa_attr_t *attr);
int xfa_state_add(xfa_state_t *std, xfa_state_t *stt, void *val, int check);
int xfa_do_once(xfa_system_t *sys, xfa_state_t *stt,
		int (*dproc) (void *, xfa_system_t *, xfa_state_t *), void *priv);
xfa_state_t *xfa_nfa2dfa(xfa_system_t *sys, xfa_state_t *stt);
int xfa_label_states(xfa_system_t *sys, xfa_state_t *stt, unsigned long *label);
int xfa_dfa_minimize(xfa_system_t *sys, xfa_state_t *stt);


#endif
