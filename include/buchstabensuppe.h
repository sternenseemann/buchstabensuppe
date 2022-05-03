/*!
 * @file buchstabensuppe.h
 */
#ifndef BUCHSTABENSUPPE_H
#define BUCHSTABENSUPPE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>  /* sockaddr */

/*!
 * @name UTF-32 Buffer API
 * @{
 */
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

//! @}

/*!
 * @name Grayscale Bitmap API
 * @{
 */

/*!
 * @brief 8-bit bitmap
 *
 * bs_bitmap_t wraps a `unsigned char` buffer
 * representing a bitmap of a certain width
 * and height. The pixels are written into
 * the buffer row by row. This means the
 * index of a pixel is computed like this:
 * `y * width + x`.
 */
typedef struct bs_bitmap {
  unsigned char *bs_bitmap;         //!< Dynamically allocated bitmap buffer

  int            bs_bitmap_height;  //!< Height of the bitmap
  int            bs_bitmap_width;   //!< Width of the bitmap
  // TODO make unsigned
} bs_bitmap_t;

/*!
 * @brief Create a new bitmap
 *
 * Creates a new bitmap with the given width and height.
 * The allocated bitmap is then initialized to hold the
 * value `initial` for every pixel.
 *
 * The caller is responsible for freeing the returned bitmap.
 */
bs_bitmap_t bs_bitmap_new(int width, int height, unsigned char inital);

/*!
 * @brief Free a bitmap's buffer
 *
 * Frees the buffer allocated for the given bitmap and
 * updates its dimensions to zero width and height, so
 * future operations won't crash. Note that this does
 * _not_ free the bitmap structure itself as it is not
 * intended to be dynamically allocated.
 */
void bs_bitmap_free(bs_bitmap_t *bitmap);

/*!
 * @brief Increase the size of a bitmap
 *
 * Resizes the bitmap to a new size, but will only increase
 * its size. The newly allocated space is then intialized
 * with the given value.
 */
bool bs_bitmap_extend(bs_bitmap_t *bitmap, int new_width,
  int new_height, unsigned char initial_value);

/*!
 * @brief Set a pixel value
 *
 * Sets the pixel at the specified location to the given value
 * or does nothing if the location is out of bounds.
 */
void bs_bitmap_set(bs_bitmap_t bitmap, int x, int y, unsigned char value);

/*!
 * @brief Get a pixel value
 *
 * Gets the value at the specified location. If the location
 * is out of bounds it returns `def` and sets `errno` to `EINVAL`.
 */
unsigned char bs_bitmap_get(bs_bitmap_t bitmap, int x, int y, unsigned char def);

/*!
 * @brief Copy a bitmap into another one
 *
 * Copies the source bitmap into the destination one with the
 * given offset. Any overflow is cut off, offsets may be
 * negative as well.
 */
void bs_bitmap_copy(bs_bitmap_t destination, int offset_x,
  int offset_y, bs_bitmap_t source);

/*!
 * @brief Print a representation of a bitmap to stdout
 *
 * Uses the unicode block character to render a representation
 * of the bitmap. If `binary_image` is false, the image is
 * considered as a grayscale image and all values greater than
 * 0x80 are rendered as “white” pixels, all below as “black”
 * pixels. Else 1 is “white”, 0 is “black”.
 */
void bs_bitmap_print(bs_bitmap_t bitmap, bool binary_image);

/*!
 * @brief Map a bitmap
 *
 * Calls the provided function on every pixel of the bitmap
 * and sets it to the returned value.
 */
void bs_bitmap_map(bs_bitmap_t bitmap, unsigned char (*fun)(unsigned char));

/*!
 * @brief Invert a binary pixel
 *
 * To be used with bs_bitmap_map().
 */
unsigned char bs_pixel_invert_binary(unsigned char);

/*!
 * @brief Invert a grayscale pixel
 *
 * To be used with bs_bitmap_map().
 */
unsigned char bs_pixel_invert_grayscale(unsigned char);

/*!
 * @brief Convert a grayscale pixel to binary
 *
 * To be used with bs_bitmap_map().
 */
unsigned char bs_pixel_to_binary(unsigned char);

/*!
 * @brief convert a binary pixel to grayscale
 *
 * To be used with bs_bitmap_map().
 */
unsigned char bs_pixel_to_grayscale(unsigned char);

//! @}

/*!
 * @name Bitmap Views
 * @{
 */

