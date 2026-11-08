/* cowsay-elks 1.1: compact C adaptation for ELKS / Intel 8086 and 8088.
 * Balloon/face conventions and default cow from cowsay 3.03,
 * (c) 1999-2000 Tony Monroe, https://github.com/tnalpgge/rank-amateur-cowsay
 * New C implementation, November 2026. Distributed under GPL-3.0-or-later.
 * No warranty. See LICENSE. Not a Perl cowfile interpreter.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CAP 8192
#define LINES 1024
static char text[CAP];
static unsigned short starts[LINES], lens[LINES];
static int used, count, widest;
static const char *program="cowsay";
static void fail(const char *s) { fprintf(stderr,"%s: %s\n",program,s); exit(1); }
static void add(int c) {
    if (used >= CAP-1) fail("message exceeds 8191 bytes");
    text[used++] = c;
}
static void line(int start, int len) {
    if (count == LINES) fail("message exceeds 1024 lines");
    if (len > 255) fail("line exceeds 255 columns; omit -n to wrap");
    starts[count]=start; lens[count++]=len;
    if (len>widest) widest=len;
}
static void repeat(int ch,int n) { while(n-->0) putchar(ch); }
static int space(int c) { return c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\v' || c=='\f'; }
static void usage(void) {
    puts("cowsay-elks 1.1 (native C, GPL-3.0-or-later)\n"
         "Usage: cowsay [-bdgpstwy] [-n] [-W width] [-e eyes] [-T tongue]\n"
         "              [-f default|elk] [--] [message ...]\n"
         "Without a message, reads stdin. -n preserves lines and expands tabs.\n"
         "-l lists cows; -h shows help. cowthink uses a thought bubble.\n"
         "Width 1..255 (default 40); ASCII input, at most 8191 bytes/1024 lines.\n"
         "elksay/elkthink default to the elk; -f selects either animal.\n"
         "Built-in animals only; no Perl cowfiles or COWPATH support.");
}
int main(int argc,char **argv) {
    int i,j,c,width=40,nowrap=0,think=0,flags=0,elk=0,start,end,pos,n,out,pending;
    char eyes[3]="oo", tongue[3]="  ", left,right,thought;
    const char *p,*val,*modes="bdgpstwy";
    const char *faces[]={"==","xx","$$","@@","**","--","OO",".."};
    p=strrchr(argv[0],'/'); program=p?p+1:argv[0];
    if (strstr(program,"think")) think=1;
    if (strstr(program,"elk")) elk=1;
    for(i=1;i<argc;i++) {
        p=argv[i];
        if(!strcmp(p,"--")) { i++; break; }
        if(p[0]!='-' || !p[1]) break;
        if(!strcmp(p,"--help")) { usage(); return 0; }
        if(!strcmp(p,"--version")) { puts("cowsay-elks 1.1"); return 0; }
        for(j=1;p[j];j++) {
            c=p[j];
            if(c=='h') { usage(); return 0; }
            if(c=='l') { puts("Built-in animals: default elk"); return 0; }
            if(c=='n') { nowrap=1; continue; }
            val=strchr(modes,c);
            if(val) { flags |= 1 << (val-modes); continue; }
            if(c!='W' && c!='e' && c!='T' && c!='f') fail("unknown option (try -h)");
            if(p[j+1]) val=p+j+1;
            else { if(++i==argc) fail("option requires an argument"); val=argv[i]; }
            if(c=='W') {
                n=0; if(!*val) fail("invalid width");
                while(*val) { if(*val<'0'||*val>'9') fail("invalid width"); n=n*10+*val++-'0'; if(n>255) fail("width must be 1..255"); }
                if(n<1) fail("width must be 1..255");
                width=n;
            } else if(c=='f') {
                if(!strcmp(val,"default") || !strcmp(val,"default.cow")) elk=0;
                else if(!strcmp(val,"elk") || !strcmp(val,"elk.cow")) elk=1;
                else fail("supported built-in animals: default, elk");
            } else {
                char *dst=c=='e'?eyes:tongue;
                dst[0]=val[0]?val[0]:' '; dst[1]=val[0]&&val[1]?val[1]:' '; dst[2]=0;
                if(dst[0]<' '||dst[0]>'~'||dst[1]<' '||dst[1]>'~') fail("face must be printable ASCII");
            }
            break;
        }
    }
    for(j=0;j<8;j++) if(flags & (1<<j)) { strcpy(eyes,faces[j]); if(j==1||j==4) strcpy(tongue,"U "); }
    if(i<argc) { for(;i<argc;i++) { if(used) add(' '); for(p=argv[i];*p;p++) add((unsigned char)*p); } }
    else { while((c=getchar())!=EOF) add(c); if(ferror(stdin)) fail("stdin read failed"); }
    for(i=0;i<used;i++) if((unsigned char)text[i]>126 || (text[i]<32&&!space(text[i]))) fail("input must be ASCII text");
    if(nowrap) {
        /* Expand tabs into a separate bounded buffer, then copy back. */
        static char expanded[CAP];
        out=0; n=0;
        for(i=0;i<used;i++) {
            c=text[i]; if(c=='\r') continue;
            if(c=='\v'||c=='\f') c=' ';
            j=c=='\t'?8-(n%8):1;
            while(j-->0) { if(out>=CAP-1) fail("expanded message exceeds 8191 bytes"); expanded[out++]=c=='\t'?' ':c; n++; }
            if(c=='\n') n=0;
        }
        memcpy(text,expanded,out); used=out;
        if(used && text[used-1]=='\n') used--;
        start=0;
        for(i=0;i<used;i++) if(text[i]=='\n') { line(start,i-start); start=i+1; }
        line(start,used-start);
    } else {
        out=0; pending=0;
        for(i=0;i<used;i++) {
            if(space(text[i])) { if(out) pending=1; }
            else { if(pending) text[out++]=' '; pending=0; text[out++]=text[i]; }
        }
        used=out; pos=0;
        while(pos<used) {
            end=pos+width; if(end>=used) end=used;
            else if(text[end]!=' ') { for(j=end;j>pos&&text[j]!=' ';j--) ; if(j>pos) end=j; }
            line(pos,end-pos); pos=end;
            while(pos<used && text[pos]==' ') pos++;
        }
        if(!count) line(0,0);
    }
    putchar(' '); repeat('_',widest+2); puts(" ");
    for(i=0;i<count;i++) {
        if(think) { left='('; right=')'; }
        else if(count==1) { left='<'; right='>'; }
        else if(i==0) { left='/'; right='\\'; }
        else if(i==count-1) { left='\\'; right='/'; }
        else { left='|'; right='|'; }
        putchar(left); putchar(' ');
        for(j=0;j<(int)lens[i];j++) putchar(text[starts[i]+j]);
        repeat(' ',widest-lens[i]); putchar(' '); putchar(right); putchar('\n');
    }
    putchar(' '); repeat('-',widest+2); puts(" ");
    thought=think?'o':'\\';
    if(elk) {
        printf("        %c       \\ | /     \\ | /\n",thought);
        printf("         %c     --\\|/       \\|/--\n",thought);
        puts("                 \\         /");
        puts("                  \\_______/");
        printf("                  /(%s)     \\__\n",eyes);
        puts("                 /              >");
        printf("                /     /----%s--'\n",tongue);
        puts("       ________/     /");
        puts("      /             /");
        puts("     /              | ");
        puts("    /|    _______   |");
        puts("   / |   |       |  |");
        puts("     |   |       |  |");
        puts("     |___|       |__|");
    } else {
    printf("        %c   ^__^\n         %c  (%s)\\_______\n",thought,thought,eyes);
    puts("            (__)\\       )\\/\\");
    printf("             %s ||----w |\n",tongue);
    puts("                ||     ||");
    }
    return ferror(stdout)?1:0;
}
