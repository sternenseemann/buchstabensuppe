source ./build_config
redo-ifchange ./build_config

OBJS="third_party/stb_truetype.o bitmap.o buchstabensuppe.o"
redo-ifchange $OBJS

tmp_archive="$(mktemp -d)/$1"

$AR rc "$tmp_archive" $OBJS
$RANLIB "$tmp_archive"
mv "$tmp_archive" "$3"
