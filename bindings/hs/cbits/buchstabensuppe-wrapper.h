#include <buchstabensuppe.h>

void bsw_utf32_buffer_new(size_t, bs_utf32_buffer_t *);

void bsw_utf32_buffer_free(bs_utf32_buffer_t *);

void bsw_decode_utf8(char *, size_t, bs_utf32_buffer_t *);
