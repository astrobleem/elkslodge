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
assert run('--version').stdout.startswith(b'neofetch-elks 1.1')
assert run('--help').returncode == 0
r=run('--not-supported')
assert r.returncode == 2 and not r.stdout
assert max(map(len, run('--tandy').stdout.splitlines())) <= 80
print('CLI, explicit profile, unknown fields, sanitization and line-width checks passed')

# Profile selection must never pretend to be hardware detection.
r=run('--no-color')
assert b'TANDY' not in r.stdout and b'PCjr' not in r.stdout
assert b'ELKS real-mode PC' in r.stdout
r=run('--pcjr','--no-color')
assert b'IBM PCjr / 8088 (selected)' in r.stdout and b'| IBM PCjr |' in r.stdout
assert b'TANDY' not in r.stdout and b'\x1b' not in r.stdout
r=run('--pcjr','--stdout')
assert b'IBM PCjr / 8088 (selected)' in r.stdout and b'________' not in r.stdout
for args in [('--pcjr','--tandy'),('--tandy','--pcjr')]:
 r=run(*args)
 assert r.returncode == 2 and not r.stdout
assert run('--pcjr','--pcjr').returncode == 0
assert b'--pcjr' in run('--help').stdout
assert max(map(len, run('--pcjr').stdout.splitlines())) <= 80

# A pseudo-terminal exercises color, not physical display hardware.
import pty

def terminal(*args, **env):
 master, slave = pty.openpty()
 settings = dict(os.environ, TERM='ansi')
 settings.pop('NO_COLOR', None)
 settings.update(env)
 proc = subprocess.Popen([p, *args], env=settings, stdout=slave)
 os.close(slave)
 output = b''
 try:
  while True:
   try: data = os.read(master, 4096)
   except OSError: break
   if not data: break
   output += data
 finally: os.close(master)
 assert proc.wait() == 0
 return output
assert b'\x1b[36m' in terminal('--pcjr')
assert b'\x1b' not in terminal('--pcjr', TERM='dumb')
assert b'\x1b' not in terminal('--pcjr', NO_COLOR='')
assert b'\x1b' not in terminal('--pcjr','--no-color')
assert b'\x1b' not in terminal('--pcjr','--stdout')
print('PCjr, generic artwork, conflicting profiles and terminal color checks passed')
