# Local companion

Python 3 with its standard library is sufficient. No computer address, home
path, service installation, credentials or public upstream is supplied.

From the repository root, stage the reviewed prebuilt payloads:

    python3 server/prepare.py

This copies client 0.5, its bootstrap/upgrade scripts, package binaries, data
and license notices into `server/public/`. It generates `server/catalog.json`
with reviewed client-compatible byte lengths and null upstream URLs. Files
already at those output paths are replaced; use `--output` and `--catalog`
for a separate staging directory.

Choose the actual RFC1918 IPv4 address of your modern Linux computer and
allowed LAN subnet, then run:

    python3 server/server.py --bind YOUR_LAN_IPV4 --lan YOUR_LAN_SUBNET/24

The default port is 8086. Use the actual subnet size rather than assuming /24.
Both address and subnet are mandatory. Open http://YOUR_LAN_IPV4:8086/ on the
same LAN. The static HTML 3.2-style page uses ordinary links, lists and text,
without CSS, JavaScript, frames or TLS requirements on the LAN. It is designed
for MicroWeb, but this portable release has not been visually tested in MicroWeb.
MicroWeb is a browser, not a DOS implementation of the ELKS package client.

Set `ELKSLODGE_SERVER=http://YOUR_LAN_IPV4:8086` on ELKS before using the
bootstrap. See `../client/README.md` for install, upgrade and recovery.

## Optional upstream fetching

An administrator can edit a catalog entry's `url` to a trusted HTTPS artifact
whose exact length matches `size`. Use immutable versioned or commit-pinned
artifacts. No public remote catalog is configured, discovered or synchronized
by this release. The prebuilt packages work from local cache immediately.

Only raw.githubusercontent.com, github.com, objects.githubusercontent.com and
release-assets.githubusercontent.com are allowed upstream hosts. Redirects are
validated, private upstream addresses are rejected, and downloads are bounded
and atomically cached after length checks. Missing files with a configured URL
are fetched on demand. Cached files are never automatically refreshed. Clients
cannot supply arbitrary fetch URLs. Adding another provider requires a reviewed
source change. Length checks detect truncation, not malicious substitution;
there are no cryptographic package signatures in this release.

The ELKS client has a fixed reviewed package map and lengths. Changing a binary
or adding a package requires a coordinated client release. The generated server
catalog is operational configuration, not a dynamic client dependency catalog.

## Network boundary

The service binds only the requested private IPv4 address and accepts only the
configured private subnet. It never creates firewall rules, port forwarding,
UPnP mappings, VPNs or tunnels. Do not publish it through a router or reverse
proxy. Plain HTTP is intended only for a trusted LAN. Router configuration
remains the administrator's responsibility. Directory listings, hidden paths,
parent traversal, symlink paths and query-based proxy requests are rejected.

Run it under your existing service manager if desired. No service or lingering
setting is installed or enabled automatically. New companion code is MIT;
package licenses remain separate and are copied by the staging helper.

Release 0.5 serves neofetch 1.1 at `/neofetch-1.1`. The original `/neofetch`
(1.0) and `/elkslodge-0.4` payloads remain available for older clients.
The source for these compatibility binaries is retained in the repository
[0.4 commit](https://github.com/astrobleem/elkslodge/tree/7cf87883d14fa154c89cfa13580b1f4106bfbdbd).
