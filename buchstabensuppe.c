#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "third_party/stb/stb_truetype.h"
#include <utf8proc.h>
#include <harfbuzz/hb.h>

#include "buchstabensuppe.h"

void bs_context_init(bs_context_t *ctx) {
  ctx->bs_fonts = NULL;
  ctx->bs_fonts_len = 0;
}

void bs_context_free(bs_context_t *ctx) {
  for(size_t i = 0; i < ctx->bs_fonts_len; i++) {
    free(ctx->bs_fonts[i].bs_font_file);
    hb_font_destroy(ctx->bs_fonts[i].bs_font_hb);
  }

  if(ctx->bs_fonts != NULL) {
    free(ctx->bs_fonts);
  }

  ctx->bs_fonts = NULL;
  ctx->bs_fonts_len = 0;
}

bool bs_add_font(bs_context_t *ctx, char *font_path, int font_index, unsigned int pixel_height) {
  struct stat finfo;
  memset(&finfo, 0, sizeof(struct stat));

  if(stat(font_path, &finfo) != 0) {
    return false;
  }

  size_t file_buffer_size = finfo.st_size;
  unsigned char *file_buffer = malloc(sizeof(unsigned char) * file_buffer_size);

  if(file_buffer == NULL) {
    return false;
  }

  FILE *font_file = fopen(font_path, "rb");

  if(font_file == NULL) {
    free(file_buffer);
    return false;
  }

  size_t read = fread(file_buffer, file_buffer_size,
    sizeof(unsigned char), font_file);

  if(!feof(font_file) && read == file_buffer_size * sizeof(unsigned char)) {
    fclose(font_file);
    free(file_buffer);
  }

  fclose(font_file);

  stbtt_fontinfo stbtt_font;

  if(!stbtt_InitFont(&stbtt_font, file_buffer,
        stbtt_GetFontOffsetForIndex(file_buffer, font_index))) {
    free(file_buffer);
    return false;
  }

  hb_blob_t *file = hb_blob_create((const char *) file_buffer,
    sizeof(unsigned char) * file_buffer_size, HB_MEMORY_MODE_READONLY, NULL, NULL);

  if(hb_blob_get_length(file) == 0) {
    hb_blob_destroy(file);
    free(file_buffer);
    return false;
  }

  hb_face_t *face = hb_face_create(file, font_index);
  hb_blob_destroy(file);

  if(hb_face_get_glyph_count(face) == 0) {
    free(file_buffer);
    return false;
  }

  hb_font_t *font = hb_font_create(face);
  hb_face_destroy(face);

  if(font == NULL) {
    free(file_buffer);
    return false;
  }

  hb_font_set_scale(font, 0, pixel_height);

  size_t new_index = ctx->bs_fonts_len;

  bs_font_t *tmp = realloc(ctx->bs_fonts, sizeof(bs_font_t) * (new_index + 1));

  if(tmp == NULL) {
    hb_font_destroy(font);
    free(file_buffer);
    return false;
  }

  ctx->bs_fonts_len++;

  ctx->bs_fonts = tmp;
  ctx->bs_fonts[new_index].bs_font_hb = font;
  ctx->bs_fonts[new_index].bs_font_file = file_buffer;
  ctx->bs_fonts[new_index].bs_font_file_size = file_buffer_size;
  ctx->bs_fonts[new_index].bs_font_pixel_height = pixel_height;

  memcpy(&(ctx->bs_fonts[new_index].bs_font_stbtt),
    &stbtt_font, sizeof(stbtt_fontinfo));

  return true;
}

void bs_shape_grapheme(bs_context_t *ctx, bs_utf32_buffer_t str, size_t offset, size_t len) {
  if(len == 0) {
    return;
  }

  if(ctx->bs_fonts_len <= 0) {
    return;
  }

  if(str.bs_utf32_buffer_len < offset + len) {
    return;
  }

  hb_buffer_t *buf = hb_buffer_create();

  // Add grapheme to buffer, but use item_offset to give harfbuzz context
  hb_buffer_add_utf32(buf, str.bs_utf32_buffer, str.bs_utf32_buffer_len,
    (unsigned int) offset, (int) len);

  if(hb_buffer_get_length(buf) <= 0) {
    return;
  }

  bool have_glyphs = false;
  size_t font_index = 0;

  while(!have_glyphs && font_index < ctx->bs_fonts_len) {
    hb_shape(ctx->bs_fonts[font_index].bs_font_hb, buf, NULL, 0);

//    if(!hb_buffer_has_positions(buf)) {
//      return;
//    }
 
    have_glyphs = true; // TODO
  }
}

bs_utf32_buffer_t bs_decode_utf8(const char *s, size_t l) {
  bs_utf32_buffer_t buf = bs_utf32_buffer_new(l);

  const utf8proc_uint8_t *utf8_ptr = (utf8proc_uint8_t *) s;
  size_t left = l;

  while(left > 0) {
    utf8proc_int32_t codepoint;
    utf8proc_ssize_t read = utf8proc_iterate(utf8_ptr, left, &codepoint);

    if(read <= 0) {
      errno = EINVAL;
      return buf;
    }

    utf8_ptr += read;
    left -= read;

    if(!bs_utf32_buffer_append_single(codepoint, &buf)) {
      errno = ENOMEM;
      return buf;
    }
  }

  return buf;
}

bs_utf32_buffer_t bs_utf32_buffer_new(size_t cap) {
  bs_utf32_buffer_t buf;
  buf.bs_utf32_buffer_len = 0;

  buf.bs_utf32_buffer = malloc(sizeof(uint32_t) * cap);
  buf.bs_utf32_buffer_cap = buf.bs_utf32_buffer == NULL ? 0 : cap;

  return buf;
}

void bs_utf32_buffer_free(bs_utf32_buffer_t *buf) {
  if(buf->bs_utf32_buffer != NULL) {
    free(buf->bs_utf32_buffer);
  }

  buf->bs_utf32_buffer = NULL;
  buf->bs_utf32_buffer_cap = 0;
  buf->bs_utf32_buffer_len = 0;
}

bool bs_utf32_buffer_append(uint32_t *us, size_t l, bs_utf32_buffer_t *buf) {
  size_t needed_size = buf->bs_utf32_buffer_len + l;

  if(buf->bs_utf32_buffer_cap < needed_size) {
    uint32_t *tmp = realloc(buf->bs_utf32_buffer, sizeof(uint32_t) * needed_size);

    if(tmp == NULL) {
      return false;
    }

    buf->bs_utf32_buffer = tmp;
    buf->bs_utf32_buffer_cap = needed_size;
  }

  memcpy(buf->bs_utf32_buffer + buf->bs_utf32_buffer_len, us, l * sizeof(uint32_t));
  buf->bs_utf32_buffer_len = needed_size;

  return true;
}

bool bs_utf32_buffer_append_single(uint32_t u, bs_utf32_buffer_t *buf) {
  return bs_utf32_buffer_append(&u, 1, buf);
}
