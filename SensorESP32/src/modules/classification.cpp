/**
 * This module contains the neural network inference infrastructure required to
 * classify audio samples processed by the DSP.
 *
 * @authors Mingyang C., Civan D., Thomas M.
 */

#include "TensorFlowLite_ESP32.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "classification.h"
#include "config.h"
#include "debug.h"
#include "model_loader.h"
#include "tf_lite_error_reporter.h"

tflite::MicroInterpreter* interpreter;
static tflite::TFLiteErrorReport micro_error_reporter;
tflite::ErrorReporter *error_reporter = &micro_error_reporter;
const tflite::Model *model;
tflite::AllOpsResolver resolver;
constexpr int tensor_arena_size = 2 * 1024 * 1024; // 1MB this is an experimental value if model is too big, increase this value
uint8_t *tensor_arena;

/**
 * Setup method called by the main task to initialise any resources needed by
 * the classification subsystem.
*/
void setupClassification()
{
    error_reporter = &micro_error_reporter;
    dbg_log_info("  Model is loading from flash...");
    size_t model_size = 0;
    model = ::tflite::GetModel(loadFromFlash(&model_size));
    dbg_log_info("  Model loaded from flash");
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        dbg_log_warning("Model provided is schema version %d not equal to supported version %d.", 
            model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    tensor_arena = (uint8_t *)heap_caps_malloc(tensor_arena_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    interpreter = new tflite::MicroInterpreter(model, resolver, tensor_arena,
                                           tensor_arena_size, error_reporter);
    interpreter->AllocateTensors();

    dbg_log_info("  Tensor arena allocated");
}

/**
 * Invokes the classification engine with a pointer to an MFSC buffer and returns
 * the computed category.
 * 
 * @param mfsc_features a 2D array of MFSC features
 * @param num_frames the number of slices in the MFSC (ie not the number of mel bins)
 * 
 * @returns the computed category as an integer or -1 in the case of an error.
*/
int runClassification(const float mfsc_features[][N_MEL_FILTERS], int num_frames)
{
    // Obtain a pointer to the model's input tensor has 4 dimensions
    TfLiteTensor *input = interpreter->input(0);
    TfLiteTensor *output = interpreter->output(0);

    TfLiteType input_type = input->type;

    dbg_log_info("Input type: %d",input_type);

    //print input shape
    dbg_log_info("Input shape: %d",input->dims->size);
    dbg_log_info("Interpreter %d",interpreter);
    dbg_log_info("Input tensor %d",input);
    dbg_log_info("Running inference");

    // Copy the mfsc_features into the input tensor
    for (int i = 0; i < num_frames; i++)
    {
        for (int j = 0; j < N_MEL_FILTERS; j++)
        {
            input->data.f[i * N_MEL_FILTERS + j] = mfsc_features[i][j];
        }
    }

    dbg_log_info("  Input tensor populated (%d frames)", num_frames);
    
    // Run inference
    TfLiteStatus invoke_status = interpreter->Invoke();

    if (invoke_status != kTfLiteOk)
    {
        dbg_log_warning("Invoke failed");
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
        return -1;
    }
    dbg_log_info("Inference done");

    // Get the output tensor and find the category with the highest score
    
    int num_categories = output->dims->data[1];
    int max_category = -1;
    float max_score = -FLT_MAX;
    for (int i = 0; i < num_categories; i++)
    {
        if (output->data.f[i] > max_score)
        {
            max_score = output->data.f[i];
            max_category = i;
        }
    }

    return max_category;
}
