#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UUIDs
#define SERVICE_UUID        "a45cc169-be38-42d5-a22b-f2ebbf442344"
#define MOTOR_CHARACTERISTIC_UUID "a45cc169-be38-42d5-a22b-f2ebbf442345"

// Motor control pins
#define AIN1_PIN 22
#define AIN2_PIN 21
#define BIN1_PIN 19
#define BIN2_PIN 18

BLEServer* pServer = NULL;
BLECharacteristic* pMotorCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

int leftMotorValue = 128;
int rightMotorValue = 128;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void controlMotor(int pin1, int pin2, int motorValue) {
  if (motorValue < 128) {
    analogWrite(pin1, 255 - map(motorValue, 0, 127, 0, 255));
    analogWrite(pin2, 0);
  } else if (motorValue > 128) {
    analogWrite(pin1, 0);
    analogWrite(pin2, map(motorValue, 129, 255, 0, 255));
  } else {
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
  }
}

class MotorCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() == 2) {
      leftMotorValue = static_cast<int>(value[0]);
      rightMotorValue = static_cast<int>(value[1]);
      controlMotor(AIN1_PIN, AIN2_PIN, leftMotorValue);
      controlMotor(BIN1_PIN, BIN2_PIN, rightMotorValue);
    }
  }
};



void setup() {
  Serial.begin(115200);

  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(BIN1_PIN, OUTPUT);
  pinMode(BIN2_PIN, OUTPUT);

  BLEDevice::init("SumoBot1");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pMotorCharacteristic = pService->createCharacteristic(
                      MOTOR_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pMotorCharacteristic->setCallbacks(new MotorCharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();

  Serial.println("Waiting for client connection...");
}

void loop() {
  if (deviceConnected) {
    // Additional tasks if needed
  }

  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}
