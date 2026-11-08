#include "epsg.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define EPSG_LOCK "/tmp/psgtest.lock"
#ifdef EPSG_HOST_TEST
extern void epsg_host_out(unsigned char);
#endif
static void emit(void *unused, lt_u8 value)
{
    (void)unused;
#ifdef EPSG_HOST_TEST
    epsg_host_out(value);
#else
    __asm__ volatile ("outb %0,$0xc0" : : "a" (value));
#endif
}
static void silence(EPSG *s)
{
    unsigned i;
    for(i=0;i<4;++i) (void)lt_attenuate(&s->chip,1,(lt_u8)i,15,1);
}
int epsg_open(EPSG *s,int known_tandy)
{
    int fd;
    if(!s || !known_tandy) return -1;
    /* Open only an initialized, inactive object. Never erase a live lease. */
    if(s->active) return -1;
    fd=open(EPSG_LOCK,O_WRONLY|O_CREAT|O_EXCL,0600);
    if(fd<0) return -1;
    close(fd);
    memset(s,0,sizeof(*s));
    lt_init(&s->chip,emit,0);
    (void)lt_claim(&s->chip,1,15);
    s->active=1;s->attenuation=3;
    epsg_reset(s,MS_BANK_DEFAULT);
    return 0;
}
void epsg_close(EPSG *s)
{
    if(!s || !s->active) return;
    (void)lt_stop(&s->chip,1,15);
    s->active=0;unlink(EPSG_LOCK);
}
void epsg_reset(EPSG *s,unsigned bank)
{
    if(!s->active)return;
    silence(s);MSReset(&s->synth);MSSpeakerMode(&s->synth,2);
    (void)MSBank(&s->synth,(LTP_U16)bank);
    s->paused=s->effect=s->drum_restart=0;
}
int epsg_message(EPSG *s,LTP_U32 ms,LTP_U32 msg)
{
    if(!s->active)return -1;
    if(MSMessageDeferred(&s->synth,ms,msg))return -1;
    if((msg&255UL)==0x99UL && ((msg>>16)&127UL))s->drum_restart=1;
    return 0;
}
static unsigned level(EPSG *s,unsigned value)
{
    value+=s->attenuation;return value>15?15:value;
}
void epsg_render(EPSG *s,LTP_U32 ms)
{
    unsigned i;
    if(!s->active||s->paused)return;
    MSRender(&s->synth,ms);
    for(i=0;i<3;++i){
        if(s->synth.volume[i]<15)
            (void)lt_tone(&s->chip,1,(lt_u8)i,s->synth.p[i],0);
        (void)lt_attenuate(&s->chip,1,(lt_u8)i,(lt_u8)level(s,s->synth.volume[i]),0);
    }
    if(s->effect && (LTP_I32)(ms-s->effect_until)>=0)s->effect=0;
    if(s->effect){
        (void)lt_noise(&s->chip,1,(lt_u8)(s->effect==1?4:s->effect==2?5:6),0);
        (void)lt_attenuate(&s->chip,1,3,(lt_u8)level(s,5),0);
    }else{
        if(s->synth.volume[3]<15)(void)lt_noise(&s->chip,1,(lt_u8)s->synth.noise,s->drum_restart);
        (void)lt_attenuate(&s->chip,1,3,(lt_u8)level(s,s->synth.volume[3]),0);
    }
    s->drum_restart=0;
}
void epsg_pause(EPSG *s,unsigned paused)
{
    if(!s->active)return;
    s->paused=paused!=0;if(s->paused)silence(s);
}
void epsg_volume(EPSG *s,unsigned attenuation)
{if(s->active)s->attenuation=attenuation>15?15:attenuation;}
void epsg_effect(EPSG *s,unsigned effect,LTP_U32 ms)
{
    if(!s->active||s->paused||effect<1||effect>3)return;
    s->effect=effect;s->effect_until=ms+180UL;
    (void)lt_noise(&s->chip,1,(lt_u8)(effect==1?4:effect==2?5:6),1);
}
