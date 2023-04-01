/**
 * Entry point for the application.
 */

#include "src/modules/config.h"
#include "src/modules/utils.h"
#include "src/modules/debug.h"
#include "src/modules/audio_input.h"
#include "src/modules/dsp.h"
#include "src/modules/classification.h"
#include "src/modules/communication.h"
#include "src/modules/model_loader.h"

// Setup is called once on-boot automatically
void setup()
{
    // Setup any of the required hardware for debugging purposes
    // This method doesn't do anything if no debugging is enabled in config.h
    setupDebug();

    // Setup the filesystem
    dbg_log_info("Mounting FS...");
    setupFS();

    // When in model uploading mode, upload the model to the external flash
    // and then do nothing else.
#ifdef SAVE_MODEL_TO_FS
    dbg_log_info("Saving model to flash...");
    saveToFlash();
    dbg_log_info("Model saved to flash");

    delay(100);

    // LOADING MODEL FROM FLASH
    size_t size = 0;
    uint8_t *model = loadFromFlash(&size);
    dbg_log_info("Model loaded from flash");
    dbg_log_info("Model size: %d", size);
    while (1){delay(1000);}
#endif

    dbg_displayString(0, 0, "Initialising I2S device...");
    dbg_log_info("Initialising I2S device...");
    dbg_display();

    // Initialize the I2S peripheral
    setupAudioInput();

    dbg_clearDisplay();
    dbg_displayString(0, 0, "Initialising DSP...");
    dbg_log_info("Initialising DSP...");
    dbg_display();

    // Create FFT
    setupDSP();

    dbg_clearDisplay();
    dbg_displayString(0, 0, "Initialising classification subsystem...");
    dbg_log_info("Initialising classification subsystem...");
    dbg_display();

    // Setup the neural network
    setupClassification();
    // Test classification engine
    // dbg_log_info("Test classification engine. Returned: %d", 
    //              runClassification(demo_mfsc(), SAMPLE_DURATION_SECONDS * AUDIO_SAMPLE_RATE * 2 / FFT_SIZE));

    dbg_clearDisplay();
    dbg_displayString(0, 0, "Initialising WiFi...");
    dbg_log_info("Initialising WiFi...");
    dbg_display();

    // Setup WiFi
    setupCommunication();

    dbg_clearDisplay();
}

__attribute__((aligned(16)))
uint16_t buffer_a[I2S_DMA_BUF_LEN] = {0};
__attribute__((aligned(16)))
uint16_t buffer_b[I2S_DMA_BUF_LEN] = {0};
/* Buffer for storing an entire mfsc of a sample.
 * The buffer is a 2D array with the mel filterbank energies on the y axis and time on the x axis.
 */
__attribute__((aligned(16))) 
float mfsc_buffer[SAMPLE_DURATION_SECONDS * AUDIO_SAMPLE_RATE * 2 / FFT_SIZE][N_MEL_FILTERS] = {0};

char server_msg_buff[32] = {0};

