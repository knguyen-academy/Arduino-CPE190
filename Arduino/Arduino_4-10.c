//Created by: Khoi Nguyen
//Revised: 04-13-2017

#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#define signalPin 5
#define collectLed 8
#define sendLed 13
#define FILE_NO 2
#define rxPin 0
#define txPin 1
SoftwareSerial BTSetup (rxPin, txPin);

//Global variables

///SD card varibles///
int time_index=0;
int signalState=0;  //use to signal Sending State
File fp[FILE_NO];     // Create 7 files in SD
char filename[10];  // dynamic files name

int fileIndex = 0; //increment and open new file once the file is too large

///Gyrop variables///
long accelX, accelY, accelZ; // Accel register values 
float gForceX, gForceY, gForceZ; // Accel in gforces 
long gyroX, gyroY, gyroZ; // Gyro register values 
float rotX, rotY, rotZ; // Gyro in angular velocity 


void setup() 
{
    pinMode(signalPin, INPUT);  // Pin to signal the Send State
    pinMode(collectLed, OUTPUT); //collectLed pin
    pinMode(sendLed, OUTPUT); //sendLed pin
    BTSetup.begin(115200);
    
    //Create 8 new file on Set up, otherwise it will be halted !!!
    for (int i = 0; i <FILE_NO; i++)
  {
    sprintf(filename, "test%d_%d.txt", i, fileIndex);
    fp[i] = SD.open(filename,O_CREAT);
  }
  
  closeFile();
  
  //while (!Serial){;} // wait for serial port to connect, for testing
  
  // Mounting SD card 
    if (!SD.begin(4)) 
  {
    Serial.println("initialization failed!");
    return;
  }

  //Gyro setup///
  Wire.begin();
  setupMPU();

}

void loop() 
{
  
    char buff[50];       // buff for sprintf format
    int intVoltage[6];  // voltage in int  
    float voltage[6]; // voltage in float
    char str_voltage[6]; // convert float for sprintf
    int A[] = {A1, A2, A3, A4, A5, A7};    // analog pins
   

  digitalWrite(collectLed, HIGH);
  digitalWrite(sendLed, LOW);
  
  
  //////////////////////*****COLLECT STATE****** /////////////// 
  
     // open the file for write.
    for (int i = 0; i <FILE_NO; i++)
  {
    sprintf(filename, "test%d_%d.txt", i, fileIndex);
    fp[i] = SD.open(filename,FILE_WRITE);
  }

  
  // if the files opened okay, write to it:
  if (fp[0] && fp[1] ) 
  {
    //Read data from 4 analogs Pin A1-A4
    for (int i = 0; i < 6; i++)
    {
      intVoltage[i] = analogRead(A[i]);     //read from A1-A4
      voltage[i] = intVoltage[i] * (3.3 / 1024.0); // Convert the analog reading 
      dtostrf(voltage[i], 4, 5, str_voltage);    //convert float voltage to string
      //sprintf(buff,"%d , %s\n",time_index, intVoltage[i]);  //format string to buff, to print to file
      //fp[i].print(buff);              //Write to file in SD
    fp[0].print("File");fp[0].print(i); fp[0].print(", ");
      fp[0].print(time_index); fp[0].print(", "); fp[0].print(str_voltage); fp[0].print("\n");
    }//end for
    
    time_index++;

   ////Read Gryro and print to File4
  //recordGyroRegisters();
  //fp[4].print("X=");    
  //fp[4].print(rotX); fp[4].print(", ");
  //fp[4].print(" Y=");
  //fp[4].print(rotY); fp[4].print(", ");
  //fp[4].print(" Z=");
  //fp[4].print(rotZ);
  //fp[4].print("\n");
  
  //Read Accel and print to File5
  recordAccelRegisters();

  fp[1].print("File6"); fp[1].print(", ");
  fp[1].print(gForceX); fp[1].print(", ");
  fp[1].print(gForceY);fp[1].print(", ");
  fp[1].print(gForceZ);
  fp[1].print("\n");
    
  
    // Close all files every write
    if(fp[0].size() > 65536)
    {
      fileIndex++;
      closeFile();
      for (int i = 0; i <FILE_NO; i++)
      {
        sprintf(filename, "test%d_%d.txt", i, fileIndex);
        Serial.print("File exceeded max size, opening new file: ");
        Serial.println(filename);
        fp[i] = SD.open(filename,O_CREAT);
      }
    }
    closeFile();
    //delay(1000);
   
  } 
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
  //delay(100);
  
  //////////////////////*****SENDING STATE****** ///////////////
  signalState = digitalRead(signalPin);
  if(signalState==1)  //go to send state if signal =1
  {
    //closeFile();
    digitalWrite(collectLed, HIGH);
    digitalWrite(sendLed, HIGH);
    delay(55000); //use this for sync with the Pi (restart)
  
    digitalWrite(collectLed, LOW);
    // Open each file and print to terminal/Bluetooth
    for (int i = 0; i <FILE_NO; i++)
      {
        for(int j = 0; j <= fileIndex ; j++)
        {
          int d=i+1;
          sprintf(filename, "test%d_%d.txt", i, j);
          fp[i] = SD.open(filename,FILE_READ);
          //sprintf(filename, "file%d",d);
          //BTSetup.print(filename);
          //BTSetup.print("\n");
          while (fp[i].available()) 
          {
            BTSetup.write(fp[i].read());
          }
          fp[i].close();
        }
      }
      
    BTSetup.println("Done");
    BTSetup.println("Done");
    BTSetup.println("Done");BTSetup.println("Done");
    //Remove files after read
    for (int i = 0; i <FILE_NO; i++)
      {
        for(int j = 0; j <=fileIndex; j++)
        {
          sprintf(filename, "test%d_%d.txt", i, j);
          SD.remove(filename);
        }
      }
    
    time_index = 0; //reset time_index
    //delay(2000);

    //After upload, signal pin ==1, or the shoes is on the docking, it will stay in this state, not collecting
    while(1)
    {
      digitalWrite(sendLed, HIGH);
      delay(100);
      digitalWrite(sendLed, LOW);
      delay(100);
      signalState = digitalRead(signalPin);
      if(signalState ==0)   //if signal =0, or start using the shoe
      {
        delay(5000);
        break;      //go back to collect state 
      }//end if 
    }//end while(1)
  }//end if-SendState

}//end forever loop

void setupMPU(){
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001)
  Wire.write(0x6B); //Accessing the register 6B for power management 
  Wire.write(0b00000000); //Setting SLEEP register to 0 to take off of sleep mode
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B  for gyro config
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg/s will cap movement up to 250 deg/s 
  Wire.endTransmission(); 
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission();
  }

  void recordAccelRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processAccelData();
  }

/* Divide the raw accel values by the LSB ofr the +/- 2g */
void processAccelData(){
  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0;}

/* Record raw gyro values from registers of the gyro*/
void recordGyroRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();}
  
/* Process the gyro data by dviding by lsb for 250 deg/sec limit */
void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;
}

void closeFile()
{
  for(int i =0; i<FILE_NO; i++)
  fp[i].close();
 
}