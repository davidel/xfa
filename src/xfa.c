/*    Copyright 2023 Davide Libenzi
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 * 
 */


#include "xfa-include.h"


typedef struct s_xfa_move {
	struct ll_list_head link;
	unsigned long map[XFA_TRANSMAP_ULONGS];
	unsigned long flags;
	struct s_xfa_state *to;
} xfa_move_t;


xfa_trans_t *xfa_alloc_trans(xfa_system_t *sys, unsigned long const *map,
			     unsigned long flags, xfa_state_t *from, xfa_state_t *to) {
	int mapsize;
	xfa_trans_t *trn;

	mapsize = flags & XFATF_EPSTRANS ? 0 : XFA_MAPSIZE;
	if ((trn = (xfa_trans_t *) XFASYS_ALLOC(sys, sizeof(xfa_trans_t) + mapsize)) == XFA_NULL) {
		XFASYS_ERROR(sys, XFAE_ALLOC);
		return XFA_NULL;
	}
	trn->flags = flags;
	trn->from = from;
	trn->to = to;
	trn->map = mapsize > 0 ? (unsigned long *) &trn[1] : XFA_NULL;
	LL_LIST_ADDT(&trn->tlink, &from->tlist);
	LL_LIST_ADDT(&trn->flink, &to->flist);
	if (trn->map != XFA_NULL) {
		if (map != XFA_NULL)
			xfa_mapcpy(trn->map, map, XFA_TRANSMAP_ULONGS);
		else
			xfa_map_init(trn->map, XFA_TRANSMAP_ULONGS);
	}

	return trn;
}

static void xfa_relink_trans(xfa_trans_t *trn, xfa_state_t *from, xfa_state_t *to) {
	LL_LIST_DEL(&trn->tlink);
	LL_LIST_DEL(&trn->flink);
	trn->from = from;
	trn->to = to;
	LL_LIST_ADDT(&trn->tlink, &from->tlist);
	LL_LIST_ADDT(&trn->flink, &to->flist);
}

void xfa_free_trans(xfa_system_t *sys, xfa_trans_t *trn) {
	if (!LL_NODE_UNLINKED(&trn->tlink))
		LL_LIST_DEL(&trn->tlink);
	if (!LL_NODE_UNLINKED(&trn->flink))
		LL_LIST_DEL(&trn->flink);
	XFASYS_FREE(sys, trn);
}

xfa_state_t *xfa_alloc_state(xfa_system_t *sys, int phsize) {
	xfa_state_t *stt;

	if ((stt = (xfa_state_t *) XFASYS_ALLOC(sys, sizeof(xfa_state_t))) == XFA_NULL) {
		XFASYS_ERROR(sys, XFAE_ALLOC);
		return XFA_NULL;
	}
	LL_INIT_LIST_HEAD(&stt->tlist);
	LL_INIT_LIST_HEAD(&stt->flist);
	LL_INIT_LIST_HEAD(&stt->alist);
	stt->flags = 0;
	stt->label = 0;
	if (xfa_ptrhash_create(sys, &stt->sh, phsize) < 0) {
		XFASYS_FREE(sys, stt);
		return XFA_NULL;
	}

	return stt;
}

void xfa_free_state(xfa_system_t *sys, xfa_state_t *stt, int recurse) {
	struct ll_list_head *pos;
	xfa_trans_t *trn;

	while ((pos = LL_LIST_FIRST(&stt->flist)) != XFA_NULL) {
		trn = LL_LIST_ENTRY(pos, xfa_trans_t, flink);
		trn->to = XFA_NULL;
		LL_LIST_DEL(pos);
		if (LL_NODE_UNLINKED(&trn->tlink))
			xfa_free_trans(sys, trn);
	}
	while ((pos = LL_LIST_FIRST(&stt->tlist)) != XFA_NULL) {
		trn = LL_LIST_ENTRY(pos, xfa_trans_t, tlink);
		if (recurse && trn->to != XFA_NULL)
			xfa_free_state(sys, trn->to, recurse);
		trn->from = XFA_NULL;
		LL_LIST_DEL(pos);
		if (LL_NODE_UNLINKED(&trn->flink))
			xfa_free_trans(sys, trn);
	}
	xfa_ptrhash_free(&stt->sh);
	xfa_gen_freelist(sys, &stt->alist,
			 (void (*)(xfa_system_t *, void *)) xfa_free_attr,
			 XFA_OFFSETOF(xfa_attr_t, link));
	XFASYS_FREE(sys, stt);
}

