#ifndef MODEL_LOADER_H 
#define MODEL_LOADER_H

/**
 * This module is responsible for loading the model from flash memory.
 * The model is stored in the form of a binary file in the flash memory.
 * The model is loaded into the memory and then passed to the interpreter.
 * The interpreter is responsible for running the model on the input data.
 * 
 * @authors Civan D., Thomas M.
*/

/**
 * Initialises the filesystem on the external flash used to store the model.
*/
void setupFS ();

/**
 * Saves the model to the external flash storage.
*/
void saveToFlash ();

/**
 * Loads the machine learning model from external flash.
 * 
 * @param size returns the loaded size of the model
 * 
 * @returns a pointer to the loaded model
*/
uint8_t* loadFromFlash(size_t *size);

#endif // MODEL_LOADER_H
