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
