#include <EdgeNeuron.h>
#include <EdgeMath.h>
#include <Wire.h>
#include <MPU6050.h>
#include "model.h"

MPU6050 mpu;

EdgeMath edgemath;
constexpr int kTensorArenaSize = 50000;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

String class_array[] = {"Standing still","walking", "jumping", "squating"};

constexpr int kOutputSize = 4;      // Output tensor size
float output_data[4];
float input_data[30];

void setup() {
  Serial.begin(115200);

  // Initialize the EdgeNeuron model
  if (!initializeModel(model_tflite, tensor_arena, kTensorArenaSize)) {
    Serial.println("[Consentium ERROR] Model initialization failed!");
    while (true); // Halt execution
  }
  Serial.println("[Consentium INFO] Model initialized successfully!");
// Gunakan GPIO 8 (SDA) dan GPIO 9 (SCL)
  Wire.begin(8, 9); 

  Serial.begin(115200);
  Serial.println("Initializing MPU6050...");

  mpu.initialize();

  // Set range akselerometer ke ±2g
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful.");
  } else {
    Serial.println("MPU6050 connection failed.");
    while (1);

  }

}

void loop() {

for (int i = 0; i < 10; i++) {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

 mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
float accelX =   (ax- 992) / 16384.0;
  float accelY = (ay-826) / 16384.0;
  float accelZ = az / 16384.0;

  Serial.print("Accel (g): X=");
  Serial.print(accelX, 3);
  Serial.print(" Y=");
  Serial.print(accelY, 3);
  Serial.print(" Z=");
  Serial.println(accelZ, 3);

  // Konversi akselerasi dari raw ke g
    input_data[i * 3 + 0] = accelX;
    input_data[i * 3 + 1] = accelY;
    input_data[i * 3 + 2] = accelZ;

    delay(200); // Sampling rate ~20Hz
    
  }

for (int i = 0; i < 30; i++) {
  setModelInput(input_data[i], i);  // Masukkan satu per satu
}

  // Perform inference
  Serial.println("[Consentium INFO] Running inference...");
  unsigned long start_time = millis();
  if (!runModelInference()) {
    Serial.println("[Consentium ERROR] Inference failed!");
    return;
  }
  unsigned long inference_time = millis() - start_time;
  Serial.print("[Consentium INFO] Inference complete. Time taken: ");
  Serial.print(inference_time);
  Serial.println(" ms");

 // Retrieve output data
  for (int i = 0; i < 4; i++) {
    output_data[i] = getModelOutput(i);
  }

  // Determine the predicted class
  int class_pos = edgemath.argmax(output_data, kOutputSize);
  Serial.println("[Consentium INFO] Predicted Class: " + class_array[class_pos]);

  // Log class probabilities
  Serial.println("[Consentium INFO] Class probabilities:");
  for (int i = 0; i < kOutputSize; i++) {
    Serial.print("  ");
    Serial.print(class_array[i]);
    Serial.print(": ");
    Serial.println(output_data[i]);
  }

}
