/* Native ELKS/8086 SN76489 test for a known Tandy 1000.
 * Exclusive foreground utility, no IRQ/PIT/vector/kernel changes.
 * PSG registers cannot be read back; finish with all four voices muted.
 * Do not run alongside other software writing the PSG. */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define LOCK "/tmp/psgtest.lock"
static volatile int interrupted;
static void stop(int sig) { (void)sig; interrupted = 1; }
#ifdef PSG_HOST_TEST
extern void psg_write(unsigned char value);
extern int psg_wait(unsigned long usec);
#else
static void psg_write(unsigned char value)
{
    __asm__ volatile ("outb %0,$0xc0" : : "a" (value));
}
#define psg_wait usleep
#endif
static void mute(void)
{
    unsigned i;
    for (i = 0; i < 4; ++i) psg_write((unsigned char)(0x9f + (i << 5)));
}
static void tone(unsigned voice, unsigned period, unsigned attenuation)
{
    psg_write((unsigned char)(0x80 | (voice << 5) | (period & 15)));
    psg_write((unsigned char)((period >> 4) & 63));
    psg_write((unsigned char)(0x90 | (voice << 5) | attenuation));
}
static int wait_ticks(unsigned ticks)
{
    while (ticks-- && !interrupted) {
        if (psg_wait(50000UL) < 0 && errno != EINTR) return -1;
    }
    return interrupted ? -1 : 0;
}
static int play(void)
{
    static const unsigned periods[] = {428,381,339,320,285,254,226,214};
    unsigned i;
    /* C major scale. PSG master clock 3.579545 MHz, divider 32. */
    for (i = 0; i < 8 && !interrupted; ++i) {
        tone(0, periods[i], 6);
        if (wait_ticks(5) < 0) return -1;
    }
    mute();
    if (interrupted) return -1;
    /* C/E/G chord: three simultaneous hardware oscillators. */
    tone(0,428,8); tone(1,339,8); tone(2,285,8);
    if (wait_ticks(20) < 0) return -1;
    mute();
    if (interrupted) return -1;
    /* Quiet short white-noise burst, fixed clock, independent of tone 2. */
    psg_write(0xe6); psg_write(0xfa);
    return wait_ticks(4);
}
int main(int argc, char **argv)
{
    int fd, status;
    if (argc != 2 || (strcmp(argv[1],"--tandy") && strcmp(argv[1],"--mute"))) {
        puts("Usage: psgtest --tandy  (3-second scale, chord, noise)");
        puts("       psgtest --mute   (silence all four PSG voices)");
        puts("Known Tandy 1000 hardware only. Port C0 is unsafe on other PCs.");
        puts("Stop other PSG programs first. Ctrl-C stops and silences.");
        return 2;
    }
    signal(SIGINT,stop); signal(SIGTERM,stop); signal(SIGHUP,stop);
    signal(SIGQUIT,stop); signal(SIGTSTP,stop);
    fd = open(LOCK,O_WRONLY|O_CREAT|O_EXCL,0600);
    if (fd < 0) {
        perror(LOCK);
        puts("Another test may be running. Remove a stale lock only after checking.");
        return 1;
    }
    close(fd);
    mute();
    status = strcmp(argv[1],"--mute") ? play() : 0;
    mute();
    unlink(LOCK);
    if (status < 0) { puts("Sound stopped."); return 1; }
    puts("PSG muted. Done.");
    return 0;
}
