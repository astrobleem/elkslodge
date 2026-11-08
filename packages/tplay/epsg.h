/* ELKS foreground transport for the portable libtandy MIDI synthesizer.
 * No IRQ ownership, heap or OS calls in the upstream renderer.
 * One open epsg per process; all users must honor the shared file lock.
 * Calls are foreground-only, serialized, with millisecond timestamps.
 * Pass 1 to open only on hardware explicitly known to be a Tandy 1000.
 * Initialize EPSG to all zeros before first open.
 * Always close on normal exit/signals; SIGKILL cannot be cleaned up. */
#ifndef EPSG_H
#define EPSG_H
#include "MIDISYN.H"
#include "lt_sound.h"
typedef struct {
    MSYN synth;
    lt_chip chip;
    unsigned active, attenuation, paused, effect, drum_restart;
    LTP_U32 effect_until;
} EPSG;
int epsg_open(EPSG *, int known_tandy);
void epsg_close(EPSG *);
void epsg_reset(EPSG *, unsigned bank);
int epsg_message(EPSG *, LTP_U32 ms, LTP_U32 packed_midi);
void epsg_render(EPSG *, LTP_U32 ms);
void epsg_pause(EPSG *, unsigned paused);
void epsg_volume(EPSG *, unsigned attenuation);
void epsg_effect(EPSG *, unsigned effect, LTP_U32 ms);
#endif