/*!
 * @brief Bitmap subset
 *
 * A view represents a cropped bitmap in
 * a cheap manner similar to a string slice.
 */
typedef struct bs_bm_view {
  bs_bitmap_t  bs_view_bitmap;
  int bs_view_offset_x;
  int bs_view_offset_y;
  int bs_view_width;
  int bs_view_height;
} bs_view_t;

/*
 * @brief Compact a binary bitmap
 *
 * This converts a binary bitmap into a more compact
 * representation which every byte stores 8 pixels
 * instead of just 1. The highest bit represents the
 * pixel that comes first in the bitmap.
 *
 * The resulting format is precisely what the
 * [Flidpot UDP protocol](https://wiki.openlab-augsburg.de/Flipdots#per-udp-schnittstelle)
 * requires.
 *
 * `view` describes the bitmap to be used and the area
 * of it. `size` will hold the length of the returned array.
 *
 * If the `view` contains areas which are not covered by its
 * bitmap, these pixels are treated as if they had the value
 * `def`.
 *
 * On error `NULL` is returned and `size` is 0. The allocated
 * memory must be freed by the caller.
 */
uint8_t *bs_view_bitarray(bs_view_t view, size_t *size, unsigned char def);

/*!
 * @brief Axis description
 *
 * Describes which direction to perform a movement in for
 * bs_scroll_next_view() and bs_page_next_view().
 */
enum bs_dimension {
  BS_DIMENSION_X,    //!< X Axis
  BS_DIMENSION_Y,    //!< Y Axis
};

/*!
 * @brief Calculates next view for a one dimensional scroll through a bitmap.
 *
 * Advance a bitmap view by `step` in a given dimension. Will return `true`
 * as soon as the bitmap is out of view and wrap around, i. e. sets the offset
 * to `-len`. This is intended to provide the view for the next frame to be
 * rendered to a (flipdot) display.
 *
 * The given view is expected to be fully intialized and `bs_view_width` and
 * `bs_view_height` to be equal to the dimensions of the target screen. Also
 * the first view position must be set by the caller.
 *
 * @param view bitmap view to alter
 * @param step step to advance by
 * @param dim  dimension to advance in
 *
 * @return true if the bitmap has come out of view,
 *         i. e. the scrolling motion is finished
 */
bool bs_scroll_next_view(bs_view_t *view, int step, enum bs_dimension dim);

/*!
 * @brief Calculates next view for one dimensional paging through a bitmap.
 *
 * This is similar to bs_scroll_next_view(), but the scroll step is the target
 * display's width or height and the picture is never scrolled out of view.
 *
 * If the edge of the bitmap is in view, true is returned and the offset is
 * set so the first page is in view.
 *
 * @param view bitmap view to alter
 * @param step step to advance by
 * @param dim  dimension to advance in
 *
 * @return true if another page would mean the bitmap would come out of view
 *         i. e. all pages have been viewed
 */
bool bs_page_next_view(bs_view_t *view, int direction, enum bs_dimension dim);

/*!
 * @brief Render a bitmap view onto a flipdot display
 *
 * This function renders a view to a flipdot display compatible bitarray
 * using bs_view_bitmap() and sends it to a display listening at the given
 * address using a given socket.
 *
 * The viewed bitmap must be a binary bitmap for this to work properly.
 * Zero is black, all values greater than zero are white.
 *
 * @param sockfd          file descriptor of a `SOCK_DGRAM` socket
 * @param addr            address of the target flipdot display
 * @param addrlen         length of the address structure
 * @param view            bitmap view to render
 * @param overflow_color  value area outside of the picture should get (either 0 or 1)
 */
int bs_flipdot_render(int sockfd, struct sockaddr *addr, socklen_t addrlen,
  bs_view_t view, unsigned char overflow_color);

//! @}

/*!
 * @name Font Rendering
 * @{
 */

typedef struct hb_font_t hb_font_t;
typedef struct SFT_Font SFT_Font;

typedef struct bs_font {
  hb_font_t      *bs_font_hb;
  SFT_Font       *bs_font_schrift;
  unsigned char  *bs_font_file;
  size_t          bs_font_file_size;
  unsigned int    bs_font_pixel_height;
} bs_font_t;

enum bs_rendering_flag {
  BS_RENDER_BINARY      = 0x01,
  BS_RENDER_NO_FALLBACK = 0x04,
};

typedef struct bs_context {
  bs_font_t  *bs_fonts;
  size_t      bs_fonts_len;

  int         bs_rendering_flags;
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

//! @}

#endif
