/* Native ELKS frontend; shared libtandy SMF parser, banks and MIDI synth.
 * Unlike DOS TPLAY this neither launches Windows nor installs a timer hook.
 * File validation precedes hardware acquisition. All library pointer targets
 * are globals/heap; this build uses ELKS CS separate, DS=ES=SS ABI. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/select.h>
#include "epsg.h"
#include "SMF.H"
#include "MIDIMSG.H"
#include "demo.h"
#define MAX_INPUT 32768U
#define EVENT_BUDGET 128U
static EPSG sound;
static SMF_EVENT event;
static struct termios saved_term, raw_term;
static struct timeval tv, timeout;
static fd_set input;
static unsigned char *file_data;
static unsigned term_saved, pending, ended, paused, bank;
static volatile int interrupted;
static LTP_U32 position, duration;
static char key;
static void stop(int sig){(void)sig;interrupted=1;}
static void cleanup(void)
{
    epsg_close(&sound);
    if(term_saved){tcsetattr(0,TCSANOW,&saved_term);term_saved=0;}
    smf_close();free(file_data);file_data=0;
}
static int clock_ms(LTP_U32 *value)
{
    if(gettimeofday(&tv,0)<0)return -1;
    *value=(LTP_U32)tv.tv_sec*1000UL+(LTP_U32)tv.tv_usec/1000UL;
    return 0;
}
static LTP_U32 now,last;
static int load_file(const char *name)
{
    FILE *f;long size;
    f=fopen(name,"rb");if(!f){perror(name);return 0;}
    if(fseek(f,0L,SEEK_END)|| (size=ftell(f))<0 || size>MAX_INPUT || size<14){
        puts("MIDI file must be 14..32768 bytes in this ELKS build.");fclose(f);return 0;
    }
    rewind(f);file_data=malloc((unsigned)size);
    if(!file_data){puts("Not enough free memory for MIDI.");fclose(f);return 0;}
    if(fread(file_data,1,(unsigned)size,f)!=(unsigned)size||ferror(f)){
        puts("Cannot read complete MIDI file.");fclose(f);return 0;
    }
    fclose(f);return smf_open(file_data,(unsigned)size);
}
/* Preflight the renderer admission policy too, before any PSG writes. */
static int admit(void)
{
    unsigned budget=0;int rc;
    while(!interrupted){
        rc=smf_step(&event);
        if(rc<0)return 0;
        if(!rc)return smf_rewind();
        if(rc==1&&!MSValidMessage((LTP_U32)event.status|((LTP_U32)event.data1<<8)|((LTP_U32)event.data2<<16),2)){
            printf("Unsupported MIDI message: %02x %u %u at %lu ms\n",event.status,event.data1,event.data2,(unsigned long)event.ms);
            return 0;
        }
        if(++budget==EVENT_BUDGET){budget=0;if(usleep(1000UL)<0&&errno!=EINTR)return 0;}
    }
    return 0;
}
/* Bound parser/dispatcher work, including skipped metadata, each iteration.
 * Do not publish a partial same-time MIDI group when the budget runs out. */
