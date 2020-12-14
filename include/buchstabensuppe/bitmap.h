/*!
 * @file buchstabensuppe/bitmap.h
 */
// TODO: show correct header path in doxygen
#ifndef BUCHSTABENSUPPE_BITMAP_H
#define BUCHSTABENSUPPE_BITMAP_H

#include <stdbool.h>
#include <stdint.h>

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
 * of the bitmap. If `binary_image` is false, the image is
 * considered as a grayscale image and all values greater than
 * 0x80 are rendered as “white” pixels, all below as “black”
 * pixels. Else 1 is “white”, 0 is “black”.
 */
void bs_bitmap_print(bs_bitmap_t bitmap, bool binary_image);

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
 * On error `NULL` is returned and `size` is 0. The allocated
 * memory must be freed by the caller.
 */
uint8_t *bs_view_bitarray(bs_view_t view, size_t *size);

/*!
 * @name Bitmap Processing
 * @{
 */

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

#endif
