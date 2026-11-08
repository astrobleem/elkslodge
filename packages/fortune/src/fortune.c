/* Small-memory fortune reader for ELKS/8086. MIT licensed, see LICENSE. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define DATADIR "/usr/lib/fortune"
static unsigned long state;
static unsigned long next_random(void) {
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return state;
}
static int read32(FILE *f, unsigned long *v) {
    unsigned char b[4];
    if (fread(b,1,4,f)!=4) return 0;
    *v=(unsigned long)b[0]|((unsigned long)b[1]<<8)|
       ((unsigned long)b[2]<<16)|((unsigned long)b[3]<<24);
    return 1;
}
static void usage(void) {
    puts("fortune [-n number] [fortunes|computers|bofh|file]\n"
         "bofh [-n number]\n"
         "  Print a random fortune or BOFH excuse.\n"
         "  -n selects a specific entry (starting at 1).\n"
         "  FORTUNE_DIR overrides /usr/lib/fortune.\n"
         "  Plain text files use a line containing % between entries.");
}
/* Locate one entry without retaining its text in memory. */
static int scan(FILE *f, unsigned long target, unsigned long *count, long *start) {
    int c, d, e, at_start=1, nonempty=0;
    long pos=0, entry=0;
    *count=0;
    rewind(f);
    while ((c=fgetc(f))!=EOF) {
        if (at_start && c=='%') {
            d=fgetc(f);
            if (d=='\r') e=fgetc(f); else e=d;
            if (e=='\n' || e==EOF) {
                if (nonempty) { ++*count; if (*count==target) *start=entry; }
                entry=ftell(f); pos=entry; nonempty=0; at_start=1; continue;
            }
            if (fseek(f,pos+1,SEEK_SET)) return 0;
        }
        nonempty=1; at_start=(c=='\n'); pos=ftell(f);
    }
    if (nonempty) { ++*count; if (*count==target) *start=entry; }
    return !ferror(f);
}
static int print_entry(FILE *f) {
    int c,d,e, at_start=1, last='\n'; long p;
    while ((c=fgetc(f))!=EOF) {
        if (at_start && c=='%') {
            p=ftell(f); d=fgetc(f); e=(d=='\r')?fgetc(f):d;
            if(e=='\n'||e==EOF) break;
            if(fseek(f,p,SEEK_SET)) return 0;
        }
        putchar(c); last=c; at_start=(c=='\n');
    }
    if(last!='\n') putchar('\n');
    return !ferror(f) && !ferror(stdout);
}
int main(int argc,char **argv) {
    char path[256], idx[260]; const char *name="fortunes", *dir, *base;
    FILE *f, *ix; unsigned long count=0,n=0,off=0; long start=0;
    int i, explicit_n=0, supplied=0, indexed=0;
    base=strrchr(argv[0],'/'); base=base?base+1:argv[0];
    if(!strcmp(base,"bofh")) name="bofh";
    for(i=1;i<argc;i++) {
        if(!strcmp(argv[i],"-h")||!strcmp(argv[i],"--help")) {usage(); return 0;}
        if(!strcmp(argv[i],"-n")) {
            char *end;
            if(++i>=argc || argv[i][0]<'0'||argv[i][0]>'9') {usage(); return 2;}
            n=strtoul(argv[i],&end,10); explicit_n=1;
            if(*end || !n || n>1000000UL) {fputs("fortune: invalid number\n",stderr); return 2;}
        } else if(argv[i][0]=='-' || supplied) {usage(); return 2;}
        else {name=argv[i]; supplied=1;}
    }
    dir=getenv("FORTUNE_DIR"); if(!dir||!*dir) dir=DATADIR;
    if(strchr(name,'/')) {
        if(strlen(name)>=sizeof(path)) return 2;
        strcpy(path,name);
    } else {
        if(strlen(dir)+strlen(name)+2>sizeof(path)) return 2;
        strcpy(path,dir); strcat(path,"/"); strcat(path,name);
    }
    f=fopen(path,"rb"); if(!f) {perror(path);return 1;}
    strcpy(idx,path);strcat(idx,".idx");ix=fopen(idx,"rb");
    if(ix) {
        char magic[4];
        if(fread(magic,1,4,ix)==4&&!memcmp(magic,"FIDX",4)&&read32(ix,&count)&&count&&count<=1000000UL) indexed=1;
    }
    if(!indexed && (!scan(f,0,&count,&start)||!count)) {
        fputs("fortune: empty or unreadable data\n",stderr); if(ix) fclose(ix);fclose(f);return 1;
    }
    if(!explicit_n) {
        state=(unsigned long)time(NULL)^((unsigned long)getpid()<<16);
        if(!state) state=1;
        n=next_random()%count+1;
    }
    if(n>count) {fprintf(stderr,"fortune: entry out of range (1-%lu)\n",count);if(ix)fclose(ix);fclose(f);return 2;}
    if(indexed) {
        if(fseek(ix,8+(long)(n-1)*4,SEEK_SET)||!read32(ix,&off)||off>0x7fffffffUL) {
            fputs("fortune: invalid index\n",stderr);fclose(ix);fclose(f);return 1;
        }
        start=(long)off;
    } else if(!scan(f,n,&count,&start)) {if(ix)fclose(ix);fclose(f);return 1;}
    if(ix)fclose(ix);
    if(fseek(f,start,SEEK_SET)||!print_entry(f)) {fclose(f);return 1;}
    fclose(f);return 0;
}
