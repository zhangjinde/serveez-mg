/*
 * sparsevec.h - sparse vector declarations
 *
 * Copyright (C) 2000, 2001 Stefan Jahn <stefan@lkcc.org>
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SPARSEVEC_H__
#define __SPARSEVEC_H__ 1

#include "defines.h"

/* general sparse vector defines */
#define SVZ_SPVEC_BITS 4                     /* values 1 .. 6 possible */
#define SVZ_SPVEC_SIZE (1 << SVZ_SPVEC_BITS) /* values 1 .. 64 possible */
#define SVZ_SPVEC_MASK ((1 << SVZ_SPVEC_SIZE) - 1)

/*
 * On 32 bit architectures SVZ_SPVEC_SIZE is no larger than 32 and on
 * 64 bit architectures it is no larger than 64.  It specifies the number
 * of bits the `spvec->fill' (unsigned long) field can hold.
 */

/* sparse vector chunk structure */
typedef struct svz_spvec_chunk svz_spvec_chunk_t;
struct svz_spvec_chunk
{
  svz_spvec_chunk_t *next;     /* pointer to next sparse vector chunk */
  svz_spvec_chunk_t *prev;     /* pointer to previous sparse vector chunk */
  unsigned long offset;        /* first sparse vector index in this chunk */
  unsigned long fill;          /* usage bit-field */
  unsigned long size;          /* size of this chunk */
  void *value[SVZ_SPVEC_SIZE]; /* value storage */
};

/* top level sparse vector structure */
typedef struct svz_spvec_list svz_spvec_t;
struct svz_spvec_list
{
  unsigned long length;     /* size of the sparse vector (last index +1) */
  unsigned long size;       /* element count */
  svz_spvec_chunk_t *first; /* first sparse vector chunk */
  svz_spvec_chunk_t *last;  /* last sparse vector chunk */
};

__BEGIN_DECLS

/*
 * Exported sparse vector functions.  A sparse vector is a kind of data array
 * which grows and shrinks on demand.  It unifies the advantages of chained
 * lists (less memory usage than simple arrays) and arrays (faster access
 * to specific elements).  This implementation can handle gaps in between
 * the array elements.
 */

SERVEEZ_API svz_spvec_t *svz_spvec_create (void);
SERVEEZ_API void svz_spvec_destroy (svz_spvec_t *);
SERVEEZ_API void svz_spvec_add (svz_spvec_t *, void *);
SERVEEZ_API void svz_spvec_clear (svz_spvec_t *);
SERVEEZ_API unsigned long svz_spvec_contains (svz_spvec_t *,
                                              void *);
SERVEEZ_API void *svz_spvec_get (svz_spvec_t *, unsigned long);
SERVEEZ_API unsigned long svz_spvec_index (svz_spvec_t *, void *);
SERVEEZ_API void *svz_spvec_delete (svz_spvec_t *, unsigned long);
SERVEEZ_API unsigned long svz_spvec_delete_range (svz_spvec_t *,
                                                  unsigned long,
                                                  unsigned long);
SERVEEZ_API void *svz_spvec_set (svz_spvec_t *, unsigned long,
                                 void *);
SERVEEZ_API void *svz_spvec_unset (svz_spvec_t *, unsigned long);
SERVEEZ_API unsigned long svz_spvec_size (svz_spvec_t *);
SERVEEZ_API unsigned long svz_spvec_length (svz_spvec_t *);
SERVEEZ_API void svz_spvec_insert (svz_spvec_t *,
                                   unsigned long, void *);
SERVEEZ_API void **svz_spvec_values (svz_spvec_t *);
SERVEEZ_API void svz_spvec_pack (svz_spvec_t *);

__END_DECLS

#endif /* not __SPARSEVEC_H__ */
