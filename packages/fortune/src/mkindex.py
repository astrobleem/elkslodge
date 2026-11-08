#!/usr/bin/env python3
"""Create the little-endian FIDX offset index for an unchanged fortune file."""
import pathlib, struct, sys
for arg in sys.argv[1:]:
    p = pathlib.Path(arg)
    offsets, start, pos, nonempty = [], 0, 0, False
    for line in p.read_bytes().splitlines(keepends=True):
        pos += len(line)
        if line in (b'%\n', b'%\r\n', b'%'):
            if nonempty: offsets.append(start)
            start, nonempty = pos, False
        else:
            nonempty = True
    if nonempty: offsets.append(start)
    p.with_name(p.name + '.idx').write_bytes(b'FIDX' + struct.pack('<I', len(offsets)) + b''.join(struct.pack('<I', x) for x in offsets))
    print(p.name, len(offsets), 'entries')
