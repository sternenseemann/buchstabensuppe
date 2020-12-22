source ./build_config
redo-ifchange ./build_config

OBJS="bitmap.o buchstabensuppe.o flipdot.o"
redo-ifchange $OBJS

tmp_dir="$(mktemp -d)"
tmp_archive="$tmp_dir/$1"

$AR rc "$tmp_archive" $OBJS
$RANLIB "$tmp_archive"
mv "$tmp_archive" "$3"

rm -r "$tmp_dir"