void xfa_free_attr(xfa_system_t *sys, xfa_attr_t *attr) {
	if (attr->free != XFA_NULL)
		(*attr->free) (sys, attr->adata);
	XFASYS_FREE(sys, attr);
}

int xfa_state_add(xfa_state_t *std, xfa_state_t *stt, void *val, int check) {
	if (check && xfa_ptrhash_get(&std->sh, stt, XFA_NULL) != XFA_NULL)
		return 0;
	if (xfa_ptrhash_add(&std->sh, stt, val, 0) == XFA_NULL)
		return -1;
	if (stt->flags & XFASF_TRANSFERABLE)
		std->flags |= stt->flags & XFASF_TRANSFERABLE;

	return 0;
}

int xfa_do_once(xfa_system_t *sys, xfa_state_t *stt,
		int (*dproc) (void *, xfa_system_t *, xfa_state_t *), void *priv) {
	xfa_state_t *stc;
	struct ll_list_head *pos;
	xfa_trans_t *trn;
	xfa_ptrhash_t nsh;
	xfa_ptrstack_t nsp;

	if (xfa_ptrhash_create(sys, &nsh, XFA_NFASREP_PHASH_SIZE) < 0)
		return -1;
	if (xfa_ptrstack_create(sys, &nsp) < 0) {
		xfa_ptrhash_free(&nsh);
		return -1;
	}
	if (xfa_ptrstack_push(&nsp, stt) < 0) {
		xfa_ptrstack_free(&nsp);
		xfa_ptrhash_free(&nsh);
		return -1;
	}
	while ((stc = (xfa_state_t *) xfa_ptrstack_pop(&nsp)) != XFA_NULL) {
		if (xfa_ptrhash_get(&nsh, stc, XFA_NULL) == XFA_NULL) {
			if (xfa_ptrhash_add(&nsh, stc, XFA_NULL, 0) == XFA_NULL ||
			    (*dproc) (priv, sys, stc) < 0) {
				xfa_ptrstack_free(&nsp);
				xfa_ptrhash_free(&nsh);
				return -1;
			}
			for (pos = LL_LIST_FIRST(&stc->tlist); pos != XFA_NULL;
			     pos = LL_LIST_NEXT(&stc->tlist, pos)) {
				trn = LL_LIST_ENTRY(pos, xfa_trans_t, tlink);
				if (xfa_ptrstack_push(&nsp, trn->to) < 0) {
					xfa_ptrstack_free(&nsp);
					xfa_ptrhash_free(&nsh);
					return -1;
				}
			}
		}
	}
	xfa_ptrstack_free(&nsp);
	xfa_ptrhash_free(&nsh);

	return 0;
}

static xfa_state_t *xfa_eps_closure(xfa_system_t *sys, xfa_state_t *stt) {
	xfa_state_t *cls;
	xfa_ptrhash_node_t *hn;
	xfa_state_t *stc;
	struct ll_list_head *pos;
	xfa_trans_t *trn;
	xfa_ptrstack_t nsp;
	xfa_ptrhash_enum_t enh;

	if ((cls = xfa_alloc_state(sys, XFA_DEF_PHASH_SIZE)) == XFA_NULL)
		return XFA_NULL;
	if (xfa_ptrstack_create(sys, &nsp) < 0) {
		xfa_free_state(sys, cls, 0);
		return XFA_NULL;
	}
	if ((hn = xfa_ptrhash_first(&stt->sh, &enh)) != XFA_NULL) {
		do {
			if (xfa_ptrstack_push(&nsp, hn->ptr) < 0) {
				xfa_ptrstack_free(&nsp);
				xfa_free_state(sys, cls, 0);
				return XFA_NULL;
			}
			while ((stc = (xfa_state_t *) xfa_ptrstack_pop(&nsp)) != XFA_NULL) {
				if (xfa_ptrhash_get(&cls->sh, stc, XFA_NULL) == XFA_NULL) {
					if (xfa_state_add(cls, stc, XFA_NULL, 0) < 0) {
						xfa_ptrstack_free(&nsp);
						xfa_free_state(sys, cls, 0);
						return XFA_NULL;
					}
					for (pos = LL_LIST_FIRST(&stc->tlist); pos != XFA_NULL;
					     pos = LL_LIST_NEXT(&stc->tlist, pos)) {
						trn = LL_LIST_ENTRY(pos, xfa_trans_t, tlink);
						if ((trn->flags & XFATF_EPSTRANS) &&
						    xfa_ptrstack_push(&nsp, trn->to) < 0) {
							xfa_ptrstack_free(&nsp);
							xfa_free_state(sys, cls, 0);
							return XFA_NULL;
						}
					}
				}
			}
		} while ((hn = xfa_ptrhash_next(&enh)) != XFA_NULL);
	}
	xfa_ptrstack_free(&nsp);

	return cls;
}

