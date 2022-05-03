#include <stdlib.h>

#include "buchstabensuppe-wrapper.h"

void bsw_utf32_buffer_new(size_t s, bs_utf32_buffer_t *buf) {
  *buf = bs_utf32_buffer_new(s);
}

void bsw_utf32_buffer_free(bs_utf32_buffer_t *buf) {
  if(buf != NULL) {
    bs_utf32_buffer_free(buf);
    free(buf);
  }
}

void bsw_decode_utf8(char *s, size_t l, bs_utf32_buffer_t *buf) {
  *buf = bs_decode_utf8(s, l);
}
