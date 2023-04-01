#include "TensorFlowLite_ESP32.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tf_lite_error_reporter.h"
#include "debug.h"

namespace tflite
{
  int TFLiteErrorReport::Report(const char* format, va_list args) {
    dbg_log_warning(format, args);
    return 0;
  }

  int TFLiteErrorReport::ReportError(void *, const char *format, ...) {
    dbg_log_warning(format);
    return 0;
  }
}
