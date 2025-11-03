#include "tinyml.h"

// ===== TensorFlow Lite globals =====
namespace {
  tflite::MicroErrorReporter micro_error_reporter;
  tflite::AllOpsResolver resolver;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;

  constexpr int kTensorArenaSize = 25 * 1024;   // increased arena
  static uint8_t tensor_arena[kTensorArenaSize];
}

// ===== Initialize model =====
void setupTinyML() {
  Serial.println("[TinyML] Initializing TensorFlow Lite model...");

  model = tflite::GetModel(dht_anomaly_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("[TinyML] Schema mismatch: model %d vs TFLM %d\n",
                  model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  interpreter = new tflite::MicroInterpreter(model, resolver,
                                             tensor_arena, kTensorArenaSize,
                                             &micro_error_reporter);

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("[TinyML] AllocateTensors FAILED!");
    return;
  }

  input  = interpreter->input(0);
  output = interpreter->output(0);
  Serial.println("[TinyML] Model ready.");
}

// ===== Main inference task =====
void TaskTinyML(void *pvParameters) {
  const TickType_t period = pdMS_TO_TICKS(1000);
  TickType_t lastWake = xTaskGetTickCount();

  while (true) {
    // --- Read sensor data ---
    float t = getTemperature();
    float h = getHumidity();
    Serial.printf("[TinyML] Temp=%.2f°C, Humi=%.2f%%\n", t, h);

    // --- Normalize input ---
    input->data.f[0] = t / 50.0f;
    input->data.f[1] = h / 100.0f;

    // --- Inference ---
    if (interpreter->Invoke() != kTfLiteOk) {
      Serial.println("[TinyML] Inference failed!");
      vTaskDelayUntil(&lastWake, period);
      continue;
    }

    float score = output->data.f[0];  // anomaly probability
    Serial.printf("[TinyML] Score=%.3f\n", score);

    // --- LED behavior ---
    if (score < 0.35) {          // Normal
      Neo_setRGB1(0, 255, 0);    // Green
      Serial.println("[TinyML] Normal condition");
    } 
    else if (score < 0.6) {      // Warning
      Neo_setRGB1(255, 255, 0);  // Yellow
      Serial.println("[TinyML] Warning condition");
    } 
    else {                       // Anomaly
      Neo_setRGB1(255, 0, 0);    // Red
      Serial.println("[TinyML] Anomaly detected!");
    }

    vTaskDelayUntil(&lastWake, period);
  }
}
