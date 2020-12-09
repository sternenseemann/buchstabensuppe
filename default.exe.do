redo-ifchange build_config
source ./build_config

DEPS="$2.o libbuchstabensuppe.a"
redo-ifchange $DEPS

$CC $CFLAGS -o "$3" "$2.o" -L. -lharfbuzz -lutf8proc -lbuchstabensuppe

# vim: ft=sh
