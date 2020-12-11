#ifndef BUCHSTABENSUPPE_H
#define BUCHSTABENSUPPE_H

#include <buchstabensuppe/bitmap.h>
#include <stb_truetype.h>

#include <harfbuzz/hb.h>

#include <stdbool.h>
#include <stddef.h>

// buffers

typedef struct bs_utf32_buffer {
  uint32_t *bs_utf32_buffer;
  size_t    bs_utf32_buffer_len;
  size_t    bs_utf32_buffer_cap;
} bs_utf32_buffer_t;

bs_utf32_buffer_t bs_utf32_buffer_new(size_t);

void bs_utf32_buffer_free(bs_utf32_buffer_t *);

bool bs_utf32_buffer_append(uint32_t *, size_t, bs_utf32_buffer_t *);

bool bs_utf32_buffer_append_single(uint32_t, bs_utf32_buffer_t *);

bs_utf32_buffer_t bs_decode_utf8(const char *, size_t);

// main buchstabensuppe api

typedef struct bs_font {
  hb_font_t      *bs_font_hb;
  stbtt_fontinfo  bs_font_stbtt;
  unsigned char  *bs_font_file;
  size_t          bs_font_file_size;
  unsigned int    bs_font_pixel_height;
} bs_font_t;

typedef struct bs_context {
  bs_font_t  *bs_fonts;
  size_t      bs_fonts_len;

  bool        bs_grayscale;
} bs_context_t;

void bs_context_init(bs_context_t *);

void bs_context_free(bs_context_t *);

bool bs_add_font(bs_context_t *, const char *, int, unsigned int);

typedef struct bs_cursor {
  int bs_cursor_x;
  int bs_cursor_y;
} bs_cursor_t;

bs_bitmap_t bs_render_utf8_string(bs_context_t *, const char *, size_t);

bool bs_render_grapheme_append(bs_context_t *, bs_bitmap_t *, bs_cursor_t *,
  bs_utf32_buffer_t, size_t, size_t);

bool bs_render_utf32_string_append(bs_context_t *, bs_bitmap_t *,
  bs_cursor_t *, bs_utf32_buffer_t);

#endif
