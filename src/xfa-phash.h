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
