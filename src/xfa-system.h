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


#if !defined(_XFA_SYSTEM_H)
#define _XFA_SYSTEM_H


typedef struct s_xfa_system {
	void *priv;
	void *(*alloc) (void *, int);
	void (*free) (void *, void *);
	void (*error) (void *, int);
} xfa_system_t;

#define XFASYS_ALLOC(p, s) (*(p)->alloc)((p)->priv, s)
#define XFASYS_FREE(p, d) (*(p)->free)((p)->priv, d)
#define XFASYS_ERROR(p, e) (*(p)->error)((p)->priv, e)


#endif
