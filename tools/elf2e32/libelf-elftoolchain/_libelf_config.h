/*-
 * Copyright (c) 2008 Joseph Koshy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "elf32.h"
#include "elf64.h"

#define	LIBELF_CONFIG_ADDR	1
#define	LIBELF_CONFIG_BYTE	1
#define	LIBELF_CONFIG_DYN	1
#define	LIBELF_CONFIG_EHDR	1
#define	LIBELF_CONFIG_HALF	1
#define	LIBELF_CONFIG_NOTE	1
#define	LIBELF_CONFIG_OFF	1
#define	LIBELF_CONFIG_PHDR	1
#define	LIBELF_CONFIG_REL	1
#define	LIBELF_CONFIG_RELA	1
#define	LIBELF_CONFIG_SHDR	1
#define	LIBELF_CONFIG_SWORD	1
#define	LIBELF_CONFIG_SYM	1
#define	LIBELF_CONFIG_WORD	1
#define	LIBELF_CONFIG_VDEF	1
#define	LIBELF_CONFIG_VNEED	1
#define	LIBELF_CONFIG_XWORD	1
#define	LIBELF_CONFIG_CAP	1
#define	LIBELF_CONFIG_LWORD	1
#define	LIBELF_CONFIG_MOVE	1
#define	LIBELF_CONFIG_MOVEP	1
#define	LIBELF_CONFIG_SYMINFO	1
#define	LIBELF_CONFIG_GNUHASH	1

#ifdef __FreeBSD__

#if __FreeBSD_version >= 330000
#define	LIBELF_CONFIG_STRL_FUNCTIONS	1
#endif

#endif  /* __FreeBSD__ */


#ifdef __NetBSD__

#define	LIBELF_CONFIG_STRL_FUNCTIONS	1

#endif	/* __NetBSD__ */


#ifndef roundup2
#define	roundup2	roundup
#endif

#define	LIBELF_VCSID(ID)


#ifndef	LIBELF_CONFIG_GNUHASH
#define	LIBELF_CONFIG_GNUHASH	1

/*
 * The header for GNU-style hash sections.
 */

typedef struct {
	u_int32_t	gh_nbuckets;	/* Number of hash buckets. */
	u_int32_t	gh_symndx;	/* First visible symbol in .dynsym. */
	u_int32_t	gh_maskwords;	/* #maskwords used in bloom filter. */
	u_int32_t	gh_shift2;	/* Bloom filter shift count. */
} Elf_GNU_Hash_Header;
#endif
