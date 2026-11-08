/* Isolated host tests: no device I/O and no real filesystem lock. */
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "epsg.h"
static unsigned bytes[4096], count, lease, opens, closes, unlinks;
static EPSG state, other;
void epsg_host_out(unsigned char b)
{
    assert(lease); assert(count < 4096); bytes[count++] = b;
}
static int fake_open(const char *path, int flags, ...)
{
    assert(!strcmp(path,"/tmp/psgtest.lock"));
    assert(flags == (O_WRONLY|O_CREAT|O_EXCL));
    ++opens;
    if(lease) return -1;
    lease=1; return 9;
}
static int fake_close(int fd) { assert(fd==9); ++closes; return 0; }
static int fake_unlink(const char *path)
{
    assert(!strcmp(path,"/tmp/psgtest.lock")); assert(lease);
    ++unlinks; lease=0; return 0;
}
#define EPSG_HOST_TEST 1
#define open fake_open
#define close fake_close
#define unlink fake_unlink
#include "epsg.c"
#undef open
#undef close
#undef unlink
static void four_muted(void)
{
    unsigned i;
    assert(count>=4);
    for(i=0;i<4;++i) assert(bytes[count-4+i]==0x9f+(i<<5));
}
int main(void)
{
    unsigned n;
    assert(epsg_open(&state,0)==-1); assert(!opens&&!count);
    assert(epsg_open(&state,1)==0);
    assert(lease&&opens==1&&closes==1&&count==4); four_muted();
    assert(state.attenuation==3&&state.active);
    assert(epsg_open(&state,1)==-1); assert(opens==1&&count==4);
    assert(epsg_open(&other,1)==-1); assert(opens==2&&count==4);
    epsg_close(&other); assert(!unlinks&&lease);
    assert(epsg_message(&state,0,0x005a3c90UL)==0);
    epsg_render(&state,0); assert(state.chip.attenuation[0]<15);
    n=count; epsg_render(&state,0); assert(count==n);
    epsg_volume(&state,99); epsg_render(&state,0);
    assert(state.attenuation==15&&state.chip.attenuation[0]==15);
    epsg_volume(&state,0); epsg_render(&state,0);
    assert(state.chip.attenuation[0]<15);
    epsg_pause(&state,1); four_muted(); n=count;
    epsg_render(&state,100); epsg_effect(&state,1,100); assert(count==n);
    epsg_pause(&state,0); epsg_render(&state,0);
    assert(state.chip.attenuation[0]<15);
    epsg_effect(&state,1,100); assert(bytes[count-1]==0xe4);
    n=count; epsg_effect(&state,1,100); assert(count==n+1);
    epsg_render(&state,100); assert(state.chip.attenuation[3]==5);
    epsg_render(&state,279); assert(state.effect==1);
    epsg_render(&state,280); assert(!state.effect&&state.chip.attenuation[3]==15);
    epsg_reset(&state,MS_BANK_CHAMBER); four_muted();
    assert(state.synth.bank==MS_BANK_CHAMBER);
    epsg_close(&state); four_muted(); assert(!lease&&unlinks==1);
    n=count; epsg_close(&state); assert(count==n&&unlinks==1);
    assert(epsg_message(&state,0,0x005a3c90UL)==-1);
    epsg_render(&state,0); assert(count==n);
    assert(epsg_open(&state,1)==0); epsg_close(&state);
    assert(unlinks==2&&!lease);
    puts("epsg transport tests passed (mock I/O and lock).");
    return 0;
}
