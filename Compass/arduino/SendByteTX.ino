
#include <Wire.h> //I2C Arduino Library
#include <math.h>

#define address 0b0001101 //0011110b, I2C 7bit address of HMC5883

void setup(){
  //Initialize Serial and I2C communications
  Serial.begin(2400);
  Wire.begin();
  
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x0B); //select mode register
  Wire.write(0x01); //continuous measurement mode
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(0x09); //select mode register
  Wire.write(0b00010001); //continuous measurement mode
  Wire.endTransmission();
}

void loop(){
  
  int x,y,z; //triple axis data
  int xLSB, xMSB, yLSB, yMSB, zLSB, zMSB;
  int Angle;

  //Tell the HMC5883 where to begin reading data
  Wire.beginTransmission(address);
  Wire.write(0x00); //select register 3, X MSB register
  Wire.endTransmission();
  
 
 //Read data from each axis, 2 registers per axis
  Wire.requestFrom(address, 6);
  if(6<=Wire.available()){
    xLSB = Wire.read(); //X lsb
    xMSB = Wire.read();
    yLSB = Wire.read(); //X lsb
    yMSB = Wire.read();
    zLSB = Wire.read(); //X lsb
    zMSB = Wire.read();
  }

  //Angle= atan2((double)y,(double)x)*(180.0/3.14159265)+180.0;
  
  //Print out values of each axis
  Serial.write("x");
  Serial.write(xLSB);
  Serial.write(xMSB);
  Serial.write("y");
  Serial.write(yLSB);
  Serial.write(yMSB);
  Serial.write("z");
  Serial.write(zLSB);
  Serial.write(zMSB);

  //if (x==0 && y==0 && z==0){
  if (xLSB==0 && xMSB==0 && yLSB==0 && yMSB==0 && zLSB==0 && zMSB==0){
    // restart
      Wire.beginTransmission(address);
      Wire.write(0x0A); //select mode register
      Wire.write(0x80); //continuous measurement mode
      Wire.endTransmission();

        Wire.beginTransmission(address); //open communication with HMC5883
      Wire.write(0x0B); //select mode register
      Wire.write(0x01); //continuous measurement mode
      Wire.endTransmission();
      Wire.beginTransmission(address);
      Wire.write(0x09); //select mode register
      Wire.write(0b00010001); //continuous measurement mode
      Wire.endTransmission();
  }
  
  delay(250);
}

