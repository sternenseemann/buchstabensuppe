#include <stdio.h>

#include "buchstabensuppe.h"

int main(int argc, char **argv) {
  struct bs_context ctx;
  bs_context_init(&ctx);

  for(int i = 1; i < argc; i++) {
    if(bs_add_font(&ctx, argv[i], 0, 16)) {
      printf("Added font %s\n", argv[i]);
    }
  }

  printf("Font count: %ld\n", ctx.bs_fonts_len);

  bs_context_free(&ctx);
  return 0;
}
