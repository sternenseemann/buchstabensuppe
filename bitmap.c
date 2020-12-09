#include <stdio.h>

#include <buchstabensuppe/bitmap.h>

void bs_bitmap_print(bs_bitmap_t bitmap) {
  int h = bitmap.bs_bitmap_height;
  int w = bitmap.bs_bitmap_width;

  for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++) {
      unsigned char pixel = bitmap.bs_bitmap[y * w + x];

      fputs(pixel > 0x80 ? "█" : " ", stdout);
    }
    putchar('\n');
  }
}

void bs_bitmap_set(bs_bitmap_t b, int x, int y, unsigned char p) {
  if(x >= b.bs_bitmap_width || x < 0 ||
      y >= b.bs_bitmap_height || y < 0) {
    return;
  }

  b.bs_bitmap[y * b.bs_bitmap_width + x] = p;
}
