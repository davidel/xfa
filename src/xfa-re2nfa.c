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

#define XFA_ESCAPE_ISMAP 256


static int xfa_parse_escape(xfa_system_t *sys, unsigned long *d, int n,
			    unsigned int *pval, unsigned char const **ptr) {
	unsigned char const *str = *ptr;
	unsigned long cmap[XFA_TRANSMAP_ULONGS];

	if (*str != '\\') {
		XFASYS_ERROR(sys, XFAE_ESC_PARSE);
		return -1;
	}
	str++;
	if (*str == 'x') {
		str++;
		if (!XFA_ISXDIGIT(str[0]) || !XFA_ISXDIGIT(str[1])) {
			XFASYS_ERROR(sys, XFAE_HEX_PARSE);
			return -1;
		}
		*pval = 16 * (unsigned int) XFA_XVALUE(str[0]) + (unsigned int) XFA_XVALUE(str[1]);
		str += 2;
	} else if (*str == 'r')
		*pval = '\r', str++;
	else if (*str == 'n')
		*pval = '\n', str++;
	else if (*str == 't')
		*pval = '\t', str++;
	else if (*str == 'b')
		*pval = ' ', str++;
	else if (*str == 'f')
		*pval = '\f', str++;
	else if (*str == 'a')
		*pval = '\a', str++;
	else if (*str == 'e')
		*pval = 0x1b, str++;
	else if (*str == 's' || *str == 'S') {
		xfa_map_init(cmap, XFA_TRANSMAP_ULONGS);
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, ' ', ' ');
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, '\t', '\t');
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, '\r', '\r');
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, '\n', '\n');
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, '\f', '\f');
		if (*str == 'S')
			xfa_not_map(d, n);
		str++;
		*pval = XFA_ESCAPE_ISMAP;
	} else if (*str == 'w' || *str == 'W') {
		xfa_map_init(cmap, XFA_TRANSMAP_ULONGS);
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, 'a', 'z');
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, 'A', 'Z');
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, '0', '9');
		if (*str == 'W')
			xfa_not_map(d, n);
		str++;
		*pval = XFA_ESCAPE_ISMAP;
	} else if (*str == 'd' || *str == 'D') {
		xfa_map_init(cmap, XFA_TRANSMAP_ULONGS);
		xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, '0', '9');
		if (*str == 'D')
			xfa_not_map(d, n);
		str++;
		*pval = XFA_ESCAPE_ISMAP;
	} else
		*pval = *str++;
	if (*pval != XFA_ESCAPE_ISMAP)
		xfa_map_set_range(d, n, (int) *pval, (int) *pval);
	else
		xfa_or_map(d, cmap, n);
	*ptr = str;

	return 0;
}

static int xfa_parse_class(xfa_system_t *sys, unsigned long *d, int n, unsigned char const **ptr) {
	int inv = 0;
	unsigned int eval, evaln;
	unsigned char const *str = *ptr;
	unsigned long cmap[XFA_TRANSMAP_ULONGS];

	if (*str != '[') {
		XFASYS_ERROR(sys, XFAE_CLASS_PARSE);
		return -1;
	}
	str++;
	if (*str == '^')
		str++, inv++;
	xfa_map_init(d, n);
	for (; *str && *str != ']';) {
		xfa_map_init(cmap, XFA_TRANSMAP_ULONGS);
		if (*str == '\\') {
			if (xfa_parse_escape(sys, cmap, XFA_TRANSMAP_ULONGS, &eval, &str) < 0)
				return -1;
		} else {
			eval = *str++;
			xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, (int) eval, (int) eval);
		}
		if (*str == '-') {
			str++;
			if (eval == XFA_ESCAPE_ISMAP) {
				XFASYS_ERROR(sys, XFAE_CLASS_PARSE);
				return -1;
			}
			if (*str == '\\') {
				xfa_map_init(cmap, XFA_TRANSMAP_ULONGS);
				if (xfa_parse_escape(sys, cmap, XFA_TRANSMAP_ULONGS,
						     &evaln, &str) < 0)
					return -1;
				if (evaln == XFA_ESCAPE_ISMAP) {
					XFASYS_ERROR(sys, XFAE_CLASS_PARSE);
					return -1;
				}
			} else
				evaln = *str++;
			if (eval > evaln) {
				XFASYS_ERROR(sys, XFAE_CLASS_PARSE);
				return -1;
			}
			xfa_map_init(cmap, XFA_TRANSMAP_ULONGS);
			xfa_map_set_range(cmap, XFA_TRANSMAP_ULONGS, (int) eval, (int) evaln);
		}
		xfa_or_map(d, cmap, n);
	}
	if (*str != ']') {
		XFASYS_ERROR(sys, XFAE_CLASS_PARSE);
		return -1;
	}
	if (inv)
		xfa_not_map(d, n);
	*ptr = str + 1;

	return 0;
}


