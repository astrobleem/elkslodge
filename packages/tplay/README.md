# TPLAY for ELKS

A native 8086 MIDI player using the portable libtandy parser, instrument banks
and synthesizer, with an ELKS PSG adapter. This is experimental hardware audio,
not a software audio mixer or a DOS/Windows executable.

Copy `bin/tplay` to `/bin/tplay` and `elksbeat.mid` to
`/usr/lib/elksbeat.mid`, then run `tplay --tandy --demo` or
`tplay --tandy /usr/lib/elksbeat.mid` on a compatible Tandy with its PSG.
Do not force Tandy port I/O on an unrelated PC. Use `tplay --help` for controls.
The included MIDI is an original short demonstration, not a commercial song.

Build against a built ELKS 0.9.2 checkout and ia16 toolchain:

    ELKS_ROOT=/path/to/elks TOOLCHAIN=/path/to/toolchain sh build_tplay.sh

This produces the native binary and the ELKS PSG library. It does not install
anything or modify boot files. The staged build byte-matches the earlier LAN
payload. Host checks do not establish audio quality or real-hardware timing.

The new frontend/adapter use the MIT license in LICENSE. Vendored libtandy
files retain their license notices in libtandy/LICENSE and
libtandy/sound/CORE-MIT.TXT. ELKS libc has separate terms; see the root README.
