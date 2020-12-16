#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stb_truetype.h>
#include <utf8proc.h>
#include <harfbuzz/hb.h>

#include <buchstabensuppe.h>

#define FONT_SCALE_MULTIPLIER 1

#define LOG(...) \
  fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fputc('\n', stderr);

// context management

void bs_context_init(bs_context_t *ctx) {
  ctx->bs_fonts = NULL;
  ctx->bs_fonts_len = 0;
  ctx->bs_rendering_flags = 0;
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

bool bs_add_font(bs_context_t *ctx, const char *font_path, int font_index, unsigned int pixel_height) {
  struct stat finfo;
  memset(&finfo, 0, sizeof(struct stat));

  if(stat(font_path, &finfo) != 0) {
    LOG("Error: could not stat %s", font_path);
    return false;
  }

  if(!S_ISREG(finfo.st_mode)) {
    LOG("Error: not a regular file %s", font_path);
    return false;
  }

  size_t file_buffer_size = finfo.st_size;
  unsigned char *file_buffer = malloc(sizeof(unsigned char) * file_buffer_size);

  if(file_buffer == NULL) {
    LOG("Error: Could not allocate memory");
    return false;
  }

  FILE *font_file = fopen(font_path, "rb");

  if(font_file == NULL) {
    LOG("Error: Could not open file %s", font_path);
    free(file_buffer);
    return false;
  }

  size_t read = fread(file_buffer, file_buffer_size,
    sizeof(unsigned char), font_file);

  if(!feof(font_file) && read == file_buffer_size * sizeof(unsigned char)) {
    LOG("Error: did not read font file fully");
    fclose(font_file);
    free(file_buffer);
  }

  fclose(font_file);

  stbtt_fontinfo stbtt_font;

  if(!stbtt_InitFont(&stbtt_font, file_buffer,
        stbtt_GetFontOffsetForIndex(file_buffer, font_index))) {
    LOG("Error: stbtt_InitFont failed");
    free(file_buffer);
    return false;
  }

  hb_blob_t *file = hb_blob_create((const char *) file_buffer,
    sizeof(unsigned char) * file_buffer_size, HB_MEMORY_MODE_READONLY, NULL, NULL);

  if(hb_blob_get_length(file) == 0) {
    LOG("Error: could not create harfbuzz blob");
    hb_blob_destroy(file);
    free(file_buffer);
    return false;
  }

  hb_face_t *face = hb_face_create(file, font_index);
  hb_blob_destroy(file);

  if(hb_face_get_glyph_count(face) == 0) {
    LOG("Error: could not create harfbuzz face");
    free(file_buffer);
    return false;
  }

  hb_font_t *font = hb_font_create(face);
  hb_face_destroy(face);

  if(font == NULL) {
    LOG("Error: could not create harfbuzz font");
    free(file_buffer);
    return false;
  }

  hb_font_set_scale(font, pixel_height * FONT_SCALE_MULTIPLIER,
      pixel_height * FONT_SCALE_MULTIPLIER);

  size_t new_index = ctx->bs_fonts_len;

  bs_font_t *tmp = realloc(ctx->bs_fonts, sizeof(bs_font_t) * (new_index + 1));

  if(tmp == NULL) {
    LOG("Error: couldn't allocate memory");
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

// font rendering

bool bs_cursor_insert(bs_bitmap_t *dst, bs_cursor_t *cursor, int offset_x, int offset_y, int advance_x, int advance_y, bs_bitmap_t src) {
  int required_width = cursor->bs_cursor_x + offset_x + src.bs_bitmap_width;
  int required_height = cursor->bs_cursor_y + offset_y + src.bs_bitmap_height;

  if(required_width > dst->bs_bitmap_width ||
      required_height > dst->bs_bitmap_height) {
    bool success = bs_bitmap_extend(dst, required_width, required_height, 0);

    if(!success) {
      return false;
    }
  }

  bs_bitmap_copy(*dst, cursor->bs_cursor_x + offset_x,
    cursor->bs_cursor_y + offset_y, src);

  cursor->bs_cursor_x += advance_x;
  cursor->bs_cursor_y += advance_y;

  return true;
}

bs_bitmap_t bs_render_utf8_string(bs_context_t *ctx, const char *s, size_t l) {
  bs_bitmap_t b = { NULL, 0, 0 };
  bs_cursor_t cursor = { 0, 0 };

  if(l > 0) {
    bs_utf32_buffer_t buf = bs_decode_utf8(s, l);

    if(errno == 0) {
      (void) bs_render_utf32_string_append(ctx, &b, &cursor, buf);
    }

    bs_utf32_buffer_free(&buf);
  }

  return b;
}

bool bs_render_utf32_string_append(bs_context_t *ctx, bs_bitmap_t *target, bs_cursor_t *cursor, bs_utf32_buffer_t str) {
  utf8proc_int32_t state = 0;
  size_t start_index = 0;
  size_t len = 0;

  for(size_t i = 0; i < str.bs_utf32_buffer_len; i++) {
    len++;

    // end of string or grapheme boundary following
    bool boundary = (i + 1) >= str.bs_utf32_buffer_len ||
        utf8proc_grapheme_break_stateful(str.bs_utf32_buffer[i],
            str.bs_utf32_buffer[i + 1], &state);

    if(boundary) {
      if(!bs_render_grapheme_append(ctx, target, cursor, str, start_index, len)) {
        return false;
      }

      len = 0;
      start_index = i + 1;
    }
  }

  return true;
}

bool bs_render_grapheme_append(bs_context_t *ctx, bs_bitmap_t *target, bs_cursor_t *cursor, bs_utf32_buffer_t str, size_t offset, size_t len) {
  if(len == 0) {
    return false;
  }

  if(ctx->bs_fonts_len <= 0) {
    return false;
  }

  if(str.bs_utf32_buffer_len < offset + len) {
    return false;
  }

  bool have_glyphs = false;
  size_t font_index = 0;

  while(!have_glyphs && font_index < ctx->bs_fonts_len) {
    hb_buffer_t *buf = hb_buffer_create();
    // Add grapheme to buffer, but use item_offset to give harfbuzz context
    hb_buffer_add_utf32(buf, str.bs_utf32_buffer, str.bs_utf32_buffer_len,
      (unsigned int) offset, (int) len);

    hb_buffer_guess_segment_properties(buf);

    if(hb_buffer_get_length(buf) <= 0) {
      hb_buffer_destroy(buf);
      return false;
    }

    hb_shape(ctx->bs_fonts[font_index].bs_font_hb, buf, NULL, 0);

    unsigned int glyph_count = 0;
    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

    // first check all glyphs wether they are in this font
    have_glyphs = true;
    unsigned int missing_glyphs = 0;
    for(unsigned int i = 0; i < glyph_count; i++) {
      if(glyph_info[i].codepoint == 0) {
        have_glyphs = false;
        missing_glyphs++;
        LOG("Missing glyph: id %x, index %u", glyph_info[i].codepoint, i);
      }
    }

    LOG("Missing %u/%u glyphs", missing_glyphs, glyph_count);

    if(have_glyphs) {
      stbtt_fontinfo *font = &ctx->bs_fonts[font_index].bs_font_stbtt;
      float scale_y = stbtt_ScaleForPixelHeight(font,
        ctx->bs_fonts[font_index].bs_font_pixel_height);

      for(unsigned int i = 0; i < glyph_count; i++) {
        int abyss; // tmp var for unused values
        int tt_offset_x, tt_offset_y;

        bs_bitmap_t glyph;
        glyph.bs_bitmap = stbtt_GetGlyphBitmap(font, scale_y, scale_y,
          glyph_info[i].codepoint, &glyph.bs_bitmap_width, &glyph.bs_bitmap_height,
          &tt_offset_x, &tt_offset_y);

        if(glyph.bs_bitmap_width != 0 && glyph.bs_bitmap_height != 0) {
          LOG("Offset: HarfBuzz (%d,%d) TrueType (%d, %d)",
            glyph_pos[i].x_offset, glyph_pos[i].y_offset,
            tt_offset_x, tt_offset_y);
          LOG("Bitmap Size:                    (%d,  %d)", glyph.bs_bitmap_width,
              glyph.bs_bitmap_height);

          /*                     +--- cursor position
           *                     v
           *               +--   +----------------+                   --+
           *               |     |                |                     | p
           *               |     |                |                     | i
           *             | |     |                |                     | x
           * baseline_y1 | |     |                |                     | e
           *             v |     |                |                     | l
           *               |     |                |                     | _
           *               |     |                |                     | h
           *               |     |     +----------+   --+               | e
           *               |     |     |          |     | |             | i
           *               |     |     |          |     | | tt_offset_y | g
           *               |     |     |  glyph   |     | |             | h
           * tt_offset_x --+-----+--+  |          |     | |             | t
           *               |     |  |  |          |     | |             |
           *               |     |  v  |          |     | v             |
           *               +--   +-----+ - -  - - +   --+               |
           * baseline_y0 ^ |     |     |          |                     |
           *             | |     |     |          |                     |
           *               +--   +-----+----------+                  ---+
           *
           * This means the top right corner of the
           * glyph bitmap relative to the cursor position
           * is:
           *
           *    x: cursor_x + bbox_x0
           *    y: cursor_y + pixel_height - baseline_y0 + bbox_y0
           *
           * Also refer to the stb_truetype documentation
           * for this, especially
           *
           *   * stbtt_GetFontBoundingBox
           *   * stbtt_GetCodepointBitmapBox
           *   * stbtt_GetCodepointBitmap
           *   * DETAILED USAGE → Baseline
           *   * DETAILED USAGE → Displaying a character
           *     (note however that in the bitmap bounding
           *     box the coordinate direction is reversed
           *     and thus “above” means “below”)
           */

          int baseline_y0, baseline_y1;

          stbtt_GetFontBoundingBox(font, &abyss, &baseline_y0, &abyss, &baseline_y1);
          baseline_y0 = round(baseline_y0 * scale_y);
          baseline_y1 = round(baseline_y1 * scale_y);

          int baseline = baseline_y1;

          if(-baseline_y0 + baseline_y1 > (int) ctx->bs_fonts[font_index].bs_font_pixel_height) {
            LOG("Warn: font is actually higher than pixel size");
            // possible compromise between top & bottom in this case
            // which makes zero sense:
            // baseline = baseline_y0 + baseline_y1;
          }

          LOG("Baseline y0: %d y1: %d", baseline_y0, baseline_y1);

          int offset_x = glyph_pos[i].x_offset + tt_offset_x;
          int offset_y = glyph_pos[i].y_offset + tt_offset_y + baseline;

          LOG("Computed offset: (%d, %d)", offset_x, offset_y);

          if(ctx->bs_rendering_flags & BS_RENDER_BINARY) {
            bs_bitmap_map(glyph, &bs_pixel_to_binary);
          }

          bool result = bs_cursor_insert(target, cursor, offset_x, offset_y,
            glyph_pos[i].x_advance, glyph_pos[i].y_advance,
            glyph);

          if(!result) {
            bs_bitmap_free(&glyph);
            hb_buffer_destroy(buf);
            return false;
          }
        } else {
          cursor->bs_cursor_x += glyph_pos[i].x_advance;
          cursor->bs_cursor_y += glyph_pos[i].y_advance;
        }

        bs_bitmap_free(&glyph);
      }
    }

    hb_buffer_destroy(buf);

    font_index++;
  }

  return have_glyphs;
}

// buffer implementation

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
