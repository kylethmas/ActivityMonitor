/**
 * This module processes audio data and prepares it for the classification
 * algorithm.
 *
 * @authors Thomas M.
 */

#include "dsp.h"
#include "config.h"
#include "debug.h"
#include "utils.h"
#include <esp_dsp.h>

__attribute__((aligned(16))) float fft_window[FFT_SIZE];
__attribute__((aligned(16))) float fft_buffer[FFT_SIZE << 1];
__attribute__((aligned(16))) float fft_avg_buffer[FFT_SIZE << 1];
float filter_centers[N_MEL_FILTERS + 2]; 

char dbg_display_msg[32] = {0};

inline float toMelSpace(float x)
{
    return 1125 * log(1 + x / 700.0f);
}

inline float fromMelSpace(float x)
{
    return 700 * (exp(x / 1125.0f) - 1);
}

/**
 * Setup method called by the main task to initialise any resources needed by
 * the dsp subsystem.
 */
void setupDSP()
{
    if (dsps_fft4r_init_fc32(NULL, FFT_SIZE) != ESP_OK)
    {
        dbg_log_fatal("Not possible to initialize fixed point FFT-R4.");
        while (1)
            ;
    }

    // dsps_wind_blackman_harris_f32(fft_window, FFT_SIZE << 1);
    dsps_wind_hann_f32(fft_window, FFT_SIZE);
    for (int i = 0; i < (FFT_SIZE << 1); i++)
        fft_avg_buffer[i] = 0.0f;

    // Find the center frequencies for the mel filter banks
    // The algorithm for this is derived from:
    // http://practicalcryptography.com/miscellaneous/machine-learning/guide-mel-frequency-cepstral-coefficients-mfccs/#computing-the-mel-filterbank
    const float mel_low = toMelSpace(MEL_FILTER_LOW_FREQ);
    const float mel_high = toMelSpace(MEL_FILTER_HIGH_FREQ);
    for(int i = 0; i < N_MEL_FILTERS + 2; i++)
    {
        float freq = fromMelSpace(mel_low + (mel_high - mel_low) * (i/(float)N_MEL_FILTERS));
        filter_centers[i] = ((FFT_SIZE/2 + 1) * freq / AUDIO_SAMPLE_RATE);
    }
}

