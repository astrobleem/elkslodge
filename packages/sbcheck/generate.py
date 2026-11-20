"""Original two-tone Sound Blaster check, MIT; see repository LICENSE.
Host Python 3 only. Output is raw unsigned 8-bit mono at 8000 Hz, plus WAV.
No external audio, converter, or target-side generation is required.
"""
import argparse
import math
from pathlib import Path
import wave

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('out',type=Path)
    a=p.parse_args()
    a.out.mkdir(parents=True,exist_ok=False)
    rate=8000
    data=bytearray([128])*16384
    # Quiet margins and a pause distinguish these PCM tones from a boot beep.
    for begin,end,freq in [(1000,5000,500),(10000,15000,1000)]:
        for n in range(begin,end):
            fade=min(1.0,(n-begin)/64,(end-1-n)/64)
            data[n]=round(128+32*fade*math.sin(2*math.pi*freq*(n-begin)/rate))
    (a.out/'SBCHECK.U8').write_bytes(data)
    with wave.open(str(a.out/'SBCHECK.WAV'),'wb') as f:
        f.setnchannels(1);f.setsampwidth(1);f.setframerate(rate);f.writeframes(data)
    print('Created 16384 original U8 samples, mono 8000 Hz, 500/1000 Hz tones.')
if __name__=='__main__':main()
