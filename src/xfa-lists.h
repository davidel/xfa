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


#if !defined(_XFA_LIST_H)
#define _XFA_LIST_H


struct ll_list_head {
	struct ll_list_head *__next;
	struct ll_list_head *__prev;
};

#define LL_LIST_HEAD_INIT(name) { &(name), &(name) }

#define LL_LIST_HEAD(name)  struct ll_list_head name = LL_LIST_HEAD_INIT(name)

#define LL_INIT_LIST_HEAD(ptr)					\
	do {							\
		(ptr)->__next = (ptr); (ptr)->__prev = (ptr);	\
	} while (0)

#define LL_LIST_ADD(new, prev, next)			\
	do {						\
		struct ll_list_head *__prev = prev;	\
		struct ll_list_head *__next = next;	\
		__next->__prev = new;			\
		(new)->__next = __next;			\
		(new)->__prev = __prev;			\
		__prev->__next = new;			\
	} while (0)

#define LL_LIST_ADDH(new, head) LL_LIST_ADD(new, head, (head)->__next)

#define LL_LIST_ADDT(new, head) LL_LIST_ADD(new, (head)->__prev, head)

#define LL_LIST_UNLINK(prev, next)		\
        do {					\
		(next)->__prev = prev;		\
		(prev)->__next = next;		\
	} while (0)

#define LL_LIST_DEL(entry)						\
	do {								\
		LL_LIST_UNLINK((entry)->__prev, (entry)->__next);	\
		LL_INIT_LIST_HEAD(entry);				\
	} while (0)

#define LL_LIST_EMPTY(head) ((head)->__next == head)
#define LL_NODE_UNLINKED(entry) LL_LIST_EMPTY(entry)

#define LL_LIST_SPLICE(list, head)					\
	do {								\
		struct ll_list_head *first = (list)->__next;		\
		if (first != list) {					\
                        struct ll_list_head *last = (list)->__prev;	\
                        struct ll_list_head *at = (head)->__next;	\
                        (first)->__prev = head;				\
                        (head)->__next = first;				\
                        (last)->__next = at;				\
                        (at)->__prev = last;				\
		}							\
	} while (0)

#define LL_LIST_COUNT(c, pos, head) for (c = 0, pos = (head)->__next; pos != (head); pos = pos->__next, c++)
#define LL_LIST_ENTRY(ptr, type, member)    ((type *)((char *)(ptr) - (long)(&((type *)0)->member)))
#define LL_LIST_FOR_EACH(pos, head) for (pos = (head)->__next; pos != (head); pos = (pos)->__next)
#define LL_LIST_FIRST(head) (((head)->__next != (head)) ? (head)->__next: XFA_NULL)
#define LL_LIST_LAST(head)  (((head)->__prev != (head)) ? (head)->__prev: XFA_NULL)
#define LL_LIST_PREV(head, ptr) (((ptr)->__prev != (head)) ? (ptr)->__prev: XFA_NULL)
#define LL_LIST_NEXT(head, ptr) (((ptr)->__next != (head)) ? (ptr)->__next: XFA_NULL)


#endif