static void xfa_free_move(xfa_system_t *sys, xfa_move_t *mvs) {
	XFASYS_FREE(sys, mvs);
}

static int xfa_state_moves(xfa_system_t *sys, xfa_state_t *stt, struct ll_list_head *mlist) {
	xfa_ptrhash_node_t *hn;
	xfa_state_t *stn;
	xfa_move_t *mvs;
	struct ll_list_head *pos;
	xfa_trans_t *trn;
	xfa_ptrhash_enum_t enh;

	LL_INIT_LIST_HEAD(mlist);
	if ((hn = xfa_ptrhash_first(&stt->sh, &enh)) != XFA_NULL) {
		do {
			stn = (xfa_state_t *) hn->ptr;
			for (pos = LL_LIST_FIRST(&stn->tlist); pos != XFA_NULL;
			     pos = LL_LIST_NEXT(&stn->tlist, pos)) {
				trn = LL_LIST_ENTRY(pos, xfa_trans_t, tlink);
				if ((trn->flags & XFATF_EPSTRANS) == 0 && trn->map != XFA_NULL) {
					if ((mvs = (xfa_move_t *)
					     XFASYS_ALLOC(sys, sizeof(xfa_move_t))) == XFA_NULL) {
						xfa_gen_freelist(sys, mlist,
								 (void (*)(xfa_system_t *, void *))
								 xfa_free_move,
								 XFA_OFFSETOF(xfa_move_t, link));
						XFASYS_ERROR(sys, XFAE_ALLOC);
						return -1;
					}
					LL_LIST_ADDT(&mvs->link, mlist);
					mvs->flags = trn->flags;
					mvs->to = trn->to;
					xfa_mapcpy(mvs->map, trn->map, XFA_TRANSMAP_ULONGS);
				}
			}
		} while ((hn = xfa_ptrhash_next(&enh)) != XFA_NULL);
	}

	return 0;
}

static xfa_state_t *xfa_get_move_of(xfa_system_t *sys, struct ll_list_head *mlist,
				    struct ll_list_head *pos) {
	xfa_state_t *stt;
	xfa_move_t *mvs, *cmvs;
	unsigned long tmap[XFA_TRANSMAP_ULONGS];

	if ((stt = xfa_alloc_state(sys, XFA_DEF_PHASH_SIZE)) == XFA_NULL)
		return XFA_NULL;
	mvs = LL_LIST_ENTRY(pos, xfa_move_t, link);
	if (xfa_state_add(stt, mvs->to, XFA_NULL, 0) < 0) {
		xfa_free_state(sys, stt, 0);
		return XFA_NULL;
	}
	for (pos = LL_LIST_NEXT(mlist, pos); pos != XFA_NULL; pos = LL_LIST_NEXT(mlist, pos)) {
		cmvs = LL_LIST_ENTRY(pos, xfa_move_t, link);
		xfa_memcpy(tmap, cmvs->map, XFA_MAPSIZE);
		xfa_and_map(tmap, mvs->map, XFA_TRANSMAP_ULONGS);
		if (xfa_map_zero(tmap, XFA_TRANSMAP_ULONGS))
			continue;
		if (xfa_state_add(stt, cmvs->to, XFA_NULL, 1) < 0) {
			xfa_free_state(sys, stt, 0);
			return XFA_NULL;
		}
		xfa_not_map(tmap, XFA_TRANSMAP_ULONGS);
		xfa_and_map(cmvs->map, tmap, XFA_TRANSMAP_ULONGS);
	}

	return stt;
}

