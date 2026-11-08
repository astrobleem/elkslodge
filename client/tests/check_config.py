#!/usr/bin/env python3
"""Exercise only config parsing on the host; never call client main/install."""
import os
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
code = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
static FILE *config_fopen(const char *path, const char *mode) {
 if (!strcmp(path,"/etc/elkslodge")) path=getenv("TEST_CONFIG_PATH");
 return fopen(path,mode);
}
#define fopen config_fopen
#define main elkslodge_main
#include "elkslodge.c"
#undef main
#undef fopen
int main(void) {
 FILE *f;
 unsetenv("ELKSLODGE_SERVER");
 assert(loadserver()==-2);
 f=fopen(getenv("TEST_CONFIG_PATH"),"w");assert(f);fclose(f);
 assert(loadserver()==-2);
 setenv("ELKSLODGE_SERVER","http://example.test:8086",1);
 assert(loadserver()==0 && !strcmp(server,"http://example.test:8086"));
 f=fopen(getenv("TEST_CONFIG_PATH"),"w");assert(f);
 fputs("http://configured.test:1234\n",f);fclose(f);
 assert(loadserver()==0 && !strcmp(server,"http://example.test:8086"));
 unsetenv("ELKSLODGE_SERVER");
 assert(loadserver()==0 && !strcmp(server,"http://configured.test:1234"));
 setenv("ELKSLODGE_SERVER","https://example.test",1);assert(loadserver()==-1);
 setenv("ELKSLODGE_SERVER","http://example.test/x",1);assert(loadserver()==-1);
 setenv("ELKSLODGE_SERVER","http://example.test/",1);assert(loadserver()==0);
 puts("Configuration precedence and validation checks passed.");return 0;
}
'''
with tempfile.TemporaryDirectory() as tmp:
    p=Path(tmp); (p/'config.c').write_text(code)
    subprocess.run(['cc','-D_DEFAULT_SOURCE','-std=c99','-Wall','-Wextra','-Werror',
                    '-I',str(root),str(p/'config.c'),'-o',str(p/'check')],check=True)
    subprocess.run([str(p/'check')],check=True,
                   env=dict(os.environ,TEST_CONFIG_PATH=str(p/'config')))
