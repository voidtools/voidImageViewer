#!/bin/sh
# zig cc cross build: compiles every translation unit listed in files.txt
# then links a 64 bit windows executable. usage: sh build-zig/build.sh
# requires: python3 with the ziglang wheel (pip install ziglang).
set -e
cd "$(dirname "$0")/.."

ZIG="${ZIG:-python3 -m ziglang}"
FLAGS="-target x86_64-windows-gnu -Os -DNDEBUG -DVERSION_X64 -DUNICODE -D_UNICODE -Wno-macro-redefined"
OBJDIR="build-zig/obj"

mkdir -p "$OBJDIR"

# compile in small batches: each zig cc process is a full clang front end.
fail=0
batch=0
while read -r f; do
	[ -n "$f" ] || continue
	# libwebp includes repo relative headers ("src/dec/..."), the app code
	# is self rooted ("viv.h") - give each tree only its own include root so
	# the two string.h headers can never shadow each other.
	case "$f" in
		libwebp/*) inc="-Ilibwebp" ;;
		*) inc="-Ilibwebp" ;;
	esac
	o="$OBJDIR/$(printf '%s' "$f" | tr '/' '_' | sed 's/\.c$/.o/')"
	$ZIG cc $FLAGS $inc -c "$f" -o "$o" 2>> "$OBJDIR/errors.log" || fail=1
	batch=$((batch+1))
	if [ $batch -ge 8 ]; then
		wait 2>/dev/null || true
		batch=0
	fi
done < build-zig/files.txt
wait 2>/dev/null || true

if [ "$fail" -ne 0 ]; then
	echo "COMPILE FAILED - see $OBJDIR/errors.log"
	grep -m 12 "error:" "$OBJDIR/errors.log" || true
	exit 1
fi

$ZIG cc $FLAGS $OBJDIR/*.o -o build-zig/viv.exe -mwindows \
	-lcomctl32 -lcomdlg32 -lshlwapi -lshell32 -lgdi32 -luser32 \
	-ladvapi32 -lole32 -loleaut32 -luuid -lwinmm -ldwmapi -lversion

cp res/voidImageViewer.Manifest build-zig/viv.exe.manifest
ls -l build-zig/viv.exe
echo "BUILD OK"
