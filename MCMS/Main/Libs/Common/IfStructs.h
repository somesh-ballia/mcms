/* net/if.h -- declarations for inquiring about network interfaces
   Copyright (C) 1997,98,99,2000,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */
   
#ifndef _IF_STRUCTS_H
#define _IF_STRUCTS_H	

struct in6_ifreq {
	struct in6_addr ifr6_addr;
	uint32_t ifr6_prefixlen;
	int ifr6_ifindex;
};


#endif /* net/if.h */