static int pump(void)
{
    unsigned n;int rc;
    for(n=0;n<EVENT_BUDGET;++n){
        if(ended)break;
        if(!pending){
            rc=smf_step(&event);
            if(rc<0)return -1;
            if(!rc){ended=1;break;}
            if(rc==2)continue;
            pending=1;
        }
        if(event.ms>position)break;
        if(epsg_message(&sound,event.ms,(LTP_U32)event.status|((LTP_U32)event.data1<<8)|((LTP_U32)event.data2<<16))<0)return -1;
        pending=0;
    }
    if(n<EVENT_BUDGET)epsg_render(&sound,position);
    return 0;
}
static int restart(void)
{
    if(!smf_rewind())return -1;
    epsg_reset(&sound,bank);position=0;pending=ended=paused=0;
    if(clock_ms(&last)<0)return -1;
    puts("Replay.");return 0;
}
static int keyboard(void)
{
    int rc;
    FD_ZERO(&input);if(term_saved)FD_SET(0,&input);
    timeout.tv_sec=0;timeout.tv_usec=10000;
    rc=select(term_saved?1:0,term_saved?&input:0,0,0,&timeout);
    if(rc<0)return errno==EINTR?0:-1;
    if(rc>0&&FD_ISSET(0,&input)){
        rc=read(0,&key,1);if(rc!=1)return -1;
        if(key=='q'||key=='Q'||key==27)interrupted=1;
        else if(key==' '){paused=!paused;epsg_pause(&sound,paused);puts(paused?"Paused.":"Playing.");if(clock_ms(&last)<0)return -1;}
        else if(key=='r'||key=='R')return restart();
        else if(key=='b'||key=='B'){bank=!bank;(void)MSBank(&sound.synth,(LTP_U16)bank);puts(bank?"Bank: Chamber (new notes).":"Bank: Default (new notes).");}
        else if(key=='+'||key=='='){if(sound.attenuation)epsg_volume(&sound,sound.attenuation-1);}
        else if(key=='-')epsg_volume(&sound,sound.attenuation+1);
        else if(key>='1'&&key<='3')epsg_effect(&sound,(unsigned)(key-'0'),position);
    }
    return 0;
}
int main(int argc,char **argv)
{
    const char *name=0;int i,known=0,check=0,demo=0,rc=0,opened;
    for(i=1;i<argc;++i){
        if(!strcmp(argv[i],"--tandy"))known=1;
        else if(!strcmp(argv[i],"--check"))check=1;
        else if(!strcmp(argv[i],"--demo"))demo=1;
        else if(!strcmp(argv[i],"--chamber"))bank=1;
        else if(argv[i][0]!='-'&&!name)name=argv[i];
        else goto usage;
    }
    if((!demo&&!name)||(demo&&name)||(!check&&!known))goto usage;
    signal(SIGINT,stop);signal(SIGTERM,stop);signal(SIGHUP,stop);
    signal(SIGQUIT,stop);signal(SIGTSTP,stop);signal(SIGPIPE,stop);
    opened=demo?smf_open(demo_midi,sizeof(demo_midi)):load_file(name);
    if(!opened){printf("MIDI refused: %s\n",smf_error());cleanup();return 1;}
    if(!admit()){puts("MIDI admission refused; no hardware accessed.");cleanup();return 1;}
    duration=smf_duration_ms();
    printf("TPLAY/ELKS: %u tracks, %u events, %lu ms\n",smf_track_count(),smf_event_count(),(unsigned long)duration);
    if(check){puts("Valid MIDI; no hardware accessed.");cleanup();return 0;}
    if(interrupted){cleanup();return 1;}
    if(epsg_open(&sound,known)<0){
        perror("PSG acquire");puts("Stop other PSG apps. Check /tmp/psgtest.lock; never remove a live owner's lock.");cleanup();return 1;
    }
    (void)MSBank(&sound.synth,(LTP_U16)bank);
    puts("3 PSG tones + noise drums. Low bass octave-folded; mono output.");
    puts("Space pause | R replay | B bank | +/- volume | 1/2/3 effects | Q quit");
    if(isatty(0)){
        if(tcgetattr(0,&saved_term)<0){rc=1;goto done;}
        raw_term=saved_term;raw_term.c_lflag&=~(ICANON|ECHO);raw_term.c_cc[VMIN]=1;raw_term.c_cc[VTIME]=0;
        term_saved=1;if(tcsetattr(0,TCSANOW,&raw_term)<0){rc=1;goto done;}
    }
    if(clock_ms(&last)<0){rc=1;goto done;}
    while(!interrupted){
        LTP_U32 delta;
        if(clock_ms(&now)<0){rc=1;break;}
        delta=now-last;last=now;
        if(delta>5000UL){puts("Clock jump or long stall; playback stopped safely.");rc=1;break;}
        if(!paused){
            position+=delta;
            if(pump()<0){printf("MIDI playback refused: %s\n",smf_error());rc=1;break;}
            if(ended&&position>=duration)break;
        }
        if(keyboard()<0){rc=1;break;}
    }
done:
    cleanup();puts("Stopped; PSG muted.");return rc;
usage:
    puts("tplay --tandy --demo [--chamber]");
    puts("tplay --tandy song.mid [--chamber]");
    puts("tplay --check song.mid  (validate without hardware)");
    puts("Known Tandy 1000 only; not for ordinary PC port C0.");
    return 2;
}
