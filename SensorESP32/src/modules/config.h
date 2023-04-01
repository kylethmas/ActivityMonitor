#ifndef CONFIG_H
#define CONFIG_H

/**
 * Configuration file storing all the compile time constants used to parametrise
 * the application.
 * 
 * @authors Thomas M.
*/

// Work out what dev kit we're using to determine pin-mapping/hardware available
// In the future board platforms might be added if a custom board is spun up.
#if WIFI_Kit_32
// Due to memory limitations, this platform is no longer properly supported.
#define PLATFORM_HELTEC_WIFIKIT_32
#else
#define PLATFORM_ESP_LYRA
#endif

// This enables a special mode where the application uploads the machine learning 
// model to the external flash of the esp 32. In this mode the application only
// uploads the model and then hangs until it's reset. 
// Uploading the model only needs to be done once on a device (or whenever the
// model is updated). Make sure to disable this to actually run inferrence.
//#define SAVE_MODEL_TO_FS


///////////////////////////////////////
//// COMMUNICATION SUBSYSTEM CONSTS
// Network credentials
#define WIFI_SSID "Test"
#define WIFI_PASSWORD "Test4312"
// The URL of the API used to receive notifications from the device
// Change to localhost in case you want to use the local server
#define API_SERVER_URL "http://ashraf12.pythonanywhere.com/receive/"
// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
#define API_KEY "tPmAT5Ab3j7F9"
#define API_SENSOR_NAME "device2"

///////////////////////////////////////
//// CLASSIFICATION SUBSYSTEM CONSTS


///////////////////////////////////////
//// AUDIO SUBSYSTEM CONSTS
// I2S
#define AUDIO_SAMPLE_RATE 16000
#ifdef PLATFORM_ESP_LYRA
#define I2S_SAMPLE_RATE AUDIO_SAMPLE_RATE
#else
#define I2S_SAMPLE_RATE (AUDIO_SAMPLE_RATE/2) // Max sampling frequency = 16.000 kHz; This number seems to be a lie, I think it's doubled internally
#endif
#define ADC_INPUT (ADC1_CHANNEL_4) // pin 32
#define I2S_DMA_BUF_LEN (256)
#ifdef PLATFORM_ESP_LYRA
#define AUDIO_BIT_DEPTH 16
#define AUDIO_DATA_SIGNED
#else
#define AUDIO_BIT_DEPTH 12
#endif


///////////////////////////////////////
//// DSP CONSTS
// Defines how loud (in peak dB, where 0dB is the loudest sound the microphone can pick up) a sound 
// needs to be for it to be further analysed by the classification system
#define AUDIO_AMPLITUDE_THRESHOLD -60.0
#define SAMPLE_DURATION_SECONDS 3
// FFT
// For a radix 4 FFT this can only be one of: 64, 256, 1024
#define FFT_SIZE 256
#define FFT_INPUT_SCALE 1.0f
#define FFT_AVG_WEIGHT 0.9f
// The lowest frequency bin should be at around 62.5Hz, so to remove dc offset 
// and low frequency noise, we'll start at the next frequency, which is at 125Hz
#define MEL_FILTER_LOW_FREQ 125
#define MEL_FILTER_HIGH_FREQ 8000
#define N_MEL_FILTERS 26


///////////////////////////////////////
//// DEBUGGING CONSTS
// Serial Logging
#define DEBUG_SERIAL_LOGGING

// Uncomment whichever level of logging you want
#define DEBUG_VERBOSE
// #define DEBUG_INFO
// #define DEBUG_WARNING
// #define DEBUG_FATAL

// Log all audio samples over serial, this is REALLLY slow and results in missed 
// samples
//#define DEBUG_LOG_ALL_SAMPLES

// Log all recorded mfscs over serial
// #define DEBUG_DUMP_MFSC

// Replace the sampled microphone data with a tone sweep
// #define DEBUG_USE_TEST_TONE_SWEEP

// Display
#ifdef PLATFORM_HELTEC_WIFIKIT_32
#define DEBUG_DISPLAY_ENABLE
#define DEBUG_DISPLAY_AUDIO
#define DEBUG_DISPLAY_FFT

#define DEBUG_DISPLAY_WIDTH 128
#define DEBUG_DISPLAY_HEIGHT 64
#define DEBUG_SCREEN_ADDRESS 0x3c
#define DEBUG_DISPLAY_HARDWARE_PARAMS DEBUG_DISPLAY_WIDTH, DEBUG_DISPLAY_HEIGHT, &Wire, RST_OLED

#define CPU_FREQ_KHZ 240000
#endif

#endif // CONFIG_H