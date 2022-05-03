#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <buchstabensuppe.h>

bool bs_bitmap_extend(bs_bitmap_t *b, int new_w, int new_h, unsigned char init) {
  int diff_x = fmax(new_w - b->bs_bitmap_width, 0);
  int diff_y = fmax(new_h - b->bs_bitmap_height, 0);

  new_w = fmax(new_w, b->bs_bitmap_width);
  new_h = fmax(new_h, b->bs_bitmap_height);

  if(diff_y == 0 && diff_x == 0) {
    return true;
  }

  if((b->bs_bitmap_height == 0 && diff_y == 0) ||
      (b->bs_bitmap_width == 0 && diff_x == 0)) {
    b->bs_bitmap_width = new_w;
    b->bs_bitmap_height = new_h;
    return true;
  }

  // perform y only resize if possible because it doesn't require copying
  // since the bitmap consists of rows we can just use realloc

  if(diff_y > 0 && diff_x == 0) {
    unsigned char *tmp = realloc(b->bs_bitmap, new_h * b->bs_bitmap_width);

    if(tmp == NULL) {
      errno = ENOMEM;
      return false;
    }

    b->bs_bitmap = tmp;

    memset(b->bs_bitmap + (b->bs_bitmap_height * b->bs_bitmap_width), init,
      diff_y * b->bs_bitmap_width);

    b->bs_bitmap_height = new_h;

    return true;
  }

  if(diff_y >= 0 && diff_x > 0) {
    unsigned char *tmp = malloc(new_h * new_w);

    if(tmp == NULL) {
      errno = ENOMEM;
      return false;
    }

    memset(tmp, init, new_h * new_w);

    for(int y = 0; y < b->bs_bitmap_height; y++) {
      memcpy(tmp + y * new_w, b->bs_bitmap + y * b->bs_bitmap_width,
        b->bs_bitmap_width);
    }

    free(b->bs_bitmap);

    b->bs_bitmap = tmp;
    b->bs_bitmap_width = new_w;
    b->bs_bitmap_height = new_h;

    return true;
  }

  errno = ENOTSUP;
  return false;
}

bs_bitmap_t bs_bitmap_new(int w, int h, unsigned char init) {
  bs_bitmap_t b = { NULL, 0, 0 };

  if(w > 0 && h > 0) {
    (void) bs_bitmap_extend(&b, w, h, init);
  }

  return b;
}

void bs_bitmap_set(bs_bitmap_t b, int x, int y, unsigned char p) {
  if(x >= b.bs_bitmap_width || x < 0 ||
      y >= b.bs_bitmap_height || y < 0) {
    return;
  }

  b.bs_bitmap[y * b.bs_bitmap_width + x] = p;
}

void bs_bitmap_free(bs_bitmap_t *b) {
  b->bs_bitmap_height = 0;
  b->bs_bitmap_width = 0;

  if(b->bs_bitmap != 0) {
    free(b->bs_bitmap);
  }
}

unsigned char bs_bitmap_get(bs_bitmap_t b, int x, int y, unsigned char def) {
  if(x < 0 || y < 0 ||
      x >= b.bs_bitmap_width || y >= b.bs_bitmap_height) {
    errno = EINVAL;
    return def;
  }

  return b.bs_bitmap[y * b.bs_bitmap_width + x];
}

void bs_bitmap_copy(bs_bitmap_t dst, int offset_x, int offset_y, bs_bitmap_t src) {
  // TODO optimized implementation for when widths are equal
  // and offset_x == 0

  int src_min_y = fmax(0, (-1) * offset_y);
  int src_max_y = fmin(dst.bs_bitmap_height - offset_y, src.bs_bitmap_height);

  int src_min_x = fmax(0, (-1) * offset_x);
  int src_max_x = fmin(dst.bs_bitmap_width - offset_x, src.bs_bitmap_width);

  for(int y = src_min_y; y < src_max_y; y++) {
    int dst_y = y + offset_y;

    unsigned char *dst_ptr = dst.bs_bitmap + (dst.bs_bitmap_width * dst_y)
      + src_min_x + offset_x;

    memcpy(dst_ptr, src.bs_bitmap + (src.bs_bitmap_width * y) + src_min_x,
      src_max_x - src_min_x);
  }
}

void bs_bitmap_print(bs_bitmap_t bitmap, bool binary) {
  int h = bitmap.bs_bitmap_height;
  int w = bitmap.bs_bitmap_width;

  for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++) {
      unsigned char pixel = bitmap.bs_bitmap[y * w + x];

      bool white = binary ? pixel : pixel > 0x80;

      fputs(white ? "█" : " ", stdout);
    }
    putchar('\n');
  }
}

uint8_t *bs_view_bitarray(bs_view_t view, size_t *size, unsigned char def) {
  int view_max_y = view.bs_view_offset_y + view.bs_view_height;
  int view_max_x = view.bs_view_offset_x + view.bs_view_width;

  *size = ceil((view.bs_view_height * view.bs_view_width) / 8);
  uint8_t *array = malloc(sizeof(uint8_t) * (*size));
  size_t array_pos = 0;

  if(array == NULL) {
    *size = 0;
    return NULL;
  }

  for(int y = view.bs_view_offset_y; y < view_max_y; y++) {
    for(int x = view.bs_view_offset_x; x < view_max_x; x = x + 8) {
      uint8_t byte = 0;
      int max_i = fmin(8, view_max_x - x);

      for(int i = 0; i < max_i; i++) {
        // reduce pixel to a single bit works regardless of monospace and
        // grayscale bitmaps -- however grayscale bitmaps are not converted
        // on the fly TODO
        uint8_t pixel_val = bs_bitmap_get(view.bs_view_bitmap, x + i, y, def) > 0;
        byte |= pixel_val << (7 - i);
      }

      array[array_pos++] = byte;

      if(array_pos >= *size) {
        return array;
      }
    }
  }

  return array;
}

unsigned char bs_pixel_invert_binary(unsigned char p) {
  return !p;
}

unsigned char bs_pixel_invert_grayscale(unsigned char p) {
  return (0xff - p);
}

unsigned char bs_pixel_to_binary(unsigned char p) {
  return (p >= 0x80);
}

unsigned char bs_pixel_to_grayscale(unsigned char p) {
  return (p * 0xff);
}

void bs_bitmap_map(bs_bitmap_t bitmap, unsigned char (*fun)(unsigned char)) {
  for(int y = 0; y < bitmap.bs_bitmap_height; y++) {
    for(int x = 0; x < bitmap.bs_bitmap_width; x++) {
      bitmap.bs_bitmap[y * bitmap.bs_bitmap_width + x] =
        (*fun)(bitmap.bs_bitmap[y * bitmap.bs_bitmap_width + x]);
    }
  }
}
