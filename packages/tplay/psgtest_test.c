#define PSG_HOST_TEST 1
#define main psgtest_main
#include "psgtest.c"
#undef main
#include <assert.h>
static unsigned char bytes[256];
static unsigned count, ticks, cancel_at, error_at;
void psg_write(unsigned char value) { assert(count < sizeof bytes); bytes[count++] = value; }
int psg_wait(unsigned long usec)
{
    assert(usec == 50000UL); ++ticks;
    if (ticks == cancel_at) { stop(SIGINT); errno = EINTR; return -1; }
    if (ticks == error_at) { errno = EIO; return -1; }
    return 0;
}
static void reset(void) { count=ticks=cancel_at=error_at=0; interrupted=0; }
int main(void)
{
    unsigned i;
    reset(); assert(play()==0); assert(ticks==64); assert(count==43);
    assert(bytes[0]==0x8c && bytes[1]==26 && bytes[2]==0x96);
    assert(bytes[28]==0x8c && bytes[29]==26 && bytes[30]==0x98);
    assert(bytes[31]==0xa3 && bytes[32]==21 && bytes[33]==0xb8);
    assert(bytes[34]==0xcd && bytes[35]==17 && bytes[36]==0xd8);
    assert(bytes[41]==0xe6 && bytes[42]==0xfa);
    for(i=1;i<=64;++i) {
        reset(); cancel_at=i; assert(play()==-1); assert(ticks==i);
        mute(); assert(bytes[count-4]==0x9f && bytes[count-3]==0xbf);
        assert(bytes[count-2]==0xdf && bytes[count-1]==0xff);
    }
    reset(); error_at=3; assert(play()==-1 && ticks==3);
    reset(); interrupted=1; assert(play()==-1 && ticks==0);
    puts("PASS: PSG tone/chord/noise bytes, 64 yielded ticks, interrupt at every tick, error path, four-channel mute");
    return 0;
}
