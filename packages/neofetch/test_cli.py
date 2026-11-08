import os, subprocess
from pathlib import Path
p = str(Path(__file__).resolve().parent / 'neofetch-host')
def run(*args, **env):
 return subprocess.run([p, *args], env=dict(os.environ, **env), capture_output=True)
r=run('--stdout','--tandy')
assert r.returncode == 0 and b'\x1b' not in r.stdout and b'________' not in r.stdout
assert b'Tandy 1000 EX / 8088 (selected)' in r.stdout
assert b'Memory: unavailable (host rendering test)' in r.stdout
r=run('--no-color')
assert r.returncode == 0 and b'________' in r.stdout and b'\x1b' not in r.stdout
r=run('--stdout', SHELL='\x1b[2Jbad\nvalue', TERM='\x07bad')
assert b'\x1b' not in r.stdout and b'\x07' not in r.stdout and b'?[2Jbad?value' in r.stdout
assert run('--version').stdout.startswith(b'neofetch-elks 1.0')
assert run('--help').returncode == 0
r=run('--not-supported')
assert r.returncode == 2 and not r.stdout
assert max(map(len, run('--tandy').stdout.splitlines())) <= 80
print('CLI, explicit profile, unknown fields, sanitization and line-width checks passed')
