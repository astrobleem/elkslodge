# ELKSLODGE client

A small native 8086/8088 client for a self-hosted ELKSLODGE companion. The modern
companion handles upstream HTTPS; ELKS downloads from your trusted LAN over HTTP.
No personal server address is built in.

## Configure and install

On ELKS, set the address of **your** companion, for example:

    ELKSLODGE_SERVER=http://your-server:8086
    export ELKSLODGE_SERVER
    curl -o /tmp/get "$ELKSLODGE_SERVER/get"
    sh /tmp/get

The bootstrap records that address in `/etc/elkslodge`. Alternatively create that
file yourself with one line, `http://HOST:PORT`. `ELKSLODGE_SERVER` overrides the
file. IPv4 addresses and DNS names are supported; HTTPS and URL credentials are
not supported by the client. No server configuration is required for help or
interrupted-install recovery.

    elkslodge list
    elkslodge install neofetch
    elkslodge install tplay
    elkslodge install fortune
    elkslodge install cowsay
    elkslodge install elksay
    elkslodge install all

`fortune` and `bofh` install both commands and all six indexed data files.
`cowsay` and `cowthink` install both commands, including `-f elk` support.
`elksay` and `elkthink` install the two elk commands. `tplay` includes
`/usr/lib/elksbeat.mid`; sound support is experimental.

For an installed client, download `/upgrade` from your companion and run it with
`sh`. The upgrade preserves `/etc/elkslodge`. It stages and checks the full new
client before replacing the old one. Do not interrupt power during activation.
If interrupted there, restore `/bin/.lodgeup/old` to `/bin/elkslodge` before retrying.

## Build

Use an ELKS checkout with its libc built and an ia16-elf toolchain:

    make ELKS_ROOT=/path/to/elks TOOLCHAIN=/path/to/toolchain

The build targets the 8086 small model. `TOOLCHAIN` defaults to `ELKS_ROOT/cross`.
Do not compile and run this installer as a host application: its destinations
are real absolute ELKS paths such as `/bin`.

The `get` and `upgrade` scripts pin the exact released binary length and download
`/elkslodge-0.5`. After rebuilding, update their numeric length checks before
publishing a matching release. Do not replace an immutable published endpoint
with different bytes; use a new release name and update the scripts together.
Release 0.4 and later require explicit server configuration; it has no default server.

## Safety and limitations

Installs require root and ELKS curl in `/bin` or `/usr/bin`. All files download
into `/.lodge` and must match embedded lengths before activation. Existing files
are retained until all downloads complete. Failures roll back; after an
interruption use `elkslodge recover`. Never run recovery alongside an active
installer. Keep power on; ELKS filesystem writes are not power-loss-proof.

Keep enough free space for downloads plus previous versions. Destinations and
staging must share one filesystem. HTTP and length checks provide neither
cryptographic authentication nor protection from malicious substitution. Use a
trusted LAN only. Server-supplied shell installation scripts are not used by the
native package installer.

This is a curated MVP with embedded package definitions. It has no dynamic
catalog, dependency solver, uninstall, or automatic update mechanism. Versioned
elk/cow payload routes avoid breaking earlier clients. A repository change
requires rebuilding the client.

New client source and scripts are MIT licensed. ELKS libc and separately hosted
packages retain their own licenses; this license does not relicense those
components. Review linked-runtime obligations before distributing binaries.

Host checks (syntax, release lengths, and isolated configuration parsing; never installation):

    python3 tests/check_static.py
    python3 tests/check_config.py

Release 0.5 installs neofetch 1.1 from the immutable `/neofetch-1.1` route.
Use `neofetch --pcjr` for the explicit IBM PCjr profile or `--tandy` for the
Tandy profile; these select presentation, not automatic hardware detection.
Earlier clients continue using the original neofetch endpoint and payload.
