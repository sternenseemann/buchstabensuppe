/*!
 * @file buchstabensuppe/bitmap.h
 */
// TODO: show correct header path in doxygen
#ifndef BUCHSTABENSUPPE_BITMAP_H
#define BUCHSTABENSUPPE_BITMAP_H

#include <stdbool.h>

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
 * @brief Bitmap subset
 *
 * A view represents a cropped bitmap in
 * a cheap manner similar to a string slice.
 */
typedef struct bs_bm_view {
  bs_bitmap_t  bs_view_bitmap;
  unsigned int bs_view_offset_x;
  unsigned int bs_view_offset_y;
  unsigned int bs_view_width;
  unsigned int bs_view_height;
} bs_view_t;

/*!
 * @brief Create a new bitmap
 *
 * Creates a new bitmap with the given width and height.
 * The allocated bitmap is then initialized to hold the
 * value 0 for every pixel.
 *
 * The caller is responsible for freeing the returned bitmap.
 */
bs_bitmap_t bs_bitmap_new(int width, int height);

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
 * Resizes the bitmap to a new size which must be bigger than
 * the old one on at least one axis. The newly initialized
 * value is then intialized with the given value.
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
 * is out of bounds it returns 0 and sets `errno` to `EINVAL`.
 */
unsigned char bs_bitmap_get(bs_bitmap_t bitmap, int x, int y);

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
 * of the bitmap. All values greater than 0x80 are considered
 * a “white” pixel, all below a “black” pixel.
 */
void bs_bitmap_print(bs_bitmap_t bitmap);

#endif
