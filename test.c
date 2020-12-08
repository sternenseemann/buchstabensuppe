#include "third_party/test.h"
#include "buchstabensuppe.h"

#include <errno.h>

#define FAMILY_EMOJI "👩‍👩‍👧‍👦"

int main(void) {
  bs_utf32_buffer_t family = bs_decode_utf8(FAMILY_EMOJI, sizeof(FAMILY_EMOJI) - 1);

  test_case("No errors in decode", errno == 0);

  bool family_decoded_correctly =
    family.bs_utf32_buffer_len == 7 &&
    family.bs_utf32_buffer[0] == 128105 &&
    family.bs_utf32_buffer[1] == 8205 &&
    family.bs_utf32_buffer[2] == 128105 &&
    family.bs_utf32_buffer[3] == 8205 &&
    family.bs_utf32_buffer[4] == 128103 &&
    family.bs_utf32_buffer[5] == 8205 &&
    family.bs_utf32_buffer[6] == 128102;

  test_case("Family emoji is decoded correctly", family_decoded_correctly);

  bs_utf32_buffer_free(&family);

  test_case("Freed buffer has no capacity", family.bs_utf32_buffer_cap == 0 &&
    family.bs_utf32_buffer_len == 0);

  FILE *random = fopen("/dev/urandom", "rb");

  if(random != NULL) {
    char buf[1024];

    size_t read = fread(buf, 1, sizeof(buf), random);

    if(!ferror(random) && read == sizeof(buf)) {
      errno = 0;
      bs_utf32_buffer_t unlikely = bs_decode_utf8(buf, sizeof(buf));

      test_case("Utf8-decoding random stuff failed", errno == EINVAL);

      bs_utf32_buffer_free(&unlikely);
    }
  }
}
