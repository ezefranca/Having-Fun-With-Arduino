#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };



// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}



// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
    Wire.begin();
    Serial.begin(115200);
    while (!Serial); 
    mpu.initialize();
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    devStatus = mpu.dmpInitialize();
    
    if (devStatus == 0) {
        mpu.setDMPEnabled(true);
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();
        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
}


// ================================================================
// ===                    HELPERS                               ===
// ================================================================

String jsonQuaternionFromBuffer() {
  
  String returnString = "quaternion: {";
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  returnString += "w:";
  returnString += q.w;
  returnString += ", x:";
  returnString += q.x;
  returnString += ", y:";
  returnString += q.y;
  returnString += ", z:";
  returnString += q.z;
  returnString += "}";
  return returnString;
}

String jsonEulerFromBuffer() {
  String returnString = "euler: [";
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetEuler(euler, &q);
  returnString += (euler[0] * 180/M_PI);
  returnString += (",");
  returnString += (euler[1] * 180/M_PI);
  returnString += (",");
  returnString += (euler[2] * 180/M_PI);
  returnString += ("]");
  return returnString; 
}

String jsonYawPitchRollFromBuffer() {
  String returnString = "yawPitchRoll: {";
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
  returnString += ("y: ");
  returnString += (ypr[0] * 180/M_PI);
  returnString += (", p:");
  returnString += (ypr[1] * 180/M_PI);
  returnString += (", r:");
  returnString += (ypr[2] * 180/M_PI);
  returnString += ("}");
  return returnString;
}

String jsonAccelerationFromBuffer() {

  String returnString = "acceleration: {";
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetAccel(&aa, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
  returnString += ("x: ");
  returnString += (aaReal.x);
  returnString += (", y:");
  returnString += (aaReal.y);
  returnString += (", z:");
  returnString += (aaReal.z);
  returnString += ("} ");
  return returnString;
}

String jsonWorldAccelerationFromBuffer() {
  String returnString = "worldAcceleration: {";
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetAccel(&aa, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
  mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
  returnString += ("x: ");
  returnString += (aaWorld.x);
  returnString += (", y:");
  returnString += (aaWorld.y);
  returnString += (", z:");
  returnString += (aaWorld.z);
  returnString += ("} ");
  return returnString;
}

void sendToOutput(String input) {
  Serial.print(input);
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
    if (!dmpReady) return;
    while (!mpuInterrupt && fifoCount < packetSize) {
        // other program behavior stuff here
    }
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();
    fifoCount = mpu.getFIFOCount();
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));
    } else if (mpuIntStatus & 0x02) {
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        fifoCount -= packetSize;
       // Serial.print(jsonQuaternionFromBuffer());
       // Serial.print(jsonEulerFromBuffer());  
       // Serial.print(jsonYawPitchRollFromBuffer());
       // Serial.print(jsonAccelerationFromBuffer());
        sendToOutput(jsonWorldAccelerationFromBuffer());
    }
}



