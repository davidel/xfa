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
