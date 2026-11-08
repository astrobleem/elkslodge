#!/usr/bin/env python3
"""Stage the repository's reviewed prebuilt files for the LAN companion."""
import argparse
import json
import re
import shutil
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent


def prepare(output, catalog_path):
    client = REPO / 'client'
    packages = REPO / 'packages'
    entries = re.findall(r'\{"([^"]+)","[^"]+",(\d+)L,0[0-7]+\}',
                         (client / 'elkslodge.c').read_text())
    if len(entries) != 15:
        raise ValueError('Unexpected client package map; review prepare.py before staging')
    sources = {'neofetch': packages/'neofetch/bin/neofetch',
               'tplay': packages/'tplay/bin/tplay',
               'elksbeat.mid': packages/'tplay/elksbeat.mid',
               'fortune': packages/'fortune/bin/fortune',
               'bofh': packages/'fortune/bin/bofh'}
    for route, size in entries:
        if route.startswith('fortune-package/lib/fortune/'):
            sources[route] = packages/'fortune/lib/fortune'/Path(route).name
        elif route.startswith('elksay-1.1/'):
            sources[route] = packages/'cowsay/bin'/Path(route).name
        source = sources.get(route)
        if source is None or source.stat().st_size != int(size):
            raise ValueError('Missing or size-mismatched payload: ' + route)
    if (client/'elkslodge').stat().st_size != 10304:
        raise ValueError('Expected reviewed 0.4 client (10304 bytes)')
    output.mkdir(parents=True, exist_ok=True)
    catalog = {}
    for route, size in entries:
        target = output/route
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(sources[route], target)
        catalog[route] = {'size': int(size), 'url': None}
    for name in ('get', 'upgrade', 'elkslodge'):
        shutil.copyfile(client/name, output/name)
    shutil.copyfile(client/'elkslodge', output/'elkslodge-0.4')
    for name in ('get', 'upgrade', 'elkslodge', 'elkslodge-0.4'):
        catalog[name] = {'size': (output/name).stat().st_size, 'url': None}
    shutil.copyfile(HERE/'index.html', output/'index.html')
    lines = ['ELKSLODGE packages (native ELKS / 8086)',
             'neofetch, tplay, fortune, bofh, cowsay, cowthink, elksay, elkthink',
             'Install with: elkslodge install PACKAGE',
             'TPLAY physical hardware audio remains experimental.',
             'htop is not included.', '']
    (output/'packages.txt').write_text('\n'.join(lines))
    # Preserve notices; do not imply MIT covers separately licensed packages.
    for component in ('client', 'packages/neofetch', 'packages/cowsay', 'packages/fortune', 'packages/tplay'):
        src = REPO/component/'LICENSE'
        target = output/'licenses'/(component.replace('/', '-') + '.txt')
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, target)
    for src in (packages/'fortune/licenses').glob('*.txt'):
        shutil.copyfile(src, output/'licenses'/('fortune-' + src.name))
    for src in (REPO/'licenses').glob('*.txt'):
        shutil.copyfile(src, output/'licenses'/src.name)
    shutil.copyfile(packages/'tplay/libtandy/LICENSE', output/'licenses/libtandy.txt')
    catalog_path.parent.mkdir(parents=True, exist_ok=True)
    catalog_path.write_text(json.dumps(catalog, indent=2) + '\n')
    return catalog


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, default=HERE/'public')
    p.add_argument('--catalog', type=Path, default=HERE/'catalog.json')
    args = p.parse_args()
    catalog = prepare(args.output, args.catalog)
    print('Staged %d catalog entries; no upstream URLs configured.' % len(catalog))


if __name__ == '__main__':
    main()
