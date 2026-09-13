#!/bin/sh
# zig cc cross build with the self screenshot harness enabled (test builds
# only - the production build must use build.sh). usage: sh build-zig/build-selfshot.sh
set -e
cd "$(dirname "$0")/.."

ZIG="${ZIG:-python3 -m ziglang}"
FLAGS="-target x86_64-windows-gnu -O1 -DNDEBUG -DVERSION_X64 -DUNICODE -D_UNICODE -DVIVP_SELF_SHOT -Wno-macro-redefined"
OBJDIR="build-zig/obj-self"

mkdir -p "$OBJDIR"

fail=0
while read -r f; do
	[ -n "$f" ] || continue
	case "$f" in
		libwebp/*) inc="-Ilibwebp" ;;
		*) inc="-Ilibwebp" ;;
	esac
	o="$OBJDIR/$(printf '%s' "$f" | tr '/' '_' | sed 's/\.c$/.o/')"
	$ZIG cc $FLAGS $inc -c "$f" -o "$o" 2>> "$OBJDIR/errors.log" || fail=1
done < build-zig/files.txt

# the harness itself.
o="$OBJDIR/src_viv_selfshot.o"
$ZIG cc $FLAGS -Ilibwebp -c src/viv_selfshot.c -o "$o" 2>> "$OBJDIR/errors.log" || fail=1

if [ "$fail" -ne 0 ]; then
	echo "COMPILE FAILED - see $OBJDIR/errors.log"
	grep -m 12 "error:" "$OBJDIR/errors.log" || true
	exit 1
fi

$ZIG cc $FLAGS $OBJDIR/*.o -o build-zig/viv_self.exe -mwindows \
	-lcomctl32 -lcomdlg32 -lshlwapi -lshell32 -lgdi32 -luser32 \
	-ladvapi32 -lole32 -loleaut32 -luuid -lwinmm -ldwmapi -lversion

cp res/voidImageViewer.Manifest build-zig/viv_self.exe.manifest
ls -l build-zig/viv_self.exe
echo "BUILD OK"