static int xfa_next_map(xfa_system_t *sys, unsigned long *d, int n, unsigned char const **ptr) {
	unsigned int eval;
	unsigned char const *str = *ptr;

	if (*str == '[')
		return xfa_parse_class(sys, d, n, ptr);
	xfa_map_init(d, n);
	if (*str == '\\')
		return xfa_parse_escape(sys, d, n, &eval, ptr);
	if (*str == '.')
		xfa_not_map(d, n);
	else
		xfa_map_set_range(d, n, *str, *str);
	*ptr = str + 1;

	return 0;
}

static int xfa_get_rep_count(xfa_system_t *sys, int *rep, unsigned char const **ptr) {
	unsigned char const *str = *ptr;

	if (*str == '*') {
		rep[0] = 0;
		rep[1] = -1;
		str++;
	} else if (*str == '+') {
		rep[0] = 1;
		rep[1] = -1;
		str++;
	} else if (*str == '?') {
		rep[0] = 0;
		rep[1] = 1;
		str++;
	} else if (*str == '{') {
		str++;
		if (*str == ',')
			rep[0] = 0, str++;
		else {
			if (!XFA_ISDIGIT(*str)) {
				XFASYS_ERROR(sys, XFAE_REPCNT_PARSE);
				return -1;
			}
			rep[0] = (int) xfa_atol((char const *) str);
			for (str++; XFA_ISDIGIT(*str); str++);
		}
		if (*str == '}')
			rep[1] = rep[0], str++;
		else {
			if (*str == ',')
				str++;
			if (*str == '}')
				rep[1] = -1, str++;
			else {
				if (!XFA_ISDIGIT(*str)) {
					XFASYS_ERROR(sys, XFAE_REPCNT_PARSE);
					return -1;
				}
				rep[1] = (int) xfa_atol((char const *) str);
				for (str++; XFA_ISDIGIT(*str); str++);
				if (*str++ != '}') {
					XFASYS_ERROR(sys, XFAE_REPCNT_PARSE);
					return -1;
				}
			}
		}
		if ((rep[0] > rep[1] && rep[1] != -1) || rep[1] == 0) {
			XFASYS_ERROR(sys, XFAE_REPCNT_PARSE);
			return -1;
		}
	} else
		rep[0] = rep[1] = 1;
	*ptr = str;

	return 0;
}


static int xfa_nfa_clone_rec(xfa_system_t *sys, xfa_state_t *sts, xfa_state_t *csts,
			     xfa_state_t *ste, xfa_state_t *cste, xfa_ptrhash_t *nsh) {
	struct ll_list_head *pos;
	xfa_ptrhash_node_t *hn;
	xfa_trans_t *trn;
	xfa_state_t *to, *ato;

	if (xfa_ptrhash_get(nsh, sts, XFA_NULL) == XFA_NULL) {
		if (xfa_ptrhash_add(nsh, sts, csts, 0) == XFA_NULL)
			return -1;
		for (pos = LL_LIST_FIRST(&sts->tlist); pos != XFA_NULL;
		     pos = LL_LIST_NEXT(&sts->tlist, pos)) {
			trn = LL_LIST_ENTRY(pos, xfa_trans_t, tlink);
			ato = XFA_NULL;
			if ((hn = xfa_ptrhash_get(nsh, trn->to, XFA_NULL)) == XFA_NULL) {
				if (trn->to == sts)
					to = csts;
				else if (trn->to == ste)
					to = cste;
				else if ((to = ato = xfa_alloc_state(sys, 0)) == XFA_NULL)
					return -1;
			} else
				to = (xfa_state_t *) hn->val;
			if (xfa_alloc_trans(sys, trn->map, trn->flags, csts, to) == XFA_NULL) {
				if (ato != XFA_NULL)
					xfa_free_state(sys, ato, 0);
				return -1;
			}
			if (xfa_nfa_clone_rec(sys, trn->to, to, ste, cste, nsh) < 0)
				return -1;
		}
	}

	return 0;
}