static xfa_state_t *xfa_add_nfastate(xfa_system_t *sys, xfa_ptrhash_t *nsh, xfa_state_t *stt) {
	xfa_ptrhash_node_t *hn;
	xfa_state_t *stn;
	xfa_ptrhash_enum_t enh;

	if ((hn = xfa_ptrhash_first(nsh, &enh)) != XFA_NULL) {
		do {
			stn = (xfa_state_t *) hn->ptr;
			if (xfa_ptrhash_cmp(&stn->sh, &stt->sh) == 0)
				return stn;
		} while ((hn = xfa_ptrhash_next(&enh)) != XFA_NULL);
	}

	return xfa_ptrhash_add(nsh, stt, XFA_NULL, 0) == XFA_NULL ? XFA_NULL : stt;
}

static int xfa_process_moves(xfa_system_t *sys, xfa_state_t *stn,
			     struct ll_list_head *mlist, xfa_ptrhash_t *nsh) {
	int sadd;
	struct ll_list_head *pos;
	xfa_move_t *mvs;
	xfa_state_t *stm, *cls;

	for (sadd = 0, pos = LL_LIST_FIRST(mlist); pos != XFA_NULL; pos = LL_LIST_NEXT(mlist, pos)) {
		mvs = LL_LIST_ENTRY(pos, xfa_move_t, link);
		if (xfa_map_zero(mvs->map, XFA_TRANSMAP_ULONGS))
			continue;
		if ((stm = xfa_get_move_of(sys, mlist, pos)) == XFA_NULL)
			return -1;
		if ((cls = xfa_eps_closure(sys, stm)) == XFA_NULL) {
			xfa_free_state(sys, stm, 0);
			return -1;
		}
		xfa_free_state(sys, stm, 0);
		if ((stm = xfa_add_nfastate(sys, nsh, cls)) == XFA_NULL) {
			xfa_free_state(sys, cls, 0);
			return -1;
		}
		sadd += stm == cls ? 1 : 0;
		if (xfa_alloc_trans(sys, mvs->map, mvs->flags, stn, stm) == XFA_NULL) {
			xfa_free_state(sys, cls, 0);
			return -1;
		}
		if (stm != cls)
			xfa_free_state(sys, cls, 0);
	}

	return sadd;
}

static int xfa_hash2stack_copy(xfa_ptrhash_t *nsh, xfa_ptrstack_t *sp) {
	int count;
	xfa_ptrhash_node_t *hn;
	xfa_ptrhash_enum_t enh;

	for (count = 0, hn = xfa_ptrhash_first(nsh, &enh);
	     hn != XFA_NULL; hn = xfa_ptrhash_next(&enh))
		if (xfa_ptrstack_push(sp, hn->ptr) < 0)
			return -1;

	return count;
}

