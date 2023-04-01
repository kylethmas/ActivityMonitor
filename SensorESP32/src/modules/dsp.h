#ifndef DSP_H
#define DSP_H

#include <Arduino.h>

/**
 * This module processes audio data and prepares it for the classification
 * algorithm.
 *
 * @authors Thomas M.
 */

/**
 * Given a double buffer defines which window should be processed.
 * BUFFER_A \ Window A     |
 * BUFFER_A |              /
 * BUFFER_A |    \ Window B
 * BUFFER_A /    |  
 * BUFFER_B      |    \ Window C
 * BUFFER_B      /    |
 * BUFFER_B           |    \ Window D
 * BUFFER_B           /    |
*/
typedef enum DSP_WINDOW
{
    A,
    B,
    C,
    D
};

/**
 * Setup method called by the main task to initialise any resources needed by
 * the dsp subsystem.
 */
void setupDSP();

/**
 * Performs an initial analysis of an audio sample to determine if it's interesting or not.
*/
bool dspInitialAnalysis(uint16_t* buffer, int n_samples);

/**
 * Process audio samples with the dsp. This computes the FFT and performs any
 * other filtering needed before classification.
 * 
 * @param buffer_a The first half of the circular buffer to process
 * @param buffer_b The second half of the circular buffer to process
 * @param n_samples The size of one of the input buffers
 * @param window_index The index of the window to process
 * @param output_buffer The buffer to store the filterbank energies in
*/
void dspProcessSamples(uint16_t *buffer_a, uint16_t* buffer_b, int n_samples, DSP_WINDOW window_index, float* output_buffer);

#endif // DSP_H