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


#if !defined(_XFA_MACROS_H)
#define _XFA_MACROS_H

#if defined(CHAR_BIT)
#define XFA_CHARBIT CHAR_BIT
#else
#define XFA_CHARBIT 8
#endif

#define XFA_NULL ((void *) 0)

#define XFA_MIN(a, b) ((a) < (b) ? (a): (b))
#define XFA_MAX(a, b) ((a) > (b) ? (a): (b))
#define XFA_OFFSETOF(t, f) ((long) &((t *) 0)->f)
#define XFA_ISDIGIT(c) ((c) >= '0' && (c) <= '9')
#define XFA_ISODIGIT(c) ((c) >= '0' && (c) <= '8')
#define XFA_LOCHAR(c) ((c) >= 'A' && (c) <= 'Z' ? 'a' + ((c) - 'A'): (c))
#define XFA_ISXDIGIT(c) (XFA_ISDIGIT(c) || (XFA_LOCHAR(c) >= 'a' && XFA_LOCHAR(c) <= 'f'))
#define XFA_XVALUE(c) (XFA_ISDIGIT(c) ? (c) - '0': 10 + XFA_LOCHAR(c) - 'a')
#define XFA_DVALUE(c) ((c) - '0')


#endif