// Loop is called repeatedly after setup()
void loop()
{
    // Once the model uploader has run, don't do anything as the model uploader only uploads the model.
#ifdef LOAD_MODEL_TO_SPIFFS
    delay(1000);
    return;
#endif

    // Save power by running at a low clock speed until we find something interesting
    // with WiFi enabled this reduces current usage by 24% (source: https://youtu.be/JFDiqPHw3Vc?t=833)
    setCpuFrequencyMhz(80);

    while (1)
    {
        // Continuously sample the microphone and trigger a full sample and analysis if we find an interesting sample
        // TODO: This initial sampling and triggering could be done with an external peripheral to save power;
        //       the ULP could be used to do this while in light_sleep or deep_sleep, but research would need to be done
        //       to see if this responds fast enough.

        int samplesRead = I2S_DMA_BUF_LEN;
        int dbg_samples_read = sampleMicrophone(buffer_a);
        // Perform an initial analysis to determine if the signal is interesting
        if (dspInitialAnalysis(buffer_a, samplesRead))
        {
            // Do a full sample & analysis
            // Go back to full speed for analysis
            setCpuFrequencyMhz(240);
            dbg_clearDisplay();

            dbg_log_info("Preparing to process samples...\n");

            // Sample an entire sample and compute the MFCC
            int window_index = 0;
            float dbg_freq = 3000;
            int dbg_wind_ind = 0;
#ifdef DEBUG_USE_TEST_TONE_SWEEP
            dbg_fill_sine_wave(buffer_a, dbg_wind_ind, dbg_freq);
#endif
            do
            {
                // Because we window the data we need to process each buffer twice:
                // A \ Window 1     |
                // A |              /
                // A |    \ Window 2
                // A /    |
                // B      |    \ Window 3
                // B      /    |
                // B           |    \ Window 4
                // B           /    |
                // This will all break if samplesRead is not the size of the buffer

                // dbg_log("window index = %d   samplesRead = %d\n", window_index, samplesRead);
                int sampled = sampleMicrophone(buffer_b);
                if (sampled != I2S_DMA_BUF_LEN)
                    dbg_log_warning("Didn't sample enough data read: %d expected %d", sampled, I2S_DMA_BUF_LEN);
                samplesRead += I2S_DMA_BUF_LEN;

#ifdef DEBUG_USE_TEST_TONE_SWEEP
                dbg_fill_sine_wave(buffer_b, ++dbg_wind_ind, dbg_freq += 200);
#endif

                dspProcessSamples(buffer_a, buffer_b, I2S_DMA_BUF_LEN, DSP_WINDOW::A, mfsc_buffer[window_index++]);
                dspProcessSamples(buffer_a, buffer_b, I2S_DMA_BUF_LEN, DSP_WINDOW::B, mfsc_buffer[window_index++]);

                sampled = sampleMicrophone(buffer_a);
                if (sampled != I2S_DMA_BUF_LEN)
                    dbg_log_warning("Didn't sample enough data read: %d expected %d", sampled, I2S_DMA_BUF_LEN);
                samplesRead += I2S_DMA_BUF_LEN;

#ifdef DEBUG_USE_TEST_TONE_SWEEP
                dbg_fill_sine_wave(buffer_a, ++dbg_wind_ind, dbg_freq += 200);
#endif

                dspProcessSamples(buffer_a, buffer_b, I2S_DMA_BUF_LEN, DSP_WINDOW::C, mfsc_buffer[window_index++]);
                dspProcessSamples(buffer_a, buffer_b, I2S_DMA_BUF_LEN, DSP_WINDOW::D, mfsc_buffer[window_index++]);
            } while (samplesRead <= SAMPLE_DURATION_SECONDS * AUDIO_SAMPLE_RATE);

            dbg_log_info("Finished processing!");

            
// Dump the MFSC over serial
#ifdef DEBUG_DUMP_MFSC
            // Add a short prolog and epilog to identify the start and end of dumps.
            // serial 0x7E, 0x00, 0x7E
            Serial.write(0x7E);
            Serial.write(0x00);
            Serial.write(0x7E);

            dbg_send_buffer(reinterpret_cast<float *>(mfsc_buffer), sizeof(mfsc_buffer));

            // serial 0x7E, 0x01, 0x7E
            Serial.write(0x7E);
            Serial.write(0x00);
            Serial.write(0x7E);
#endif

            // Now run the classification algorithm on it
            int category = runClassification(mfsc_buffer, SAMPLE_DURATION_SECONDS * AUDIO_SAMPLE_RATE * 2 / FFT_SIZE);
            dbg_log_info("Sample classified as: %d", category);

            // If needed queue up a message for the communication subsystem
            sprintf(server_msg_buff, "category_%d", category);
            sendData(server_msg_buff);

            dbg_display();
            // Return to low speed
            setCpuFrequencyMhz(80);
        }
    }
}
