/*
  Hatari - audio.h

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.
*/

#ifndef HATARI_AUDIO_H
#define HATARI_AUDIO_H

#include <stdint.h>

extern int nAudioFrequency;
extern bool bSoundWorking;
extern int SoundBufferSize;
extern int SdlAudioBufferSize;
extern int pulse_swallowing_count;


extern void Audio_Init(void);
extern void Audio_UnInit(void);
extern void Audio_Lock(void);
extern void Audio_Unlock(void);
extern void Audio_FreeSoundBuffer(void);
extern void Audio_SetOutputAudioFreq(int Frequency);
extern void Audio_EnableAudio(bool bEnable);

/* Copy up to `frames` stereo frames out of the mix ring. Returns how many
 * were copied. 0 when sound is off or the ring is empty. */
extern int Audio_Read(int16_t *interleaved, int frames);

#endif  /* HATARI_AUDIO_H */