static int xfa_nfa_clone(xfa_system_t *sys, xfa_state_t *sts, xfa_state_t *ste,
			 xfa_state_t **csts, xfa_state_t **cste) {
	int res;
	xfa_ptrhash_t nsh;

	if (xfa_ptrhash_create(sys, &nsh, XFA_NFASREP_PHASH_SIZE) < 0)
		return -1;
	if ((*csts = xfa_alloc_state(sys, 0)) == XFA_NULL) {
		xfa_ptrhash_free(&nsh);
		return -1;
	}
	if ((*cste = xfa_alloc_state(sys, 0)) == XFA_NULL) {
		xfa_free_state(sys, *csts, 0);
		xfa_ptrhash_free(&nsh);
		return -1;
	}
	res = xfa_nfa_clone_rec(sys, sts, *csts, ste, *cste, &nsh);
	xfa_ptrhash_free(&nsh);
	if (res < 0) {
		xfa_free_state(sys, *cste, 0);
		xfa_free_state(sys, *csts, 1);
	}

	return res;
}

static int xfa_add_rep(xfa_system_t *sys, xfa_state_t *inp, xfa_state_t **outp,
		       xfa_state_t *sts, xfa_state_t *ste, int *rep) {
	int i;
	xfa_state_t *stc, *csts, *cste, *sto;

	stc = inp;
	for (i = 0; i < rep[0]; i++) {
		if (xfa_nfa_clone(sys, sts, ste, &csts, &cste) < 0)
			return -1;
		if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, csts) == XFA_NULL) {
			xfa_free_state(sys, csts, 1);
			return -1;
		}
		stc = cste;
	}
	sto = stc;
	if (rep[1] < 0) {
		if (xfa_nfa_clone(sys, sts, ste, &csts, &cste) < 0)
			return -1;
		if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, csts) == XFA_NULL) {
			xfa_free_state(sys, csts, 1);
			return -1;
		}
		if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, cste) == XFA_NULL ||
		    xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, cste, stc) == XFA_NULL)
			return -1;
		sto = cste;
	} else if (rep[0] < rep[1]) {
		if ((sto = xfa_alloc_state(sys, 0)) == XFA_NULL)
			return -1;
		if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, sto) == XFA_NULL) {
			xfa_free_state(sys, sto, 0);
			return -1;
		}
		for (i = rep[0]; i < rep[1]; i++) {
			if (xfa_nfa_clone(sys, sts, ste, &csts, &cste) < 0)
				return -1;
			if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, csts) == XFA_NULL) {
				xfa_free_state(sys, csts, 1);
				return -1;
			}
			if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, cste, sto) == XFA_NULL)
				return -1;
			stc = cste;
		}
	}
	*outp = sto;

	return 0;
}

static int xfa_regex_parse(xfa_system_t *sys, xfa_state_t *inp, xfa_state_t **outp,
			   unsigned char const **ptr) {
	int i;
	xfa_state_t *stc, *stn, *sto;
	unsigned long map[XFA_TRANSMAP_ULONGS];
	int rep[2];

	if (xfa_next_map(sys, map, XFA_TRANSMAP_ULONGS, ptr) < 0 ||
	    xfa_get_rep_count(sys, rep, ptr) < 0)
		return -1;
	stc = inp;
	for (i = 0; i < rep[0]; i++) {
		if ((stn = xfa_alloc_state(sys, 0)) == XFA_NULL)
			return -1;
		if (xfa_alloc_trans(sys, map, 0, stc, stn) == XFA_NULL) {
			xfa_free_state(sys, stn, 0);
			return -1;
		}
		stc = stn;
	}
	if (stc == inp) {
		if ((sto = xfa_alloc_state(sys, 0)) == XFA_NULL)
			return -1;
		if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, sto) == XFA_NULL) {
			xfa_free_state(sys, sto, 0);
			return -1;
		}
		stc = sto;
	}
	sto = stc;
	if (rep[1] < 0) {
		if ((sto = xfa_alloc_state(sys, 0)) == XFA_NULL)
			return -1;
		if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, sto) == XFA_NULL) {
			xfa_free_state(sys, sto, 0);
			return -1;
		}
		if (xfa_alloc_trans(sys, map, 0, stc, sto) == XFA_NULL ||
		    xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, sto, stc) == XFA_NULL)
			return -1;
	} else if (rep[0] < rep[1]) {
		if ((sto = xfa_alloc_state(sys, 0)) == XFA_NULL)
			return -1;
		if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, sto) == XFA_NULL) {
			xfa_free_state(sys, sto, 0);
			return -1;
		}
		for (i = rep[0] + 1; i < rep[1]; i++) {
			if ((stn = xfa_alloc_state(sys, 0)) == XFA_NULL)
				return -1;
			if (xfa_alloc_trans(sys, map, 0, stc, stn) == XFA_NULL) {
				xfa_free_state(sys, stn, 0);
				return -1;
			}
			if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stn, sto) == XFA_NULL)
				return -1;
			stc = stn;
		}
		if (xfa_alloc_trans(sys, map, 0, stc, sto) == XFA_NULL)
			return -1;
	}
	*outp = sto;

	return 0;
}

