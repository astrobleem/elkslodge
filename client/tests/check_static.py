#!/usr/bin/env python3
"""Non-executing host checks; never run the installer on the host."""
from pathlib import Path
import re
import subprocess
root = Path(__file__).resolve().parents[1]
subprocess.run(['cc', '-D_DEFAULT_SOURCE', '-std=c99', '-Wall', '-Wextra',
                '-Werror', '-fsyntax-only', str(root/'elkslodge.c')], check=True)
size = (root/'elkslodge').stat().st_size
for name in ['get', 'upgrade']:
    subprocess.run(['sh', '-n', str(root/name)], check=True)
    text = (root/name).read_text()
    assert re.search(r'test "\$n" -ne '+str(size)+r';', text), name
for name in ['elkslodge.c', 'Makefile', 'get', 'upgrade']:
    text = (root/name).read_text()
    assert '/workspace/' not in text and '192.168.' not in text, name
assert 'SERVER "http' not in (root/'elkslodge.c').read_text()
print('Static syntax, release lengths and portable configuration checks passed.')
