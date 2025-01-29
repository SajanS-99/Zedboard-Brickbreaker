#ifndef AUDIO_H
#define AUDIO_H

/*
 * Header file for audio.c so that we can more easily play audio from core game logic
 * */

void init_audio();
//This function initializes everything needed to play audio from an SD card.
//and is mostly an altered form of the guide's int main() function.
//It sets up the DMA, mounts and reads the SD card, and saves .wav file names.

void remotePlay(int num, int vol);
//This function is basically just a wrapper for the play .wav function
//It takes in a volume value, and song index
//updates the volume as necessary, stops any currently playing music, and plays the instructed song.

void cleanup_audio();
//This function is basically just a wrapper for the function to reset the DMA so that external files can call it easier.

#endif
