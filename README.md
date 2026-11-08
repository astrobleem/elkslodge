# ELKSLODGE

A small lodge for ELKS software: native 8086 programs, their source, and a
self-hosted companion for getting them onto an old PC. MicroWeb visitors are
welcome too: the catalog is ordinary HTML, with no JavaScript requirement.

This repository is a project distribution, not an already-running public
HTTP mirror. GitHub downloads use modern HTTPS; an older browser can browse
the companion over trusted local HTTP. A DOS browser visiting the catalog
does not make the ELKS executables DOS-compatible.

## What is here

- `client/`: ELKSLODGE 0.5, a native curated installer, bootstrap and upgrade.
- `server/`: the modern Python companion and self-host setup instructions.
- `packages/neofetch/`: system summary without invented hardware facts.
- `packages/fortune/`: fortune and BOFH reader, data, and original notices.
- `packages/cowsay/`: cowsay, cowthink, elksay and elkthink in native C.
- `packages/tplay/`: experimental Tandy PSG MIDI playback and original demo.

Each package includes source and prebuilt ELKS executables under `bin/`.
The client executable is `client/elkslodge`. These target ELKS 0.9.2 on
8086/8088 PCs, not DOS, Windows, a modern Linux host, or ELKS protected mode.
No OS disk images, firmware, personal server configuration, or ROMs are here.

## Start here

Read `server/README.md` to set up the companion on a modern computer, then
`client/README.md` for an explicit server address and installation instructions.
Do not run ELF/ELKS binaries on the modern host. For manual installation,
follow the README inside the package you want.

The current client has a fixed, size-pinned package list. It is not a general
package manager and does not read a dynamic public catalog. Keep its payloads
and routes together. HTTP and length checks are not authentication or
cryptographic integrity: use the installer only with a trusted local server.
Never run a bootstrap script downloaded from an untrusted public HTTP mirror.

## Build and checks

Set `ELKS_ROOT` and `TOOLCHAIN` to a built ELKS 0.9.2 source checkout and ia16
cross-toolchain, then follow the package's Makefile or build script.
Cowsay/elksay and neofetch also have `make host` and `python3 test_cli.py`.
The client also includes isolated configuration and release checks in client/tests.
Portable client 0.5 passed ELKS guest bootstrap, upgrade, package installation,
truncation rejection and interrupted-install recovery checks.
These are host-side CLI checks, not ELKS hardware emulation. The package
native rebuilds byte-matched their earlier guest-tested LAN payloads.
The TPLAY mocked-transport check passes; its vendored synth has existing
host sign-comparison warnings. Hardware audio/timing remain experimental.

The optional kernel reservation is separately maintained in
[the ELKS fork's tandy-video-v0.9.2 branch](https://github.com/astrobleem/elks/tree/tandy-video-v0.9.2).
It is experimental source, not a bundled kernel update. Read its limitations
before building it. The lodge does not silently alter the kernel or boot files.

## Licenses and rebuilding linked libraries

The new client, server and TPLAY adapter use MIT. Package license files apply
to their own code, data and artwork, rather than a blanket relicensing:
cowsay/elksay is GPL-3.0-or-later, neofetch retains its MIT notice, fortune data
retains its original attributions and notices, and libtandy retains its own
MIT notices. This product includes software developed by the University of
California, Berkeley and its contributors.

The native programs statically link ELKS libc, whose components include GNU
Library/Lesser GPL code. Those terms are not replaced by MIT. Copies of the
library licenses are in `licenses/`. Complete application source and build
commands are provided here so you can modify and relink the applications.
The exact ELKS 0.9.2 library and build sources used are available in
[the ELKS source tree](https://github.com/astrobleem/elks/tree/e7a56a4334e0ef9a2261d1c2de54db31ba40fdf0)
and its [source archive](https://github.com/astrobleem/elks/archive/e7a56a4334e0ef9a2261d1c2de54db31ba40fdf0.tar.gz).
Build that checkout's libc/toolchain following its BUILD.md, then rebuild a
package with the resulting ELKS_ROOT and TOOLCHAIN. Preserve all applicable
notices when redistributing. Everything is supplied without warranty under
its respective license.

Neofetch 1.1 adds `--pcjr` and neutral default PC artwork. Its native output
was checked in a generic ELKS QEMU guest, including console colors; this is
not a physical PCjr compatibility test. Older client download routes are
preserved. Compatibility binary sources remain in the
[0.4 commit](https://github.com/astrobleem/elkslodge/tree/7cf87883d14fa154c89cfa13580b1f4106bfbdbd).
