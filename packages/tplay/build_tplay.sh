#!/bin/sh
set -eu
cd "$(dirname "$0")"
export TOPDIR="${ELKS_ROOT:?Set ELKS_ROOT to your built ELKS source checkout}"
export CROSSDIR="${TOOLCHAIN:?Set TOOLCHAIN to your ELKS ia16 toolchain}"
export PATH="$CROSSDIR/bin:$TOPDIR/elks/tools/bin:$PATH"
CFLAGS="-mcmodel=small -melks-libc -mtune=i8086 -Wall -Wextra -Werror -Wno-unused-function -Os -mno-segment-relocation-stuff -fno-inline -fno-builtin-printf -fno-builtin-fprintf -ffunction-sections -fdata-sections"
for f in epsg.c tplay.c libtandy/src/lt_chip.c libtandy/sound/SMF.C libtandy/sound/MIDISYN.C libtandy/sound/INSTR.C; do
 name=$(basename "$f" | tr 'A-Z' 'a-z')
 ia16-elf-gcc $CFLAGS -Wno-misleading-indentation -I"$TOPDIR/include" -I"$TOPDIR/libc/include" -I"$TOPDIR/elks/include" -Ilibtandy/include -Ilibtandy/sound -x c -c "$f" -o "${name%.*}.o"
done
ia16-elf-gcc $CFLAGS -Wl,--gc-sections -maout-heap=36864 -maout-stack=2048 -maout-symtab tplay.o epsg.o lt_chip.o smf.o midisyn.o instr.o -o tplay.sym
ia16-elf-gcc $CFLAGS -Wl,--gc-sections -maout-heap=36864 -maout-stack=2048 tplay.o epsg.o lt_chip.o smf.o midisyn.o instr.o -o tplay
ia16-elf-ar rcs libelkspsg.a epsg.o lt_chip.o midisyn.o instr.o
