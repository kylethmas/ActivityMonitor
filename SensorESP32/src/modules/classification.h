#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

/**
 * This module contains the neural network inference infrastructure required to 
 * classify audio samples processed by the DSP.
 * 
 * @authors Mingyang C., Civan D., Thomas M.
*/

#include "config.h"

/**
 * Setup method called by the main task to initialise any resources needed by
 * the classification subsystem.
*/
void setupClassification();

/**
 * Invokes the classification engine with a pointer to an MFSC buffer and returns
 * the computed category.
 * 
 * @param mfsc_features a 2D array of MFSC features
 * @param num_frames the number of slices in the MFSC (ie not the number of mel bins)
 * 
 * @returns the computed category as an integer or -1 in the case of an error.
*/
int runClassification(const float mfsc_features[][N_MEL_FILTERS], int num_frames);

#endif // CLASSIFICATION_H