xfa_state_t *xfa_nfa2dfa(xfa_system_t *sys, xfa_state_t *stt) {
	int dadd, res;
	xfa_state_t *stn, *nfsr;
	xfa_ptrhash_node_t *hn;
	xfa_ptrhash_t nsh, mksh;
	xfa_ptrhash_enum_t enh;
	struct ll_list_head mlist;
	xfa_ptrstack_t sp;

	if (xfa_ptrhash_create(sys, &nsh, XFA_NFASREP_PHASH_SIZE) < 0)
		return XFA_NULL;
	if (xfa_ptrhash_create(sys, &mksh, XFA_NFASREP_PHASH_SIZE) < 0) {
		xfa_ptrhash_free(&nsh);
		return XFA_NULL;
	}
	if ((stn = xfa_alloc_state(sys, XFA_DEF_PHASH_SIZE)) == XFA_NULL) {
		xfa_ptrhash_free(&mksh);
		xfa_ptrhash_free(&nsh);
		return XFA_NULL;
	}
	if (xfa_state_add(stn, stt, XFA_NULL, 0) < 0 ||
	    (nfsr = xfa_eps_closure(sys, stn)) == XFA_NULL) {
		xfa_free_state(sys, stn, 0);
		xfa_ptrhash_free(&mksh);
		xfa_ptrhash_free(&nsh);
		return XFA_NULL;
	}
	xfa_free_state(sys, stn, 0);
	xfa_ptrstack_create(sys, &sp);
	if (xfa_ptrhash_add(&nsh, nfsr, XFA_NULL, 0) == XFA_NULL)
		goto error;
	do {
		if (xfa_hash2stack_copy(&nsh, &sp) < 0)
			goto error;
		dadd = 0;
		while ((stn = (xfa_state_t *)
			xfa_ptrstack_pop(&sp)) != XFA_NULL) {
			if (xfa_ptrhash_get(&mksh, stn, XFA_NULL) != XFA_NULL)
				continue;
			if (xfa_ptrhash_add(&mksh, stn, XFA_NULL, 0) == XFA_NULL ||
			    xfa_state_moves(sys, stn, &mlist) < 0)
				goto error;
			res = xfa_process_moves(sys, stn, &mlist, &nsh);
			xfa_gen_freelist(sys, &mlist,
					 (void (*)(xfa_system_t *, void *))
					 xfa_free_move, XFA_OFFSETOF(xfa_move_t,
								     link));
			if (res < 0)
				goto error;
			dadd += res;
		}
	} while (dadd > 0);
	xfa_ptrstack_free(&sp);
	xfa_ptrhash_free(&mksh);
	xfa_ptrhash_free(&nsh);

	return nfsr;

error:
	xfa_ptrstack_free(&sp);
	xfa_free_state(sys, nfsr, 1);
	xfa_ptrhash_free(&mksh);
	xfa_ptrhash_free(&nsh);
	return XFA_NULL;
}

static int xfa_move_trans(xfa_system_t *sys, xfa_state_t *stt, xfa_trans_t *trn,
			  xfa_state_t *from, xfa_state_t *to) {
	struct ll_list_head *pos;
	xfa_trans_t *ctrn;
	unsigned long map[XFA_TRANSMAP_ULONGS], cmap[XFA_TRANSMAP_ULONGS];

	if (trn->map != XFA_NULL)
		xfa_mapcpy(map, trn->map, XFA_TRANSMAP_ULONGS);
	for (pos = LL_LIST_FIRST(&stt->tlist); pos != XFA_NULL;
	     pos = LL_LIST_NEXT(&stt->tlist, pos)) {
		ctrn = LL_LIST_ENTRY(pos, xfa_trans_t, tlink);
		if (ctrn->from != from || ctrn->to != to)
			continue;
		if ((ctrn->flags & XFATF_EPSTRANS) && (trn->flags & XFATF_EPSTRANS)) {
			xfa_free_trans(sys, trn);
			return 0;
		}
		if (ctrn->map != XFA_NULL && trn->map != XFA_NULL) {
			xfa_mapcpy(cmap, ctrn->map, XFA_TRANSMAP_ULONGS);
			xfa_not_map(cmap, XFA_TRANSMAP_ULONGS);
			xfa_and_map(map, cmap, XFA_TRANSMAP_ULONGS);
			if (xfa_map_zero(map, XFA_TRANSMAP_ULONGS)) {
				xfa_free_trans(sys, trn);
				return 0;
			}
		}
	}
	for (pos = LL_LIST_FIRST(&stt->flist); pos != XFA_NULL;
	     pos = LL_LIST_NEXT(&stt->flist, pos)) {
		ctrn = LL_LIST_ENTRY(pos, xfa_trans_t, flink);
		if (ctrn->from != from || ctrn->to != to)
			continue;
		if ((ctrn->flags & XFATF_EPSTRANS) && (trn->flags & XFATF_EPSTRANS)) {
			xfa_free_trans(sys, trn);
			return 0;
		}
		if (ctrn->map != XFA_NULL && trn->map != XFA_NULL) {
			xfa_mapcpy(cmap, ctrn->map, XFA_TRANSMAP_ULONGS);
			xfa_not_map(cmap, XFA_TRANSMAP_ULONGS);
			xfa_and_map(map, cmap, XFA_TRANSMAP_ULONGS);
			if (xfa_map_zero(map, XFA_TRANSMAP_ULONGS)) {
				xfa_free_trans(sys, trn);
				return 0;
			}
		}
	}
	if (trn->map != XFA_NULL)
		xfa_mapcpy(trn->map, map, XFA_TRANSMAP_ULONGS);
	xfa_relink_trans(trn, from, to);

	return 0;
}