static int xfa_regex_bld(xfa_system_t *sys, xfa_state_t *sts, xfa_state_t *ste,
			 unsigned char const **ptr, int level) {
	unsigned char const *str = *ptr;
	xfa_state_t *stc, *csts, *cste;
	int rep[2];

	if ((stc = xfa_alloc_state(sys, 0)) == XFA_NULL)
		return -1;
	if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, sts, stc) == XFA_NULL) {
		xfa_free_state(sys, stc, 0);
		return -1;
	}
	for (; *str;) {
		if (*str == '(') {
			str++;
			if ((csts = xfa_alloc_state(sys, 0)) == XFA_NULL)
				return -1;
			if ((cste = xfa_alloc_state(sys, 0)) == XFA_NULL) {
				xfa_free_state(sys, csts, 0);
				return -1;
			}
			if (xfa_regex_bld(sys, csts, cste, &str, level + 1) < 0 ||
			    xfa_get_rep_count(sys, rep, &str) < 0) {
				xfa_free_state(sys, cste, 0);
				xfa_free_state(sys, csts, 1);
				return -1;
			}
			if (xfa_add_rep(sys, stc, &stc, csts, cste, rep) < 0) {
				xfa_free_state(sys, cste, 0);
				xfa_free_state(sys, csts, 1);
				return -1;
			}
			xfa_free_state(sys, csts, 1);
			continue;
		} else if (*str == ')') {
			if (!level) {
				XFASYS_ERROR(sys, XFAE_REGEX_PARSE);
				return -1;
			}
			str++;
			break;
		} else if (*str == '|') {
			if (str == *ptr) {
				XFASYS_ERROR(sys, XFAE_REGEX_PARSE);
				return -1;
			}
			if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, ste) == XFA_NULL)
				return -1;
			if ((stc = xfa_alloc_state(sys, 0)) == XFA_NULL)
				return -1;
			if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, sts, stc) == XFA_NULL) {
				xfa_free_state(sys, stc, 0);
				return -1;
			}
			str++;
			continue;
		}
		if (xfa_regex_parse(sys, stc, &stc, &str) < 0)
			return -1;
	}
	if (xfa_alloc_trans(sys, XFA_NULL, XFATF_EPSTRANS, stc, ste) == XFA_NULL)
		return -1;
	*ptr = str;

	return 0;
}

int xfa_re2nfa(xfa_system_t *sys, xfa_state_t **csts, xfa_state_t **cste,
	       unsigned char const **ptr) {
	int res;

	if ((*csts = xfa_alloc_state(sys, 0)) == XFA_NULL)
		return -1;
	if ((*cste = xfa_alloc_state(sys, 0)) == XFA_NULL) {
		xfa_free_state(sys, *csts, 0);
		return -1;
	}
	res = xfa_regex_bld(sys, *csts, *cste, ptr, 0);
	if (res < 0) {
		xfa_free_state(sys, *cste, 0);
		xfa_free_state(sys, *csts, 1);
	} else {
		(*csts)->flags |= XFASF_START;
		(*cste)->flags |= XFASF_TARGET;
	}

	return res;
}

