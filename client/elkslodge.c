/* ELKSLODGE 0.5: curated, recoverable installs for ELKS/8086. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define WORK "/.lodge"
struct item { const char *url, *dest; long size; int mode; };
static struct item files[] = {
 {"neofetch-1.1","/bin/neofetch",5840L,0755},
 {"tplay","/bin/tplay",18848L,0755},
 {"elksbeat.mid","/usr/lib/elksbeat.mid",954L,0644},
 {"fortune","/bin/fortune",7936L,0755},
 {"bofh","/bin/bofh",7936L,0755},
 {"fortune-package/lib/fortune/fortunes","/usr/lib/fortune/fortunes",24513L,0644},
 {"fortune-package/lib/fortune/fortunes.idx","/usr/lib/fortune/fortunes.idx",1732L,0644},
 {"fortune-package/lib/fortune/computers","/usr/lib/fortune/computers",245021L,0644},
 {"fortune-package/lib/fortune/computers.idx","/usr/lib/fortune/computers.idx",4272L,0644},
 {"fortune-package/lib/fortune/bofh","/usr/lib/fortune/bofh",29515L,0644},
 {"fortune-package/lib/fortune/bofh.idx","/usr/lib/fortune/bofh.idx",1820L,0644},
 {"elksay-1.1/cowsay","/bin/cowsay",6752L,0755},
 {"elksay-1.1/cowthink","/bin/cowthink",6752L,0755},
 {"elksay-1.1/elksay","/bin/elksay",6752L,0755},
 {"elksay-1.1/elkthink","/bin/elkthink",6752L,0755}
};
static int first,last;
static char server[104];
static int loadserver(void) {
 FILE *f; const char *env; unsigned int i;
 server[0]=0;
 env=getenv("ELKSLODGE_SERVER");
 if(env){if(strlen(env)>=sizeof(server))return -1;strcpy(server,env);}
 else {
  f=fopen("/etc/elkslodge","r");
  if(f){if(!fgets(server,sizeof(server),f))server[0]=0;fclose(f);server[strcspn(server,"\r\n")]=0;}
 }
 if(!server[0])return -2;
 i=strlen(server);if(i>7&&server[i-1]=='/')server[--i]=0;
 if(strncmp(server,"http://",7)||i<=7||i>100)return -1;
 for(i=7;server[i];i++)if(!((server[i]>='a'&&server[i]<='z')||(server[i]>='A'&&server[i]<='Z')||(server[i]>='0'&&server[i]<='9')||server[i]=='.'||server[i]=='-'||server[i]==':'))return -1;
 return 0;
}
static char tmp[40], old[40], absent[40];
static void paths(int i) { sprintf(tmp,WORK "/n%d",i); sprintf(old,WORK "/o%d",i); sprintf(absent,WORK "/a%d",i); }
static int exists(const char *s) { struct stat st; return lstat(s,&st)==0; }
static int marker(const char *s) { int fd=open(s,O_WRONLY|O_CREAT|O_EXCL,0600); if(fd<0)return -1; return close(fd); }
static int selectpkg(const char *s) {
 if(!strcmp(s,"neofetch")){first=0;last=1;}
 else if(!strcmp(s,"tplay")){first=1;last=3;}
 else if(!strcmp(s,"fortune")||!strcmp(s,"bofh")){first=3;last=11;}
 else if(!strcmp(s,"cowsay")||!strcmp(s,"cowthink")){first=11;last=13;}
 else if(!strcmp(s,"elksay")||!strcmp(s,"elkthink")){first=13;last=15;}
 else if(!strcmp(s,"all")){first=0;last=15;}
 else return -1;
 return 0;
}
static int directory(const char *s) { struct stat st; if(lstat(s,&st)<0) {if(errno!=ENOENT||mkdir(s,0755)<0)return -1;return 0;} return S_ISDIR(st.st_mode)?0:-1; }
static int fetch(const char *endpoint,const char *out) {
 char url[160]; int pid,status,waited;
 sprintf(url,"%s/%s",server,endpoint);
 pid=fork(); if(pid<0)return -1;
 if(!pid){execl("/bin/curl","curl","-o",out,url,(char *)0);execl("/usr/bin/curl","curl","-o",out,url,(char *)0);_exit(127);}
 do { waited=(int)waitpid(pid,&status,0); } while(waited<0 && errno==EINTR);
 if(waited!=pid)return -1;
 return status==0?0:-1;
}
static int cleanup(void) {
 int i,failed=0;
 for(i=first;i<last;i++){paths(i);if(exists(tmp)&&unlink(tmp)<0)failed=1;if(exists(old)&&unlink(old)<0)failed=1;if(exists(absent)&&unlink(absent)<0)failed=1;}
 if(failed)return -1;
 if(exists(WORK "/done")&&unlink(WORK "/done")<0)return -1;
 if(exists(WORK "/package")&&unlink(WORK "/package")<0)return -1;
 return rmdir(WORK);
}
static int rollback(void) {
 int i,failed=0;
 for(i=last-1;i>=first;i--){paths(i);
  if(exists(old)) {if(exists(files[i].dest)&&unlink(files[i].dest)<0){failed=1;continue;}if(rename(old,files[i].dest)<0)failed=1;}
  else if(exists(absent)&&exists(files[i].dest)&&unlink(files[i].dest)<0)failed=1;
 }
 sync(); if(failed)return -1; return cleanup();
}
static int recover(void) {
 FILE *f; char name[24], owner[24]; int pid;
 if(!exists(WORK)){puts("No interrupted install.");return 0;}
 f=fopen(WORK "/package","r");
 if(!f){puts("Missing transaction record. Keep /.lodge for inspection.");return 1;}
 if(!fgets(name,sizeof(name),f)){fclose(f);return 1;}
 if(fgets(owner,sizeof(owner),f)){pid=atoi(owner);if(pid>1 && (kill(pid,0)==0 || errno==EPERM)){fclose(f);puts("Installer is still running; recovery refused.");return 1;}}
 fclose(f);
 name[strcspn(name,"\r\n")]=0;
 if(selectpkg(name)<0){puts("Invalid transaction record; no files changed.");return 1;}
 if((exists(WORK "/done")?cleanup():rollback())<0){puts("Recovery incomplete. Keep /.lodge and retry recover.");return 1;}
 puts("Recovery complete.");return 0;
}
static int install(const char *name) {
 int i,fd,err; FILE *f; struct stat st;
 if(selectpkg(name)<0){puts("Unknown package. Run: elkslodge list");return 1;}
 if(getuid()!=0){puts("Install as root.");return 1;}
 if(directory("/bin")||directory("/usr")||directory("/usr/lib")||directory("/usr/lib/fortune")){puts("Cannot prepare destination directories.");return 1;}
 if(mkdir(WORK,0700)<0){puts("Install busy or interrupted. Run: elkslodge recover");return 1;}
 f=fopen(WORK "/package","w");if(!f){rmdir(WORK);return 1;}
 err=fprintf(f,"%s\n%d\n",name,(int)getpid())<0;
 if(fclose(f)!=0)err=1;
 if(err){puts("Cannot record install.");return 1;}sync();
 for(i=first;i<last;i++){
  if(lstat(files[i].dest,&st)==0&&!S_ISREG(st.st_mode)){puts("Destination is not a regular file.");goto fail;}
  if(lstat(files[i].dest,&st)<0&&errno!=ENOENT){puts("Cannot inspect destination.");goto fail;}
  paths(i);printf("Fetching %s\n",files[i].url);fflush(stdout);
  if(fetch(files[i].url,tmp)<0||stat(tmp,&st)<0||st.st_size!=files[i].size||chmod(tmp,files[i].mode)<0){puts("Download failed or wrong size. Existing programs unchanged.");goto fail;}
 }
 signal(SIGINT,SIG_IGN);signal(SIGTERM,SIG_IGN);signal(SIGHUP,SIG_IGN);
 sync(); puts("Installing...");
 for(i=first;i<last;i++){
  paths(i);
  if(exists(files[i].dest)){if(rename(files[i].dest,old)<0)goto fail;}
  else {fd=marker(absent);if(fd<0)goto fail;}
  sync();if(rename(tmp,files[i].dest)<0)goto fail;sync();
 }
 if(marker(WORK "/done")<0 && !exists(WORK "/done"))goto fail;
 sync();
 if(cleanup()<0){puts("Installed; run elkslodge recover to clean staging.");return 1;}
 puts("Installed successfully.");return 0;
 fail:
 if(rollback()<0)puts("Recovery needed: elkslodge recover. Do not delete /.lodge.");
 else puts("Install cancelled; previous files restored.");
 return 1;
}
int main(int argc,char **argv) {
 int config;
 if(argc==2&&!strcmp(argv[1],"recover")){if(getuid()!=0)return 1;return recover();}
 if(argc>1 && (!strcmp(argv[1],"list")||!strcmp(argv[1],"install"))){
  config=loadserver();
  if(config<0){puts(config==-2?"No server configured. Set ELKSLODGE_SERVER or /etc/elkslodge to http://host:port":"Invalid server. Use http://host:port in ELKSLODGE_SERVER or /etc/elkslodge");return 1;}
 }
 if(argc==2&&!strcmp(argv[1],"list")){puts("ELKSLODGE 0.5 - ELKS/8086 packages\nneofetch  Native system information\ntplay     Experimental Tandy PSG MIDI player + demo MIDI\nfortune   Fortune + BOFH and all indexed data\nbofh      Alias for the complete fortune package\ncowsay    Talking cow + cowthink companion\ncowthink  Alias for the complete cowsay package\nelksay    Talking elk + elkthink companion\nelkthink  Alias for the complete elksay package\nall       Install everything above\n\nThis release uses pinned package sizes, not cryptographic authentication.");printf("Server: %s\n",server);return 0;}
 if(argc==3&&!strcmp(argv[1],"install"))return install(argv[2]);
 puts("ELKSLODGE 0.5\n  elkslodge list\n  elkslodge install neofetch|tplay|fortune|bofh|cowsay|cowthink|elksay|elkthink|all\n  elkslodge recover\nDownloads are staged before installation. Keep power on during install.");return argc==1?0:1;
}