static int xfa_merge_states(xfa_system_t *sys, xfa_state_t *stt) {
	xfa_ptrhash_node_t *hn;
	xfa_state_t *stn, *from, *to;
	struct ll_list_head *pos;
	xfa_trans_t *trn;
	xfa_ptrhash_enum_t enh;

	if ((hn = xfa_ptrhash_first(&stt->sh, &enh)) != XFA_NULL) {
		do {
			stn = (xfa_state_t *) hn->ptr;
			if (stn->flags & XFASF_TRANSFERABLE)
				stt->flags |= stn->flags & XFASF_TRANSFERABLE;
			while ((pos = LL_LIST_FIRST(&stn->tlist)) != XFA_NULL) {
				trn = LL_LIST_ENTRY(pos, xfa_trans_t, tlink);
				to = trn->to;
				if (xfa_ptrhash_get(&stt->sh, to, XFA_NULL) != XFA_NULL)
					to = stt;
				if (xfa_move_trans(sys, stt, trn, stt, to) < 0)
					return -1;
			}
			while ((pos = LL_LIST_FIRST(&stn->flist)) != XFA_NULL) {
				trn = LL_LIST_ENTRY(pos, xfa_trans_t, flink);
				from = trn->from;
				if (xfa_ptrhash_get(&stt->sh, from, XFA_NULL) != XFA_NULL)
					from = stt;
				if (xfa_move_trans(sys, stt, trn, from, stt) < 0)
					return -1;
			}
			xfa_free_state(sys, stn, 0);
		} while ((hn = xfa_ptrhash_next(&enh)) != XFA_NULL);
	}

	return 0;
}

static int xfa_label_proc(void *priv, xfa_system_t *sys, xfa_state_t *stt) {
	unsigned long *label = (unsigned long *) priv;

	stt->label = (*label)++;

	return 0;
}

int xfa_label_states(xfa_system_t *sys, xfa_state_t *stt, unsigned long *label) {
	return xfa_do_once(sys, stt, xfa_label_proc, label);
}

static int xfa_getptr_proc(void *priv, xfa_system_t *sys, xfa_state_t *stt) {
	xfa_state_t ***psts = (xfa_state_t ***) priv;

	*(*psts)++ = stt;

	return 0;
}

