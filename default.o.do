redo-ifchange build_config
source ./build_config

redo-ifchange "$2.c"

if test -e "$2.h"; then
  redo-ifchange "$2.h"
fi

if [[ "$2" = "buchstabensuppe" || "$2" = "stb_truetype" ]]; then
  redo-ifchange third_party/stb/stb_truetype.h
fi

$CC $CFLAGS -o "$3" -c "$2.c"
