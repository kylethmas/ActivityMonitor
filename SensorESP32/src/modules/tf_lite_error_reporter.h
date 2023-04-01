#ifndef TF_LITE_ERROR_REPORTER_H
#define TF_LITE_ERROR_REPORTER_H

/**
 * 
*/

#include "TensorFlowLite_ESP32.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

namespace tflite
{
    class TFLiteErrorReport : public ErrorReporter
    {
        public:
            virtual int Report(const char *format, va_list args);
            int Report(const char *format, ...);
            int ReportError(void *, const char *format, ...);
    };
}

#endif // TF_LITE_ERROR_REPORTER_H