static int xfa_compare_states(xfa_state_t *sta, xfa_state_t *stb,
			      unsigned long *pmap, unsigned long nsts) {
	unsigned long lnl, albl, blbl, tmp;
	struct ll_list_head *apos, *bpos;
	xfa_trans_t *atrn, *btrn;
	unsigned long map[XFA_TRANSMAP_ULONGS], cmap[XFA_TRANSMAP_ULONGS];

	lnl = nsts / XFA_ULONG_BITS + 1;
	for (apos = LL_LIST_FIRST(&sta->tlist); apos != XFA_NULL;
	     apos = LL_LIST_NEXT(&sta->tlist, apos)) {
		atrn = LL_LIST_ENTRY(apos, xfa_trans_t, tlink);
		if (atrn->map != XFA_NULL)
			xfa_mapcpy(map, atrn->map, XFA_TRANSMAP_ULONGS);
		for (bpos = LL_LIST_FIRST(&stb->tlist); bpos != XFA_NULL;
		     bpos = LL_LIST_NEXT(&stb->tlist, bpos)) {
			btrn = LL_LIST_ENTRY(bpos, xfa_trans_t, tlink);
			if ((atrn->flags & XFATF_EPSTRANS) && (btrn->flags & XFATF_EPSTRANS)) {
				albl = atrn->to->label;
				blbl = btrn->to->label;
				if (albl < blbl)
					tmp = albl, albl = blbl, blbl = tmp;
				if (pmap[albl * lnl + blbl / XFA_ULONG_BITS] &
				    (1 << (blbl % XFA_ULONG_BITS))) {
					albl = sta->label;
					blbl = stb->label;
					if (albl < blbl)
						tmp = albl, albl = blbl, blbl = tmp;
					pmap[albl * lnl + blbl / XFA_ULONG_BITS] |=
						(1 << (blbl % XFA_ULONG_BITS));
					return 1;
				}
				break;
			}
			if (atrn->map != XFA_NULL && btrn->map != XFA_NULL) {
				xfa_mapcpy(cmap, btrn->map, XFA_TRANSMAP_ULONGS);
				xfa_and_map(cmap, map, XFA_TRANSMAP_ULONGS);
				if (!xfa_map_zero(cmap, XFA_TRANSMAP_ULONGS)) {
					albl = atrn->to->label;
					blbl = btrn->to->label;
					if (albl < blbl)
						tmp = albl, albl = blbl, blbl = tmp;
					if (pmap[albl * lnl + blbl / XFA_ULONG_BITS] &
					    (1 << (blbl % XFA_ULONG_BITS))) {
						albl = sta->label;
						blbl = stb->label;
						if (albl < blbl)
							tmp = albl, albl = blbl, blbl = tmp;
						pmap[albl * lnl + blbl / XFA_ULONG_BITS] |=
							(1 << (blbl % XFA_ULONG_BITS));
						return 1;
					}
					xfa_not_map(cmap, XFA_TRANSMAP_ULONGS);
					xfa_and_map(map, cmap, XFA_TRANSMAP_ULONGS);
				}
			}
		}
		if ((atrn->map != XFA_NULL &&
		     !xfa_map_zero(map, XFA_TRANSMAP_ULONGS)) ||
		    (atrn->map == XFA_NULL && bpos == XFA_NULL)) {
			albl = sta->label;
			blbl = stb->label;
			if (albl < blbl)
				tmp = albl, albl = blbl, blbl = tmp;
			pmap[albl * lnl + blbl / XFA_ULONG_BITS] |= (1 << (blbl % XFA_ULONG_BITS));
			return 1;
		}
	}
	for (apos = LL_LIST_FIRST(&stb->tlist); apos != XFA_NULL;
	     apos = LL_LIST_NEXT(&stb->tlist, apos)) {
		atrn = LL_LIST_ENTRY(apos, xfa_trans_t, tlink);
		if (atrn->map != XFA_NULL)
			xfa_mapcpy(map, atrn->map, XFA_TRANSMAP_ULONGS);
		for (bpos = LL_LIST_FIRST(&sta->tlist); bpos != XFA_NULL;
		     bpos = LL_LIST_NEXT(&sta->tlist, bpos)) {
			btrn = LL_LIST_ENTRY(bpos, xfa_trans_t, tlink);
			if ((atrn->flags & XFATF_EPSTRANS) && (btrn->flags & XFATF_EPSTRANS))
				break;
			if (atrn->map != XFA_NULL && btrn->map != XFA_NULL) {
				xfa_mapcpy(cmap, btrn->map, XFA_TRANSMAP_ULONGS);
				xfa_and_map(cmap, map, XFA_TRANSMAP_ULONGS);
				if (!xfa_map_zero(cmap, XFA_TRANSMAP_ULONGS)) {
					xfa_not_map(cmap, XFA_TRANSMAP_ULONGS);
					xfa_and_map(map, cmap, XFA_TRANSMAP_ULONGS);
				}
			}
		}
		if ((atrn->map != XFA_NULL &&
		     !xfa_map_zero(map, XFA_TRANSMAP_ULONGS)) ||
		    (atrn->map == XFA_NULL && bpos == XFA_NULL)) {
			albl = sta->label;
			blbl = stb->label;
			if (albl < blbl)
				tmp = albl, albl = blbl, blbl = tmp;
			pmap[albl * lnl + blbl / XFA_ULONG_BITS] |= (1 << (blbl % XFA_ULONG_BITS));
			return 1;
		}
	}

	return 0;
}

