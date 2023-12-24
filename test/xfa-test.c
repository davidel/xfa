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


#include <stdio.h>
#include <stdlib.h>
#include "xfa-include.h"


typedef struct s_xfa_sysif_ctx {
	int error;
} xfa_sysif_ctx_t;

typedef struct s_xfa_print_ctx {
	char const *pre;
} xfa_print_ctx_t;


static void *xfa_sys_alloc(void *priv, int size) {
	xfa_sysif_ctx_t *sys = (xfa_sysif_ctx_t *) priv;

	return malloc(size);
}

static void xfa_sys_free(void *priv, void *data) {
	xfa_sysif_ctx_t *sys = (xfa_sysif_ctx_t *) priv;

	free(data);
}

static void xfa_sys_error(void *priv, int error) {
	xfa_sysif_ctx_t *sys = (xfa_sysif_ctx_t *) priv;

	sys->error = error;
}

static int xfa_print_proc(void *priv, xfa_system_t * sys, xfa_state_t * stt) {
	xfa_print_ctx_t *psc = (xfa_print_ctx_t *) priv;
	struct ll_list_head *pos;
	xfa_trans_t *trn;
	char const *shape;

	for (pos = LL_LIST_FIRST(&stt->tlist); pos != XFA_NULL;
	     pos = LL_LIST_NEXT(&stt->tlist, pos)) {
		trn = LL_LIST_ENTRY(pos, xfa_trans_t, tlink);

		shape = stt->flags & XFASF_TARGET ? "doublecircle" :
			stt->flags & XFASF_START ? "box" : "circle";
		fprintf(stdout, "node [shape = %s]; %s%lu;\n", shape, psc->pre, stt->label);

		shape = trn->to->flags & XFASF_TARGET ? "doublecircle" :
			trn->to->flags & XFASF_START ? "box" : "circle";
		fprintf(stdout, "node [shape = %s]; %s%lu;\n", shape, psc->pre, trn->to->label);

		fprintf(stdout, "%s%lu -> %s%lu [ label = \"",
			psc->pre, stt->label, psc->pre, trn->to->label);

		if (trn->map != XFA_NULL) {
			int i;
			unsigned long *map = trn->map;

			for (i = 0; i < 256; i++)
				if (map[i / XFA_ULONG_BITS] & (1UL << (i % XFA_ULONG_BITS)))
					fprintf(stdout, "%c", i);
		} else
			fprintf(stdout, "{eps}");
		fprintf(stdout, "\" ];\n");
	}

	return 0;
}

static void xfa_print_fa(xfa_system_t * sys, xfa_state_t * dfa, char const *name,
			 char const *pre) {
	unsigned long label = 0;
	xfa_print_ctx_t psc;

	xfa_label_states(sys, dfa, &label);
	fprintf(stdout,
		"subgraph %s {\n" "rankdir=LR;\n" "size=\"20,10\"\n" "orientation=land;\n", name);
	psc.pre = pre;
	xfa_do_once(sys, dfa, xfa_print_proc, &psc);
	fprintf(stdout, "}\n");
}

int main(int ac, char **av) {
	unsigned char const *re;
	xfa_state_t *sts, *ste, *dfa;
	xfa_sysif_ctx_t sys;
	xfa_system_t xsys;

	if (ac < 2) {
		fprintf(stderr, "use: %s regex\n", av[0]);
		return 1;
	}
	re = (unsigned char const *) av[1];

	sys.error = 0;
	xsys.priv = &sys;
	xsys.alloc = xfa_sys_alloc;
	xsys.free = xfa_sys_free;
	xsys.error = xfa_sys_error;
	if (xfa_re2nfa(&xsys, &sts, &ste, &re) < 0) {
		fprintf(stderr, "failed to compile regex: re='%s' error=%d\n", av[1], sys.error);
		return 2;
	}

	fprintf(stdout, "digraph FA {\n" "rankdir=LR;\n" "size=\"20,10\"\n" "orientation=land;\n");

	xfa_print_fa(&xsys, sts, "NFA", "N");

	if ((dfa = xfa_nfa2dfa(&xsys, sts)) != XFA_NULL) {
		xfa_print_fa(&xsys, dfa, "DFA", "D");
		xfa_dfa_minimize(&xsys, dfa);
		xfa_print_fa(&xsys, dfa, "XFA", "X");
		xfa_free_state(&xsys, dfa, 1);
	} else
		fprintf(stderr, "error converting NFA to DFA: error=%d\n", sys.error);
	fprintf(stdout, "}\n");

	xfa_free_state(&xsys, sts, 1);

	return 0;
}

