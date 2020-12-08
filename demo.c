#include <stdio.h>
#include <string.h>

#include "buchstabensuppe.h"

#define RENDER_EMOJI "🧑‍🎄"

int main(int argc, char **argv) {
  struct bs_context ctx;
  bs_context_init(&ctx);

  if(argc < 3) {
    printf("Usage: %s GRAPHEME FONT [FALLBACK FONTS ...]\n", argv[0]);
  }

  for(int i = 2; i < argc; i++) {
    if(bs_add_font(&ctx, argv[i], 0, 16)) {
      printf("Added font %s\n", argv[i]);
    }
  }

  printf("Font count: %ld\n", ctx.bs_fonts_len);

  if(ctx.bs_fonts_len > 0) {
    bs_utf32_buffer_t emoji = bs_decode_utf8(argv[1], strlen(argv[1]));

    bs_shape_grapheme(&ctx, emoji, 0, emoji.bs_utf32_buffer_len);

    bs_utf32_buffer_free(&emoji);
  }

  bs_context_free(&ctx);
  return 0;
}