int xfa_dfa_minimize(xfa_system_t *sys, xfa_state_t *stt) {
	int res, marked;
	unsigned long nsts = 0, lnl, i, j;
	xfa_state_t *stn, **psts, **tsts, *sta, *stb;
	unsigned long *pmap, *pln;

	xfa_label_states(sys, stt, &nsts);
	if ((psts = (xfa_state_t **) XFASYS_ALLOC(sys, nsts * sizeof(xfa_state_t *))) == XFA_NULL) {
		XFASYS_ERROR(sys, XFAE_ALLOC);
		return -1;
	}
	tsts = psts;
	if (xfa_do_once(sys, stt, xfa_getptr_proc, &tsts) < 0) {
		XFASYS_FREE(sys, psts);
		return -1;
	}
	lnl = nsts / XFA_ULONG_BITS + 1;
	if ((pmap = (unsigned long *) XFASYS_ALLOC(sys, lnl * nsts *
						   sizeof(unsigned long))) == XFA_NULL) {
		XFASYS_FREE(sys, psts);
		return -1;
	}
	xfa_memset(pmap, 0, lnl * nsts * sizeof(unsigned long));
	for (i = 0; i < nsts; i++) {
		sta = psts[i];
		pln = &pmap[i * lnl];
		for (j = 0; j < i; j++) {
			stb = psts[j];
			if ((sta->flags & XFASF_TRANSFERABLE) != (stb->flags & XFASF_TRANSFERABLE))
				pln[j / XFA_ULONG_BITS] |= 1 << (j % XFA_ULONG_BITS);
		}
	}
	do {
		marked = 0;
		for (i = 0; i < nsts; i++) {
			pln = &pmap[i * lnl];
			for (j = 0; j < i; j++) {
				if (pln[j / XFA_ULONG_BITS] & (1 << (j % XFA_ULONG_BITS)))
					continue;
				sta = psts[i];
				stb = psts[j];
				if ((res = xfa_compare_states(sta, stb, pmap, nsts)) < 0) {
					XFASYS_FREE(sys, pmap);
					XFASYS_FREE(sys, psts);
					return -1;
				}
				marked += res;
			}
		}
	} while (marked);
	if ((tsts = (xfa_state_t **) XFASYS_ALLOC(sys, nsts * sizeof(xfa_state_t *))) == XFA_NULL) {
		XFASYS_ERROR(sys, XFAE_ALLOC);
		XFASYS_FREE(sys, pmap);
		XFASYS_FREE(sys, psts);
		return -1;
	}
	for (i = 0; i < nsts; i++) {
		tsts[i] = XFA_NULL;
		sta = psts[i];
		pln = &pmap[i * lnl];
		for (j = 0; j < i; j++) {
			if (pln[j / XFA_ULONG_BITS] & (1 << (j % XFA_ULONG_BITS)))
				continue;
			stb = psts[j];
			if (tsts[i] != XFA_NULL)
				stn = tsts[j] = tsts[i];
			else if (tsts[j] != XFA_NULL)
				stn = tsts[i] = tsts[j];
			else {
				if ((stn = xfa_alloc_state(sys, XFA_DEF_PHASH_SIZE)) == XFA_NULL) {
					for (j = 0; j <= i; j++)
						if (tsts[j] != XFA_NULL)
							xfa_free_state(sys, tsts[j], 0);
					XFASYS_FREE(sys, tsts);
					XFASYS_FREE(sys, pmap);
					XFASYS_FREE(sys, psts);
					return -1;
				}
				tsts[i] = tsts[j] = stn;
			}
			if (xfa_state_add(stn, sta, XFA_NULL, 1) < 0 ||
			    xfa_state_add(stn, stb, XFA_NULL, 1) < 0) {
				for (j = 0; j <= i; j++)
					if (tsts[j] != XFA_NULL)
						xfa_free_state(sys, tsts[j], 0);
				XFASYS_FREE(sys, tsts);
				XFASYS_FREE(sys, pmap);
				XFASYS_FREE(sys, psts);
				return -1;
			}
		}
	}
	XFASYS_FREE(sys, pmap);
	XFASYS_FREE(sys, psts);
	for (i = 0; i < nsts; i++) {
		if (tsts[i] == XFA_NULL)
			continue;
		xfa_merge_states(sys, tsts[i]);
		for (j = i + 1; j < nsts; j++)
			if (tsts[i] == tsts[j])
				tsts[j] = XFA_NULL;
	}
	XFASYS_FREE(sys, tsts);

	return 0;
}

