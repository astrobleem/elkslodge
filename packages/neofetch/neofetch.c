/* Native ELKS adaptation of the neofetch system-summary idea.
 * Original neofetch: https://github.com/dylanaraps/neofetch (MIT).
 * This is a small C reimplementation, not the upstream Bash program.
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#ifndef HOST_TEST
#include <sys/ioctl.h>
#include <linuxmt/mem.h>
#include <arch/param.h>
#endif

static int color, plain, row;
static const char *logo[] = {
    "  __________", " |  TANDY   |", " |   ELKS   |", " |__________|",
    "    |____|", "  /________/", "", "", "", "", ""
};

/* Environment and hostname strings are data, never terminal control codes. */
static void safe(const char *s, int max)
{
    unsigned char c;
    if (!s || !*s) s = "unknown";
    while (*s && max-- > 0) {
        c = (unsigned char)*s++;
        putchar(c >= 32 && c < 127 ? c : '?');
    }
}

static void field(const char *key, const char *value)
{
    if (!plain) {
        if (color) fputs("\033[36m", stdout);
        printf("%-14s", row < 11 ? logo[row] : "");
        if (color) fputs("\033[0m", stdout);
    }
    if (color) fputs("\033[1;33m", stdout);
    fputs(key, stdout);
    if (color) fputs("\033[0m", stdout);
    fputs(": ", stdout);
    safe(value, 49);
    putchar('\n');
    row++;
}

static void resources(void)
{
#ifndef HOST_TEST
    int fd;
    unsigned int ds, off;
    unsigned long ticks, seconds;
    struct mem_usage mu;
    char text[72];
    /* Linear /dev/kmem addressing works on old and new real-mode ELKS.
     * No writes, interrupt instructions, or raw hardware accesses. */
    fd = open("/dev/kmem", O_RDONLY);
    if (fd >= 0 && ioctl(fd, MEM_GETUSAGE, &mu) == 0) {
        sprintf(text, "%u / %lu KiB (ELKS main)", mu.main_used,
                (unsigned long)mu.main_used + mu.main_free);
        field("Memory", text);
    } else field("Memory", "unavailable (/dev/kmem)");
    if (fd >= 0 && ioctl(fd, MEM_GETDS, &ds) == 0 &&
        ioctl(fd, MEM_GETUPTIME, &off) == 0 &&
        lseek(fd, ((unsigned long)ds << 4) + off, SEEK_SET) != -1L &&
        read(fd, &ticks, sizeof(ticks)) == sizeof(ticks)) {
        seconds = ticks / HZ;
        sprintf(text, "%lu days, %u hours, %u mins", seconds / 86400L,
                (unsigned int)(seconds / 3600L % 24),
                (unsigned int)(seconds / 60L % 60));
        field("Uptime", text);
    } else field("Uptime", "unavailable (/dev/kmem)");
    if (fd >= 0) close(fd);
#else
    field("Memory", "unavailable (host rendering test)");
    field("Uptime", "unavailable (host rendering test)");
#endif
}

int main(int argc, char **argv)
{
    int i, tandy = 0;
    char text[100];
    const char *s;
    struct utsname u;
    struct passwd *pw;
    color = isatty(1) && getenv("NO_COLOR") == NULL;
    s = getenv("TERM");
    if (s && !strcmp(s, "dumb")) color = 0;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--stdout")) { plain = 1; color = 0; }
        else if (!strcmp(argv[i], "--no-color")) color = 0;
        else if (!strcmp(argv[i], "--tandy")) tandy = 1;
        else if (!strcmp(argv[i], "--version")) {
            puts("neofetch-elks 1.0 (native C subset; not upstream Bash)");
            return 0;
        } else if (!strcmp(argv[i], "--help")) {
            puts("Usage: neofetch [--tandy] [--stdout] [--no-color]");
            puts("  --tandy     Label the user-selected Tandy 1000 EX / 8088 profile");
            puts("  --stdout    Text only, without ASCII art or ANSI colors");
            puts("  --no-color  ASCII art without ANSI colors");
            puts("  --version   Show port version");
            puts("Native ELKS subset. No Bash, config files, images or package probes.");
            puts("Memory is ELKS main allocator usage, not total installed RAM.");
            puts("Uptime uses the kernel tick counter and wraps after about 497 days.");
            return 0;
        } else {
            fputs("neofetch: unsupported option; use --help\n", stderr);
            return 2;
        }
    }
    if (uname(&u) == 0) {
        /* uname structures are fixed-size on ELKS. */
        u.sysname[sizeof(u.sysname)-1] = 0;
        u.nodename[sizeof(u.nodename)-1] = 0;
        u.release[sizeof(u.release)-1] = 0;
        u.machine[sizeof(u.machine)-1] = 0;
        field("Host", u.nodename);
        sprintf(text, "%.20s %.20s", u.sysname, u.release);
        field("OS", text);
        field("Machine", u.machine);
    } else {
        field("Host", "unavailable"); field("OS", "unavailable");
        field("Machine", "unavailable");
    }
    if (tandy) field("Profile", "Tandy 1000 EX / 8088 (selected)");
    else field("Profile", "ELKS real-mode PC");
    pw = getpwuid(getuid());
    if (pw) field("User", pw->pw_name);
    else { sprintf(text, "uid %u", (unsigned int)getuid()); field("User", text); }
    resources();
    /* These are the user's declared environment, not process detection. */
    field("Shell env", getenv("SHELL"));
    field("Terminal", getenv("TERM"));
    field("Port", "neofetch-elks 1.0");
    return ferror(stdout) ? 1 : 0;
}
