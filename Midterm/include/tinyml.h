#pragma once
#ifndef __TINYML_H__
#define __TINYML_H__

#include <Arduino.h>
#include "dht_anomaly_model.h"
#include "config.h"
#include "leds.h"
#include "temp_humi.h"

// TensorFlow Lite Micro headers
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

void setupTinyML();
void TaskTinyML(void *pvParameters);

#endif
