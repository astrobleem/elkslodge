#!/usr/bin/env python3
import subprocess
from pathlib import Path
root=Path(__file__).resolve().parent
exe=str(root/'cowsay-host')
def run(*args, data=None, ok=True):
    p=subprocess.run([exe,*args],input=data,capture_output=True,text=True)
    assert (p.returncode==0)==ok,(args,p.returncode,p.stderr)
    return p.stdout
assert run('hello').splitlines()[:3]==[' _______ ', '< hello >', ' ------- ']
assert run('hello','world')==run(data=' hello \t world\n')
s=run('-W','10','one two three four').splitlines()
assert '/ one two    \\' in s and '\\ three four /' in s,s
assert '< -hi >' in run('--','-hi')
assert '(xx)' in run('-d','x') and 'U  ||' in run('-d','x')
for opt,face in zip('bgpstwy',['==','$$','@@','**','--','OO','..']): assert '('+face+')' in run('-'+opt,'x')
assert '(^^)' in run('-e^^','-T','vv','x')
assert 'vv ||' in run('-Tvv','x')
assert '| b    |' in run('-n',data='a\nb\ncdef\n')
assert '< a       b >' in run('-n',data='a\tb\n')
assert '<  >' in run(data='')
assert run('-fdefault','hi')==run('hi')
for args in [('-W0','x'),('-W256','x'),('-W99999','x'),('-W','no','x'),('-e',),('-f','tux','x'),('-Z','x')]: run(*args,ok=False)
run(data='x'*8192,ok=False)
run('-n',data='x'*256,ok=False)
run('-n',data='\n'*1026,ok=False)
run(data='\x1b',ok=False)
run(data='café',ok=False)
for width in [1,2,10,40,255]:
    output=run('-W',str(width),data='a'*1000)
    assert max(len(x)-4 for x in output.splitlines()[1:-7])<=width
print('Host CLI tests passed')

exe=str(root/'elksay-host')
assert '/(oo)' in run('hello')
assert '^__^' not in run('hello')
assert '^__^' in run('-fdefault','hello')
assert '/(xx)' in run('-d','hello')
assert '----vv--' in run('-Tvv','hello')
assert 'elk' in run('-l')
assert max(map(len,run('hello').splitlines()))<=40
exe=str(root/'elkthink-host')
assert '( hello )' in run('hello')
assert '        o' in run('hello')
exe=str(root/'cowsay-host')
assert '/(oo)' in run('-felk','hello')
print('Elk selection, face, tongue, thought, and screen-width tests passed')
