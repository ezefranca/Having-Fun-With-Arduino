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
// ===                      STORAGE                             ===
// ================================================================


  void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(rdata);
    Wire.endTransmission();
  }

  // WARNING: address is a page address, 6-bit end will wrap around
  // also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
  void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddresspage >> 8)); // MSB
    Wire.write((int)(eeaddresspage & 0xFF)); // LSB
    byte c;
    for ( c = 0; c < length; c++)
      Wire.write(data[c]);
    Wire.endTransmission();
  }

  byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.read();
    return rdata;
  }

  // maybe let's not read more than 30 or 32 bytes at a time!
  void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,length);
    int c = 0;
    for ( c = 0; c < length; c++ )
      if (Wire.available()) buffer[c] = Wire.read();
  }


  void writeEEPROM(String input) {
    
    
    char someData[input.length()];
    int i = 0;
    while (i = 0, i < input.length(), i++) {
      someData[i] = input[i];
    }
    
    i2c_eeprom_write_page(0x50, 0, (byte *)someData, sizeof(someData)); // write to EEPROM 

    delay(10); //add a small delay

    Serial.println("Memory written");
  }

  void readEEPROM() {
    int addr=0; //first address
    byte b = i2c_eeprom_read_byte(0x50, 0); // access the first address from the memory

    while (b!=0) 
    {
      Serial.print((char)b); //print content to serial port
      addr++; //increase address
      b = i2c_eeprom_read_byte(0x50, addr); //access an address from the memory
    }
    Serial.println("fullOutput");

  }

// ================================================================
// ===                      ACCELEROMETER                       ===
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
String worldAccelerationFromBuffer() {
    String returnString = "";
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetAccel(&aa, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
  mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
  returnString += (aaWorld.x);
  returnString += (",");
  returnString += (aaWorld.y);
  returnString += (",");
  returnString += (aaWorld.z);
  returnString += ("|");
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

void setupAccelerometer() {

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

void loopAccelerometer(){

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
        sendToOutput(worldAccelerationFromBuffer());
    }
}

// ================================================================
// ===                     SETUP CONTROLS                       ===
// ================================================================

int ledPin = 13; 
int readData = 5; 
int writeData = 6;
int clearData = 7;

void setupControls(){
  pinMode(readData, INPUT);   
  pinMode(writeData, INPUT);  
  pinMode(clearData, INPUT);     
  pinMode(ledPin, OUTPUT);
}

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

boolean readOutput;
void setup() {
  setupControls();
  Wire.begin();
  Serial.begin(115200);
  if(digitalRead(writeData)) {
     setupAccelerometer();
    flash(2);
  } else if(digitalRead(readData)) {
    readOutput = false;
    flash(3);
  } else if(digitalRead(clearData)) {
    flash(4);
  } else {
    flash(5);
  }
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================
void loop() {
  
  if(digitalRead(writeData)) {
    loopAccelerometer();
  } else if(digitalRead(readData)) {
    if (!readOutput) {
      readEEPROM();
      readOutput = true;
    }
  } else if(digitalRead(clearData)) {
  
  }
}

// ================================================================
// ===                    HELPERS                               ===
// ================================================================

void sendToOutput(String input) {
  //Serial.println(input);
  writeEEPROM(input);
}

void flash(int times) {
   while (times > 0) {
      digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000); 
      digitalWrite(ledPin, LOW);   // turn the LED on (HIGH is the voltage level)
      delay(1000); 
      times--;
   }  
}  

