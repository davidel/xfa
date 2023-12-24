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

#define XFA_STD_CLUSTER_SIZE 1024

static xfa_stack_cls_t *xfa_ptrstack_alloc_cls(xfa_system_t *sys, int size) {
	xfa_stack_cls_t *cls;

	if ((cls = (xfa_stack_cls_t *) XFASYS_ALLOC(sys, sizeof(xfa_stack_cls_t) +
						    size * sizeof(void *))) == XFA_NULL) {
		XFASYS_ERROR(sys, XFAE_ALLOC);
		return XFA_NULL;
	}
	LL_INIT_LIST_HEAD(&cls->link);
	cls->size = size;
	cls->count = 0;

	return cls;
}

int xfa_ptrstack_create(xfa_system_t *sys, xfa_ptrstack_t *sp) {
	sp->sys = sys;
	LL_INIT_LIST_HEAD(&sp->clist);

	return 0;
}

static void xfa_ptrstack_free_cls(xfa_system_t *sys, xfa_stack_cls_t *cls) {
	XFASYS_FREE(sys, cls);
}

void xfa_ptrstack_free(xfa_ptrstack_t *sp) {
	xfa_gen_freelist(sp->sys, &sp->clist,
			 (void (*)(xfa_system_t *, void *)) xfa_ptrstack_free_cls,
			 XFA_OFFSETOF(xfa_stack_cls_t, link));
}

static xfa_stack_cls_t *xfa_ptrstack_clscur(xfa_ptrstack_t *sp) {
	struct ll_list_head *pos;

	if ((pos = LL_LIST_LAST(&sp->clist)) == XFA_NULL)
		return XFA_NULL;

	return LL_LIST_ENTRY(pos, xfa_stack_cls_t, link);
}

int xfa_ptrstack_push(xfa_ptrstack_t *sp, void const *ptr) {
	xfa_stack_cls_t *ccur;

	if ((ccur = xfa_ptrstack_clscur(sp)) == XFA_NULL || ccur->count >= ccur->size) {
		if ((ccur = xfa_ptrstack_alloc_cls(sp->sys, XFA_STD_CLUSTER_SIZE)) == XFA_NULL) {
			XFASYS_ERROR(sp->sys, XFAE_ALLOC);
			return -1;
		}
		LL_LIST_ADDT(&ccur->link, &sp->clist);
	}
	ccur->ptrs[ccur->count++] = ptr;

	return 0;
}

void const *xfa_ptrstack_pop(xfa_ptrstack_t *sp) {
	xfa_stack_cls_t *ccur;

	if ((ccur = xfa_ptrstack_clscur(sp)) == XFA_NULL)
		return XFA_NULL;
	while (ccur->count <= 0) {
		if (LL_LIST_PREV(&sp->clist, &ccur->link) == XFA_NULL)
			return XFA_NULL;
		LL_LIST_DEL(&ccur->link);
		xfa_ptrstack_free_cls(sp->sys, ccur);
		if ((ccur = xfa_ptrstack_clscur(sp)) == XFA_NULL)
			return XFA_NULL;
	}

	return ccur->ptrs[--ccur->count];
}

