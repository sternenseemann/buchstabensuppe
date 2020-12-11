#ifndef BUCHSTABENSUPPE_BITMAP_H
#define BUCHSTABENSUPPE_BITMAP_H

#include <stdbool.h>

// bitmap api

typedef struct bs_bitmap {
  unsigned char *bs_bitmap;

  int            bs_bitmap_height;
  int            bs_bitmap_width;
  // TODO make unsigned
} bs_bitmap_t;

typedef struct bs_bm_view {
  bs_bitmap_t  bs_view_bitmap;
  unsigned int bs_view_offset_x;
  unsigned int bs_view_offset_y;
  unsigned int bs_view_width;
  unsigned int bs_view_height;
} bs_view_t;

bs_bitmap_t bs_bitmap_new(int, int);

bool bs_bitmap_extend(bs_bitmap_t *, int, int, unsigned char);

void bs_bitmap_free(bs_bitmap_t *);

void bs_bitmap_set(bs_bitmap_t, int, int, unsigned char);

unsigned char bs_bitmap_get(bs_bitmap_t, int, int);

void bs_bitmap_copy(bs_bitmap_t, int, int, bs_bitmap_t);

void bs_bitmap_print(bs_bitmap_t);

#endif
