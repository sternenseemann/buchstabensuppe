redo-ifchange build_config
source ./build_config

DEPS="$2.o stb_truetype.o buchstabensuppe.o"
redo-ifchange $DEPS

$CC $CFLAGS -lharfbuzz -lutf8proc -o "$3" $DEPS

# vim: ft=sh