/**
 * Performs an initial analysis of an audio sample to determine if it's interesting or not.
*/
bool dspInitialAnalysis(uint16_t* buffer, int n_samples)
{
    float dc_offset = 0.0;
    float peak_magnitude = 0.0;
    for (int i = 0; i < n_samples; i++)
    {
        // Convert the sample to -1f to +1f space
        #ifdef AUDIO_DATA_SIGNED
        float sample = ((float)((int16_t)buffer[i] & ((1<<AUDIO_BIT_DEPTH) - 1)) * (1 / (float)(1<<AUDIO_BIT_DEPTH-1)) - 1);
        #else
        float sample = ((float)(buffer[i] & ((1<<AUDIO_BIT_DEPTH) - 1)) * (1 / (float)(1<<AUDIO_BIT_DEPTH-1)) - 1);
        #endif
        dc_offset += sample;
        peak_magnitude = max(abs(sample), peak_magnitude);
    }

    float amplitude = 20 * log10f((peak_magnitude - dc_offset));
    if(amplitude >= AUDIO_AMPLITUDE_THRESHOLD)
    {
        dbg_log("Sample peak amplitude: %f", amplitude);
        return true;
    } else
    {
        return false;
    }
}

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
void dspProcessSamples(uint16_t *buffer_a, uint16_t* buffer_b, int n_samples, DSP_WINDOW window_index, float* output_buffer)
{
    // Work out what bit of the buffer we need to sample
    int buff_ind;
    uint16_t* buff;
    switch (window_index)
    {
    case DSP_WINDOW::A:
        buff_ind = 0;
        buff = buffer_a;
        break;
    case DSP_WINDOW::B:
        buff_ind = n_samples / 2;
        buff = buffer_a;
        break;
    case DSP_WINDOW::C:
        buff_ind = 0;
        buff = buffer_b;
        break;
    case DSP_WINDOW::D:
        buff_ind = n_samples / 2;
        buff = buffer_b;
        break;
    }   

    // Convert the first half of the buffer to floats
    for (int i = 0; i < n_samples / 2; i++)
    {
        #ifdef AUDIO_DATA_SIGNED
        fft_buffer[i << 1] = ((float)((int16_t)buff[buff_ind+i] & ((1<<AUDIO_BIT_DEPTH) - 1)) * (1 / (float)(1<<AUDIO_BIT_DEPTH-1)) - 1)
                             * fft_window[i] * FFT_INPUT_SCALE;
        #else
        fft_buffer[i << 1] = ((float)(buff[buff_ind+i] & ((1<<AUDIO_BIT_DEPTH) - 1)) * (1 / (float)(1<<AUDIO_BIT_DEPTH-1)) - 1)
                             * fft_window[i] * FFT_INPUT_SCALE;
        #endif

        // Fill the odd values (complex part of the input) with zeros
        fft_buffer[(i << 1) | 1] = 0.0f;
    }

    // Now work out the second half to copy
    switch (window_index)
    {
    case DSP_WINDOW::A:
        buff_ind = n_samples / 2;
        buff = buffer_a;
        break;
    case DSP_WINDOW::B:
        buff_ind = 0;
        buff = buffer_b;
        break;
    case DSP_WINDOW::C:
        buff_ind = n_samples / 2;
        buff = buffer_b;
        break;
    case DSP_WINDOW::D:
        buff_ind = 0;
        buff = buffer_a;
        break;
    }

    // Convert the second half of the buffer to floats
    for (int i = n_samples / 2; i < n_samples; i++)
    {
        #ifdef AUDIO_DATA_SIGNED
        fft_buffer[i << 1] = ((float)((int16_t)buff[buff_ind + i - n_samples / 2] & ((1<<AUDIO_BIT_DEPTH) - 1)) * (1 / (float)(1<<AUDIO_BIT_DEPTH-1)) - 1)
                             * fft_window[i] * FFT_INPUT_SCALE;
        #else
        fft_buffer[i << 1] = ((float)(buff[buff_ind + i - n_samples / 2] & ((1<<AUDIO_BIT_DEPTH) - 1)) * (1 / (float)(1<<AUDIO_BIT_DEPTH-1)) - 1)
                             * fft_window[i] * FFT_INPUT_SCALE;
        #endif

        // Fill the odd values (complex part of the input) with zeros
        fft_buffer[(i << 1) | 1] = 0.0f;
    }

    // Compute the FFT
    dsps_fft4r_fc32(fft_buffer, FFT_SIZE);
    dsps_bit_rev4r_fc32(fft_buffer, FFT_SIZE);
    dsps_cplx2real_fc32(fft_buffer, FFT_SIZE);

    // Convert to power spectrum
    for (int i = 0; i < FFT_SIZE; i++)
    {
        fft_buffer[i] = 10 * log10f((fft_buffer[i * 2 + 0] * fft_buffer[i * 2 + 0] + fft_buffer[i * 2 + 1] * fft_buffer[i * 2 + 1] + 0.0000001) / (FFT_SIZE << 1));
        // fft_buffer[i] = fft_buffer[i] + 60;
    }

    // Compute the mel filters
    // These are a series of 50% overlapping triangular filters spaced 
    // logarithmically accross the spectrum. To apply the filter we simply 
    // multiply the fft by the filter coefficients and sum the resulting values 
    // into a single filter energy.
    // Derived from algorithm presented in:
    // http://practicalcryptography.com/miscellaneous/machine-learning/guide-mel-frequency-cepstral-coefficients-mfccs/#computing-the-mel-filterbank
    for (int filter = 0; filter < N_MEL_FILTERS; filter++)
    {
        output_buffer[filter] = 0;
        // Work out how wide the band of the triangular filter is, note that 
        // it's asymetrical due to the log distribution of the filter centers.
        // The filters are designed to overlap such that one filter reaches full 
        // attenuation at the center of the next filter. 
        //   |      /\    /\    /\
        // G |     / ¦\  / ¦\  / ¦\
        // a |    /  ¦ \/  ¦ \/  ¦ \
        // i |   /   ¦ /\  ¦ /\  ¦  \
        // n |  /    ¦/  \ ¦/  \ ¦   \
        //   | /     /    \/    \¦    \ ...
        //   ========¦=====¦=====¦========> Freq (log)
        //  filter:  A      B    C    ...
        // 
        // Because the filter_centers array contains 28 values (one at either 
        // end to help with interpolation) we always index at 
        // filter_centers[filter + 1] to get the relevant filter center.

        float filter_dist_low = 1.0f / max(filter_centers[filter + 1] - filter_centers[filter], 0.01f);
        float filter_dist_high = 1.0f / max(filter_centers[filter + 2] - filter_centers[filter + 1], 0.01f);

        // TODO: I think the range of this for loop can be reduced to filter_centers[filter-1]:filter_centers[filter+1], which might improve performance
        // We only look at the first half of the FFT as the fft is mirrored.
        for (int i = 0; i < FFT_SIZE / 2; i++)
        {
            if (i < filter_centers[filter + 1])
                output_buffer[filter] += fft_buffer[i] * CLAMP((i - filter_centers[filter]) * filter_dist_low, 0, 1);
            else
                output_buffer[filter] += fft_buffer[i] * CLAMP((filter_centers[filter + 2] - i) * filter_dist_high, 0, 1);
        }
    }
}