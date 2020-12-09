#ifndef BUCHSTABENSUPPE_BITMAP_H
#define BUCHSTABENSUPPE_BITMAP_H

// bitmap api

typedef struct bs_bitmap {
  unsigned char *bs_bitmap;

  // TODO make unsigned
  int   bs_bitmap_height;
  int   bs_bitmap_width;
} bs_bitmap_t;

typedef struct bs_bm_view {
  bs_bitmap_t  bs_view_bitmap;
  unsigned int bs_view_offset_x;
  unsigned int bs_view_offset_y;
  unsigned int bs_view_width;
  unsigned int bs_view_height;
} bs_view_t;

void bs_bitmap_print(bs_bitmap_t);

void bs_bitmap_set(bs_bitmap_t, int, int, unsigned char);

#endif
