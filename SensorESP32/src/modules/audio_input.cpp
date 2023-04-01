/**
 * This module handles the audio input and buffering from I2S.
 * 
 * @authors Thomas M., Civan D.
*/

#include <Arduino.h>
#include "audio_input.h"
#include "config.h"
#include "debug.h"
#include <driver/i2s.h>

#ifdef PLATFORM_ESP_LYRA
#include <ES8388.h>
ES8388 es8388(18,23,400000);
#endif

// The 4 high bits are the channel, and the data is inverted
size_t bytes_read;

#ifdef PLATFORM_ESP_LYRA
void configureES8388(){
    dbg_log_info("Configuring ES8388");
    
    if (!es8388.init()) Serial.println("Init Fail");
    es8388.inputSelect(IN1);
    es8388.setInputGain(0);
}
#endif

/**
 * Setup method called by the main task to initialise any resources needed by
 * the audio subsystem.
*/
void setupAudioInput(){
    #ifdef PLATFORM_ESP_LYRA
    // Configure the audio codec chip
    configureES8388();

    // Configure the I2S peripheral
    i2s_config_t i2s_read_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 2,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

	i2s_pin_config_t i2s_read_pin_config = {
		.bck_io_num = GPIO_NUM_5,
		.ws_io_num = GPIO_NUM_25,
		.data_out_num = GPIO_NUM_26,
		.data_in_num = GPIO_NUM_35
	};

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
    WRITE_PERI_REG(PIN_CTRL, 0xFFF0);

    dbg_log_info("Attempting to setup I2S ES8388 with sampling frequency %d Hz", I2S_SAMPLE_RATE);

    if (ESP_OK != i2s_driver_install(I2S_NUM_0, &i2s_read_config, 0, NULL))
    {
        dbg_log_fatal("Error installing I2S. Halt!");
        while (1)
            ;
    }
    
    if (ESP_OK != i2s_set_pin(I2S_NUM_0, &i2s_read_pin_config))
    {
        dbg_log_fatal("Error setting I2S pins. Halt!");
        while (1)
            ;
    }
    #else
    // Heltec Wifi Kit 32 I2S Config
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
        .sample_rate = I2S_SAMPLE_RATE,               // The format of the signal using ADC_BUILT_IN
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    dbg_log_info("Attempting to setup I2S ADC with sampling frequency %d Hz", I2S_SAMPLE_RATE);

    if (ESP_OK != i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL))
    {
        dbg_log_fatal("Error installing I2S. Halt!");
        while (1)
            ;
    }
    if (ESP_OK != i2s_set_adc_mode(ADC_UNIT_1, ADC_INPUT))
    {
        dbg_log_fatal("Error setting up ADC. Halt!");
        while (1)
            ;
    }
    if (ESP_OK != adc1_config_channel_atten(ADC_INPUT, ADC_ATTEN_DB_2_5))
    {
        dbg_log_fatal("Error setting up ADC attenuation. Halt!");
        while (1)
            ;
    }
    if (ESP_OK != i2s_adc_enable(I2S_NUM_0))
    {
        dbg_log_fatal("Error enabling ADC. Halt!");
        while (1)
            ;
    }
    #endif

    dbg_log_info("I2S setup ok");
}

/**
 * Reads the microphone data into a buffer and returns a pointer to the start of
 * the buffer. This methods blocks until the buffer is full, but the dma should
 * continue sampling the microphone after this method has finished.
 * 
 * @param buffer a pointer to the buffer containing the sampled audio
 * @returns how may samples were read into the buffer, in rare cases 
 *          this can be smaller than the size of the buffer
*/
int sampleMicrophone(uint16_t* buffer)
{
    // Blocks until the entire buffer is read or until the tick timeout is reached.
    // This *should* with a task delay, so other RTOS tasks should still be able to run in the background.
    i2s_read(I2S_NUM_0, buffer, I2S_DMA_BUF_LEN * sizeof(uint16_t), &bytes_read, portMAX_DELAY);
    return bytes_read >> 1;
}