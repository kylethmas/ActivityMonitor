/**
 * This module is responsible for loading the model from flash memory.
 * The model is stored in the form of a binary file in the flash memory.
 * The model is loaded into the memory and then passed to the interpreter.
 * The interpreter is responsible for running the model on the input data.
 * 
 * @authors Civan D., Thomas M.
*/

#include "config.h"
#include "debug.h"
#include "FS.h"
#include <LittleFS.h>

#define FORMAT_SPIFFS_IF_FAILED true

/**
 * Initialises the filesystem on the external flash used to store the model.
*/
void setupFS()
{
    dbg_log_info("Mounting LittleFS");
    int e = LittleFS.begin(FORMAT_SPIFFS_IF_FAILED);
    if (!e)
    {
        dbg_log_warning("LittleFS Mount Failed");
        dbg_log_warning("Error code: %d", e);
        return;
    }
}

#ifdef SAVE_MODEL_TO_FS
// This is the model data. It is huge so don't compile if you are not changing it. Just save once in flash and use from there.
// Otherwise it will take forever to compile.
#include "model_data.h"

/**
 * Saves the model to the external flash storage.
*/
void saveToFlash()
{
    File f = LittleFS.open("/model.bin", "w");
    if (!f)
    {
        dbg_log_warning("Failed to open file for writing");
        return;
    }
    f.write((uint8_t *)model_tflite, model_tflite_len);
    f.close();
}
#endif

/**
 * Loads the machine learning model from external flash.
 * 
 * @param size returns the loaded size of the model
 * 
 * @returns a pointer to the loaded model
*/
uint8_t *loadFromFlash(size_t *size)
{
    File f = LittleFS.open("/model.bin", "r");
    if (!f)
    {
        dbg_log_warning("Failed to open file for reading");
        return NULL;
    }
    *size = f.size();
    // heap_caps_malloc is used to allocate memory in the external RAM
    uint8_t *data = (uint8_t *)heap_caps_malloc(*size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    // Check that the malloc succeeded
    if (data == NULL)
    {
        dbg_log_warning("Failed to allocate memory for model");
        return NULL;
    }
    f.read(data, *size);
    f.close();

    return data;
}
