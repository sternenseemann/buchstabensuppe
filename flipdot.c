#include <errno.h>
#include <stdlib.h>

#include <buchstabensuppe.h>

bool bs_scroll_next_view(bs_view_t *view, int step, enum bs_dimension dim) {
  if(step == 0) {
    return true;
  }

  int bitmap_len, len;
  int *offset;

  switch(dim) {
    case BS_DIMENSION_Y:
      bitmap_len = view->bs_view_bitmap.bs_bitmap_height;
      offset = &(view->bs_view_offset_y);
      len = view->bs_view_width;
      break;
    case BS_DIMENSION_X: /* is also default case */
    default:
      bitmap_len = view->bs_view_bitmap.bs_bitmap_width;
      offset = &(view->bs_view_offset_x);
      len = view->bs_view_height;
      break;
  }

  if(step > 0 && *offset >= bitmap_len) {
    *offset = -len;
    return true;
  } else if(step < 0 && *offset <= -len) {
    *offset = bitmap_len;
    return true;
  } else {
    *offset = *offset + step;
  }

  return false;
}

bool bs_page_next_view(bs_view_t *view, int dir, enum bs_dimension dim) {
  if(dir == 0) {
    return true;
  }

  int bitmap_len, len;
  int *offset;

  switch(dim) {
    case BS_DIMENSION_Y:
      bitmap_len = view->bs_view_bitmap.bs_bitmap_height;
      offset = &(view->bs_view_offset_y);
      len = view->bs_view_width;
      break;
    case BS_DIMENSION_X: /* is also default case */
    default:
      bitmap_len = view->bs_view_bitmap.bs_bitmap_width;
      offset = &(view->bs_view_offset_x);
      len = view->bs_view_height;
      break;
  }

  if(dir > 0 && *offset + len >= bitmap_len) {
    *offset = 0;
    return true;
  } else if(dir < 0 && *offset <= 0) {
    *offset = bitmap_len - len;
    return true;
  } else {
    *offset = *offset + dir * len;
  }

  return false;
}

int bs_flipdot_render(int sockfd, struct sockaddr *addr, socklen_t addrlen, bs_view_t view, unsigned char overflow_color) {
  size_t bits_size;
  uint8_t *bits = bs_view_bitarray(view, &bits_size, overflow_color);

  if(bits == NULL) {
    errno = ENOMEM;
    return -1;
  }

  ssize_t sent = sendto(sockfd, bits, bits_size, 0, addr, addrlen);

  free(bits);

  if(sent != (ssize_t) bits_size) {
    return -1;
  }

  return 0;
}
