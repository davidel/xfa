/*
 *  XFA by Davide Libenzi ( Finite Automata library )
 *  Copyright (C) 2000  Davide Libenzi
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
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
