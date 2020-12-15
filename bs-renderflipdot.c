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
#define DEFAULT_FLIPDOT_WIDTH 80
#define DEFAULT_FLIPDOT_HEIGHT 16
#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "2323"

void print_error(const char *name, const char *err) {
  fputs(name, stderr);
  fputs(": ", stderr);
  fputs(err, stderr);
  fputc('\n', stderr);
}

void print_usage(const char *name) {
  size_t name_len = strlen(name);

  fputs(name, stderr);
  fputs(" [-s FONTSIZE] -f FONTPATH [-f FONTPATH ...] [-i] [-n]\n", stderr);

  for(size_t i = 0; i < name_len; i++) {
    fputc(' ', stderr);
  }
  fputs(" [-4|-6] [-h HOST] [-p PORT] [-W WIDTH] [-H HEIGHT] TEXT\n", stderr);

  fputs(name, stderr);
  fputs(" -?\n", stderr);

  fprintf(stderr,
    "\n"
    "  -n    dry run: only print the picture, don't send it\n"
    "  -f    font to use, can be specified multiple times, fallback in given order\n"
    "  -s    font size to use, must be specified before font(s) (default: %d)\n"
    "  -i    invert the bitmap (so text is black on white)\n"
    "  -h    hostname of the flipdots to use (default: %s)\n"
    "  -p    port of the flipdots to use (default: %s)\n"
    "  -W    width of the target flipdot display (default: %d)\n"
    "  -H    height of the target flipdot display (default: %d)\n"
    "  -4    only use IPv4 for connecting\n"
    "  -6    only use IPv6 for connecting\n"
    "  -?    display this help screen\n",
    DEFAULT_FONT_SIZE, DEFAULT_HOST, DEFAULT_PORT,
    DEFAULT_FLIPDOT_WIDTH, DEFAULT_FLIPDOT_HEIGHT);
}

bool send_bitarray(const char *host, const char *port, int family, uint8_t *bits, ssize_t bits_size, const char *progname) {
  struct addrinfo *addrs;
  struct addrinfo hints;

  bool result = false;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = family;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_NUMERICSERV;

  if(getaddrinfo(host, port, &hints, &addrs) != 0) {
    print_error(progname, "could not look up target host");
    return false;
  }

  int sockfd = socket(addrs->ai_family, SOCK_DGRAM, IPPROTO_UDP);

  if(sockfd < 0) {
    print_error(progname, "could not create socket");
  } else {
    result = sendto(sockfd, bits, bits_size, 0,
        addrs->ai_addr, addrs->ai_addrlen) == bits_size;

    close(sockfd);
  }

  freeaddrinfo(addrs);

  return result;
}

int main(int argc, char **argv) {
  const char *port = DEFAULT_PORT;
  const char *host = DEFAULT_HOST;
  const char *text;
  int font_size = -1;
  int flipdot_width  = DEFAULT_FLIPDOT_WIDTH;
  int flipdot_height = DEFAULT_FLIPDOT_HEIGHT;
  bool dry_run = false;
  bool invert = false;
  int ip_family = AF_UNSPEC;

  int opt;
  int fontcount = 0;

  bs_context_t ctx;
  bs_context_init(&ctx);

  ctx.bs_rendering_flags = BS_RENDER_BINARY;

  bool parse_error = false;

  while(!parse_error && (opt = getopt(argc, argv, "46inh:p:s:f:?W:H:")) != -1) {
    switch(opt) {
      case '4':
        ip_family = AF_INET;
        break;
      case '6':
        ip_family = AF_INET6;
        break;
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
        port = optarg;
        break;
      case 'W':
        errno = 0;
        flipdot_width = atoi(optarg);
        if(errno != 0 || flipdot_width <= 0) {
          print_error(argv[0], "flipdot width passed is not an integer");
          parse_error = true;
        }
        break;
      case 'H':
        errno = 0;
        flipdot_height = atoi(optarg);
        if(errno != 0 || flipdot_height <= 0) {
          print_error(argv[0], "flipdot height passed is not an integer");
          parse_error = true;
        }
        break;
      case 's':
        errno = 0;
        font_size = atoi(optarg);
        if(errno != 0 || font_size <= 0) {
          print_error(argv[0], "font size passed is not an integer");
          parse_error = true;
        }
        break;
      case 'f':
        if(font_size == -1) {
          font_size = DEFAULT_FONT_SIZE;
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
        bs_context_free(&ctx);
        return 0;
        break;
      default:
        parse_error = true;
        break;
    }
  }

  if(optind >= argc) {
    parse_error = true;
    print_error(argv[0], "missing TEXT argument");
  }

  if(parse_error) {
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

  if(!dry_run && (bitmap.bs_bitmap_width < flipdot_width ||
      bitmap.bs_bitmap_height < flipdot_height)) {
    bs_bitmap_extend(&bitmap, flipdot_width, flipdot_height, invert);
  }

  bs_bitmap_print(bitmap, true);

  if(!dry_run) {
    printf("Sending image to %s:%s\n", host, port);

    bs_view_t view;
    view.bs_view_bitmap = bitmap;
    view.bs_view_offset_x = 0;
    view.bs_view_offset_y = 0;
    view.bs_view_width = flipdot_width;
    view.bs_view_height = flipdot_height;

    size_t size;
    uint8_t *bits = bs_view_bitarray(view, &size);

    if(size > 0) {
      if(!send_bitarray(host, port, ip_family, bits, (ssize_t) size, argv[0])) {
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
