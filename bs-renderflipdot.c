#define _POSIX_C_SOURCE 200112L /* getopt, getaddrinfo, ... */
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <buchstabensuppe.h>

#define DEFAULT_FONT_SIZE 16

#define FLIPDOT_WIDTH 80
#define FLIPDOT_HEIGHT 16

void print_error(const char *name, const char *err) {
  fputs(name, stderr);
  fputs(": ", stderr);
  fputs(err, stderr);
  fputc('\n', stderr);
}

void print_usage(const char *name) {
  print_error(name, "[-h HOST] [-p PORT] [-s FONTSIZE] [-f FONTPATH [-f FONTPATH ...]] [-i] [-n] TEXT");
  print_error(name, "-?");
}

bool send_bitarray(const char *host, const char *port, uint8_t *bits, ssize_t bits_size, const char *progname) {
  struct addrinfo *addrs;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));

  // TODO: seems like flipdot impls are v4 only
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_NUMERICSERV;

  if(getaddrinfo(host, port, &hints, &addrs) != 0) {
    print_error(progname, "could not look up target host");
    return false;
  }

  if(addrs == NULL) {
    return false;
  }

  int sockfd = socket(addrs->ai_family, SOCK_DGRAM, IPPROTO_UDP);

  if(sockfd < 0) {
    print_error(progname, "could not create socket");
    freeaddrinfo(addrs);
    return false;
  }

  bool result = sendto(sockfd, bits, bits_size, 0,
    addrs->ai_addr, addrs->ai_addrlen) == bits_size;

  close(sockfd);

  freeaddrinfo(addrs);

  return result;
}

int main(int argc, char **argv) {
  const char *port = "2323";
  const char *host = "localhost";
  const char *text;
  int font_size = DEFAULT_FONT_SIZE;

  int opt;
  int fontcount = 0;

  bool dry_run = false;
  bool invert = false;

  bs_context_t ctx;
  bs_context_init(&ctx);

  ctx.bs_rendering_flags = BS_RENDER_BINARY;

  bool opterr = false;

  while(!opterr && (opt = getopt(argc, argv, "inh:p:s:f:?")) != -1) {
    switch(opt) {
      case 'i':
        invert = true;
        break;
      case 'n':
        dry_run = true;
        break;
      case 'h':
        host = optarg;
        break;
      case 'p':
        errno = 0;
        port = optarg;
        break;
      case 's':
        errno = 0;
        font_size = atoi(optarg);
        if(errno != 0 || font_size <= 0) {
          print_error(argv[0], "font size passed is not an integer");
          opterr = true;
        }
        break;
      case 'f':
        if(font_size == DEFAULT_FONT_SIZE) {
          print_error(argv[0], "warning: no font size specified, using default");
        }

        if(bs_add_font(&ctx, optarg, 0, font_size)) {
          fontcount++;
        } else {
          print_error(argv[0], "warning: could not add font");
        }

        break;
      case '?':
        print_usage(argv[0]);
        fputc('\n', stderr);
        fputs("  -h    hostname of the flipdots to use (default: localhost)\n", stderr);
        fputs("  -p    port of the flipdots to use (default: 2323)\n", stderr);
        fputs("  -f    font to use, can be specified multiple times, fallback in given order\n", stderr);
        fputs("  -s    font size to use, must be specified before font(s) (default: 16)\n", stderr);
        fputs("  -i    invert the bitmap (so text is black on white)\n", stderr);
        fputs("  -n    dry run: only print the picture, don't send it\n", stderr);
        fputs("  -?    display this help screen\n", stderr);
        bs_context_free(&ctx);
        return 0;
        break;
      default:
        opterr = true;
        break;
    }
  }

  if(optind >= argc) {
    opterr = true;
    print_error(argv[0], "missing TEXT argument");
  }

  if(opterr) {
    bs_context_free(&ctx);
    print_usage(argv[0]);
    return 1;
  }

  if(fontcount <= 0) {
    bs_context_free(&ctx);
    print_error(argv[0], "no valid fonts specified");
    return 1;
  }

  int status = 0;

  text = argv[optind];
  size_t text_len = strlen(text);

  bs_bitmap_t bitmap = bs_render_utf8_string(&ctx, text, text_len);

  if(!dry_run) {
    puts("Rendered image:");
  }

  if(invert) {
    bs_bitmap_map(bitmap, bs_pixel_invert_binary);
  }

  if(!dry_run && (bitmap.bs_bitmap_width < FLIPDOT_WIDTH ||
      bitmap.bs_bitmap_height < FLIPDOT_HEIGHT)) {
    bs_bitmap_extend(&bitmap, FLIPDOT_WIDTH, FLIPDOT_HEIGHT, invert);
  }

  bs_bitmap_print(bitmap, true);

  if(!dry_run) {
    printf("Sending image to %s:%s\n", host, port);

    bs_view_t view;
    view.bs_view_bitmap = bitmap;
    view.bs_view_offset_x = 0;
    view.bs_view_offset_y = 0;
    view.bs_view_width = FLIPDOT_WIDTH;
    view.bs_view_height = FLIPDOT_HEIGHT;

    size_t size;
    uint8_t *bits = bs_view_bitarray(view, &size);

    if(size > 0) {
      if(!send_bitarray(host, port, bits, (ssize_t) size, argv[0])) {
        status = 1;
      }
    } else {
      print_error(argv[0], "bs_view_bitarray failed");
      status = 1;
    }

    if(bits != NULL) {
      free(bits);
    }
  }

  bs_bitmap_free(&bitmap);
  bs_context_free(&ctx);

  return status;
}
