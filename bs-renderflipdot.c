#define _POSIX_C_SOURCE 200112L /* getopt, getaddrinfo, ... */
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <buchstabensuppe.h>

#define DEFAULT_FONT_SIZE 16
#define DEFAULT_FLIPDOT_WIDTH 80
#define DEFAULT_FLIPDOT_HEIGHT 16
#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "2323"

#define SCROLL_DELAY_MICROSECONDS 125000 // 8 FPS
#define PAGE_DELAY_SECONDS 2

enum render_mode {
  RENDER_NORMAL,
  RENDER_PAGE,
  RENDER_SCROLL,
};

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
  fputs(" [-4|-6] [-S|-P] [-h HOST] [-p PORT] [-W WIDTH] [-H HEIGHT] TEXT\n", stderr);

  fputs(name, stderr);
  fputs(" -?\n", stderr);

  fprintf(stderr,
    "\n"
    "  -n    dry run: only print the picture, don't send it\n"
    "  -f    font to use, can be specified multiple times, fallback in given order\n"
    "  -s    font size to use, must be specified before font(s) (default: %d)\n"
    "  -i    invert the bitmap (so text is black on white)\n"
    "  -S    scroll text through the screen\n"
    "  -P    page text if it overflows\n"
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

bool scroll_next_view(bs_view_t *view, int flipdot_width) {
  if(view->bs_view_offset_x >= view->bs_view_bitmap.bs_bitmap_width) {
    view->bs_view_offset_x = -flipdot_width;
    return true;
  } else {
    view->bs_view_offset_x++;
    return false;
  }
}

bool page_next_view(bs_view_t *view, int flipdot_width) {
  if(view->bs_view_offset_x + view->bs_view_width
      >= view->bs_view_bitmap.bs_bitmap_width) {
    return true;
  } else {
    view->bs_view_offset_x += flipdot_width;
  }
  return false;
}

void ignore_signal(int signum) {
  (void) signum;
}

bool render_flipdot(const char *host, const char *port, int family, const char *progname, bs_bitmap_t *bitmap, enum render_mode mode, struct itimerval delay, int flipdot_width, int flipdot_height, bool invert) {
  if(mode == RENDER_NORMAL && (bitmap->bs_bitmap_width < flipdot_width ||
      bitmap->bs_bitmap_height < flipdot_height)) {
    bs_bitmap_extend(bitmap, flipdot_width, flipdot_height, invert);
  } else {
    bs_bitmap_extend(bitmap, bitmap->bs_bitmap_width, flipdot_height, invert);
  }

  bs_view_t view;

  // initial state
  view.bs_view_bitmap = *bitmap;
  view.bs_view_width = flipdot_width;
  view.bs_view_height = flipdot_height;
  view.bs_view_offset_y = 0;

  if(mode == RENDER_SCROLL) {
    view.bs_view_offset_x = -flipdot_width;
  } else {
    view.bs_view_offset_x = 0;
  }

  struct addrinfo *addrs;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = family;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_NUMERICSERV;

  if(getaddrinfo(host, port, &hints, &addrs) != 0) {
    print_error(progname, "could not look up target host");
    return false;
  }

  int sockfd = socket(addrs->ai_family, SOCK_DGRAM, IPPROTO_UDP);

  bool failure = false;

  if(sockfd < 0) {
    print_error(progname, "could not create socket");
  } else {
    bool multiple_frames = mode == RENDER_SCROLL ||
      mode == RENDER_PAGE;
    bool finished = false;

    // render first frame immediately

    size_t bits_size;
    uint8_t *bits = bs_view_bitarray(view, &bits_size, invert);

    if(multiple_frames) {
      // TODO use sigaction
      // TODO handle SIGINT and SIGTERM
      signal(SIGALRM, ignore_signal);

      failure = setitimer(ITIMER_REAL, &delay, NULL) != 0;
    }

    while(!finished && !failure) {
      if(bits == NULL) {
        failure = true;
        break;
      }

      failure = sendto(sockfd, bits, bits_size, 0,
          addrs->ai_addr, addrs->ai_addrlen) != (ssize_t) bits_size;

      if(multiple_frames) {
        // restore handler which is removed by sendto
        signal(SIGALRM, ignore_signal);
      }

      free(bits);

      if(mode == RENDER_SCROLL) {
        finished = scroll_next_view(&view, flipdot_width);
      } else if(mode == RENDER_PAGE) {
        finished = page_next_view(&view, flipdot_width);
      }

      if(!multiple_frames) {
        finished = true;
      } else if(!finished) {
        bits = bs_view_bitarray(view, &bits_size, invert);
        pause();
      }
    }

    const struct itimerval timer_off = { { 0, 0 }, { 0, 0 } };
    setitimer(ITIMER_REAL, &timer_off, NULL);

    close(sockfd);
  }

  freeaddrinfo(addrs);

  return !failure;
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
  enum render_mode mode = RENDER_NORMAL;
  struct itimerval delay;
  memset(&delay, 0, sizeof(struct itimerval));

  int opt;
  int fontcount = 0;

  bs_context_t ctx;
  bs_context_init(&ctx);

  ctx.bs_rendering_flags = BS_RENDER_BINARY;

  bool parse_error = false;

  while(!parse_error && (opt = getopt(argc, argv, "SP46inh:p:s:f:?W:H:")) != -1) {
    switch(opt) {
      case 'S':
        mode = RENDER_SCROLL;
        memset(&delay, 0, sizeof(struct itimerval));
        delay.it_interval.tv_usec = SCROLL_DELAY_MICROSECONDS;
        delay.it_value.tv_usec = SCROLL_DELAY_MICROSECONDS;
        break;
      case 'P':
        mode = RENDER_PAGE;
        memset(&delay, 0, sizeof(struct itimerval));
        delay.it_interval.tv_sec = PAGE_DELAY_SECONDS;
        delay.it_value.tv_sec = PAGE_DELAY_SECONDS;
        break;
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

  bs_bitmap_print(bitmap, true);

  if(!dry_run) {
    printf("Sending image to %s:%s\n", host, port);

    if(!render_flipdot(host, port, ip_family, argv[0], &bitmap, mode, delay, flipdot_width, flipdot_height, invert)) {
      status = 1;
    }
  }

  bs_bitmap_free(&bitmap);
  bs_context_free(&ctx);

  return status;
}
