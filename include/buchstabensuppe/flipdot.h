/*!
 * @file buchstabensuppe/flipdot.h
 *
 * Facilities for rendering bitmaps onto flipdot displays
 * and useful bitmap manipulation when targeting flipdot displays.
 */
#ifndef BUCHSTABENSUPPE_FLIPDOT_H
#define BUCHSTABENSUPPE_FLIPDOT_H

#include <stdbool.h>     /* bool     */
#include <sys/socket.h>  /* sockaddr */

#include <buchstabensuppe/bitmap.h>

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

#endif
