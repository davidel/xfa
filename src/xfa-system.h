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
