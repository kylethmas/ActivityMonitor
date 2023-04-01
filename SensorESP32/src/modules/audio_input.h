#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

/**
 * This module handles the audio input and buffering from I2S.
 * 
 * @authors Thomas M. Civan D.
*/

/**
 * Setup method called by the main task to initialise any resources needed by
 * the audio subsystem.
*/
void setupAudioInput();

/**
 * Reads the microphone data into a buffer and returns a pointer to the start of
 * the buffer. This methods blocks until the buffer is full, but the dma should
 * continue sampling the microphone after this method has finished.
 * 
 * @param buffer a pointer to the buffer containing the sampled audio
 * @returns how may samples were read into the buffer, in rare cases 
 *          this can be smaller than the size of the buffer
*/
int sampleMicrophone(uint16_t* buffer);

#endif // AUDIO_INPUT_H