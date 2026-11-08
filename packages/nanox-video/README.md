# Experimental Tandy / PCjr Nano-X color source

This is a source patch, not a ready-made package or an installer catalog entry.
The kernel branch alone is not a complete color desktop: this Microwindows
component supplies the renderer, explicit launcher and video utilities.
No firmware, ROMs, OS images or binaries are included here.

The patch applies to [Microwindows commit ac1063ae](https://github.com/ghaerr/microwindows/tree/ac1063aec73d341db04d5f08aa8eb601215a69bc).
It changes eight files only: the ELKS makefile, framebuffer/memory integration,
Tandy/PCjr renderer, and nxcolor/vidinfo/vidreset. It excludes the separate
PicoMEM mouse work, network changes and unrelated application edits. Upstream
Microwindows license terms and notices are retained in LICENSE and the source.
Do not assume ELKSLODGE's MIT license replaces them.

## Build

Use a separate clean Microwindows checkout at the exact commit above and apply
`microwindows-tandy-pcjr.patch` there with `git apply`. Build against the matching
ELKS 0.9.2 headers, libc and ia16 toolchain. The PCjr build needs the separately
built [pcjr-video-v0.9.2 kernel and bootblocks](https://github.com/astrobleem/elks/tree/pcjr-video-v0.9.2).
The original Tandy uses the independent opt-in reservation described in the
[Tandy source branch](https://github.com/astrobleem/elks/tree/tandy-video-v0.9.2).

From the patched Microwindows root:

    export TOPDIR=/path/to/matching-elks-checkout
    export CROSSDIR=/path/to/ia16-toolchain
    export PATH="$CROSSDIR/bin:$TOPDIR/elks/tools/bin:$PATH"
    make -C src -f Makefile.elks SCREEN_TANDY=y
    ia16-elf-gcc -Os -melks -I$TOPDIR/include -I$TOPDIR/elks/include -Isrc/include -DELKS=1 -DMWCONFIG_FILE='"mwconfig.elks"' -o nxcolor nxcolor.c src/drivers/vgaplan4_asm.o
    ia16-elf-gcc -Os -melks -I$TOPDIR/include -I$TOPDIR/elks/include -o vidinfo vidinfo.c
    ia16-elf-gcc -Os -melks -I$TOPDIR/include -I$TOPDIR/elks/include -o vidreset vidreset.c src/drivers/vgaplan4_asm.o

Use an isolated ELKS filesystem copy for evaluation. The runtime names are
`/bin/nano-color` for `src/bin/nano-x`, `/bin/nxcolor`, `/bin/vidinfo` and
`/bin/vidreset`, plus `/bin/nxstart` and the desired apps from the matching
Microwindows build. Keep the normal monochrome server separate. The patch
requires the kernel video-reservation ioctl to succeed before mapping video RAM.

The clean source retains upstream serial-mouse support, not a PicoMEM mouse
driver. Without a supported mouse, use `export MOUSE_PORT=none`, then `nxcolor`.
Ctrl+A exits graphics. The launcher waits for its children to stop before
restoring text mode and the terminal. Do not run competing graphics servers. The tested ELKS build uses compiled
fonts (winFreeSansSerif11x13 and X6x13), not a separately supplied ROM font.

## Scope and limits

Expanded 640 KB PCjr emulation is the target, not stock 128 KB PCjr hardware.
The final kernel and relocated MINIX bootloader must be used together; replacing
only the kernel in a default image is insufficient. See the kernel branch's
Documentation/pcjr-video.txt for the build and memory constraints.

PCjr boot, BIOS keyboard, colored launcher/Tetris and text-mode return were
observed in DOSBox-X 2025.02.01. The clean video-only build also repeated
colored Tetris, Ctrl+A text restoration and shell typing with MOUSE_PORT=none,
without the unrelated PicoMEM mouse changes. Physical PCjr hardware and physical mouse use
have not been validated. Tetris is taller than the 200-line display and clips
at the bottom. Both 128 KB and 512 KB boot configurations visibly refuse
with the 640K requirement. The default Tandy kernel still
builds with its original bootloader behavior. Host raster checks used
ASan/UBSan; those checks do not establish physical hardware timing.
