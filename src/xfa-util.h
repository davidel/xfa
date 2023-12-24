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


#if !defined(_XFA_UTIL_H)
#define _XFA_UTIL_H


int xfa_is_prime(int n);
void *xfa_memset(void *d, int c, int n);
void *xfa_memcpy(void *d, const void *s, int n);
int xfa_mapcmp(unsigned long const *m1, unsigned long const *m2, int n);
void xfa_mapcpy(unsigned long *d, unsigned long const *s, int n);
void xfa_or_map(unsigned long *d, unsigned long const *s, int n);
void xfa_and_map(unsigned long *d, unsigned long const *s, int n);
void xfa_not_map(unsigned long *d, int n);
int xfa_map_zero(unsigned long const *s, int n);
void xfa_map_init(unsigned long *d, int n);
void xfa_map_set_range(unsigned long *d, int n, int from, int to);
void xfa_gen_freelist(xfa_system_t * sys, struct ll_list_head *head,
		      void (*fproc) (xfa_system_t *, void *), int foff);
long xfa_atol(char const *str);


#endif
