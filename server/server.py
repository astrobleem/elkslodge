#!/usr/bin/env python3
"""Private-LAN ELKSLODGE companion. Standard-library Python 3 only."""
import argparse
import ipaddress
import json
import os
import socket
import tempfile
import threading
from pathlib import Path
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import unquote, urlsplit
from urllib.request import HTTPRedirectHandler, ProxyHandler, Request, build_opener

ORIGIN_HOSTS = frozenset(('raw.githubusercontent.com', 'github.com',
    'objects.githubusercontent.com', 'release-assets.githubusercontent.com'))
PRIVATE = tuple(ipaddress.ip_network(n) for n in
    ('10.0.0.0/8', '172.16.0.0/12', '192.168.0.0/16'))
MAX_BYTES = 16 * 1024 * 1024


def safe_route(route):
    return (bool(route) and not route.startswith('/') and '\\' not in route
            and all(p not in ('', '.', '..') and not p.startswith('.')
                    for p in route.split('/')))


def validate_origin(url):
    p = urlsplit(url)
    if (p.scheme != 'https' or p.hostname not in ORIGIN_HOSTS or p.username
            or p.password or p.port not in (None, 443) or p.fragment):
        raise ValueError('Origin must be HTTPS on an approved artifact host')
    addresses = socket.getaddrinfo(p.hostname, 443, type=socket.SOCK_STREAM)
    if not addresses or any(not ipaddress.ip_address(x[4][0]).is_global for x in addresses):
        raise ValueError('Origin must resolve exclusively to public addresses')
    return url


class SafeRedirect(HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        validate_origin(newurl)
        return super().redirect_request(req, fp, code, msg, headers, newurl)


def download(entry, destination):
    size = entry['size']
    if type(size) is not int or not 0 < size <= MAX_BYTES:
        raise ValueError('Invalid artifact length')
    url = validate_origin(entry['url'])
    opener = build_opener(ProxyHandler({}), SafeRedirect())
    destination.parent.mkdir(parents=True, exist_ok=True)
    fd, temp = tempfile.mkstemp(prefix='.fetch-', dir=destination.parent)
    try:
        with os.fdopen(fd, 'wb') as out:
            with opener.open(Request(url, headers={'User-Agent': 'ELKSLODGE/0.4'}), timeout=20) as response:
                validate_origin(response.url)
                length = response.headers.get('Content-Length')
                if length is not None and int(length) != size:
                    raise ValueError('Unexpected artifact length')
                count = 0
                while True:
                    chunk = response.read(min(65536, size + 1 - count))
                    if not chunk:
                        break
                    count += len(chunk)
                    if count > size:
                        raise ValueError('Artifact exceeds expected length')
                    out.write(chunk)
                if count != size:
                    raise ValueError('Truncated artifact')
            out.flush()
            os.fsync(out.fileno())
        os.replace(temp, destination)
    finally:
        if os.path.exists(temp):
            os.unlink(temp)


def load_catalog(path):
    catalog = json.loads(path.read_text()) if path.exists() else {}
    if not isinstance(catalog, dict):
        raise ValueError('Catalog must be an object')
    for route, entry in catalog.items():
        if not safe_route(route) or not isinstance(entry, dict):
            raise ValueError('Invalid catalog route')
        if type(entry.get('size')) is not int or not 0 < entry['size'] <= MAX_BYTES:
            raise ValueError('Invalid catalog size')
        if entry.get('url'):
            p = urlsplit(entry['url'])
            if (p.scheme != 'https' or p.hostname not in ORIGIN_HOSTS or p.username
                    or p.password or p.port not in (None, 443) or p.fragment):
                raise ValueError('Unapproved origin')
    return catalog


def lan_settings(bind, subnet):
    address = ipaddress.ip_address(bind)
    lan = ipaddress.ip_network(subnet)
    if (address.version != 4 or lan.version != 4 or address not in lan
            or not any(lan.subnet_of(n) for n in PRIVATE)):
        raise ValueError('Bind address and subnet must be private RFC1918 IPv4')
    return address, lan


def make_handler(root, catalog):
    root = root.resolve()
    locks = {route: threading.Lock() for route in catalog}

    class Handler(SimpleHTTPRequestHandler):
        protocol_version = 'HTTP/1.0'

        def __init__(self, *args, **kwargs):
            super().__init__(*args, directory=str(root), **kwargs)

        def list_directory(self, path):
            self.send_error(404)
            return None

        def send_head(self):
            parsed = urlsplit(self.path)
            rel = unquote(parsed.path).lstrip('/') or 'index.html'
            target = root / rel
            if (parsed.query or not safe_route(rel)
                    or any(p.is_symlink() for p in [target, *target.parents])):
                self.send_error(403)
                return None
            if not target.exists() and rel in catalog and catalog[rel].get('url'):
                try:
                    with locks[rel]:
                        if not target.exists():
                            download(catalog[rel], target)
                except Exception as exc:
                    self.log_error('Upstream fetch failed: %s', type(exc).__name__)
                    self.send_error(502, 'Configured upstream artifact is unavailable')
                    return None
            return super().send_head()
    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bind', required=True, help='Private LAN IPv4 address of this computer')
    parser.add_argument('--lan', required=True, help='Allowed private IPv4 subnet in CIDR form')
    parser.add_argument('--port', type=int, default=8086)
    parser.add_argument('--root', type=Path, default=Path(__file__).parent / 'public')
    parser.add_argument('--catalog', type=Path, default=Path(__file__).parent / 'catalog.json')
    args = parser.parse_args()
    try:
        address, lan = lan_settings(args.bind, args.lan)
        catalog = load_catalog(args.catalog)
    except (ValueError, OSError) as exc:
        parser.error(str(exc))
    args.root.mkdir(parents=True, exist_ok=True)

    class Server(ThreadingHTTPServer):
        daemon_threads = True
        def verify_request(self, request, client_address):
            request.settimeout(20)
            return ipaddress.ip_address(client_address[0]) in lan
    Server((str(address), args.port), make_handler(args.root, catalog)).serve_forever()


if __name__ == '__main__':
    main()
