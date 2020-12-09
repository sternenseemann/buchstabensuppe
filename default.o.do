redo-ifchange build_config
source ./build_config

redo-ifchange "$2.c"

possible_headers="include/$2.h include/buchstabensuppe/$2.h third_party/stb/$2.h"

for h in $possible_headers; do
  if test -e "$h"; then
    redo-ifchange "$h"
  fi
done

case "$2" in
  buchstabensuppe)
    redo-ifchange third_party/stb/stb_truetype.h
    redo-ifchange include/buchstabensuppe/bitmap.h
    ;;
  *)
esac


$CC $CFLAGS -o "$3" -c "$2.c"
