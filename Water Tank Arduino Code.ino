
/* 
 *  this sketch will read the presssure sensor and then send 
 *  that data to the house arduino by RF. There is also a clock module that ensures
 *  that the the data is sent at the same time every day, even if it is turned off
 *  it will not lose track of time. To save power some transistors have been added to control the 
 *  grounds of the pressure sensor and rf module. The transistors are both turned on with the powerPin.
 *  sketch by caleb jackson
 */


//////////////////////////////////////////////
//GLOBAL VARIABLES
//////////////////////////////////////////////
int pressureSensorPin = A0;
int pressureReading = 0;
int powerPin = A1;// this pin controls the transistor that turns on/off the power
const long millisecondsFor23Hr = 82800000;
#define RXADDR {0xFE, 0x4C, 0xA6, 0xE5} // Address of this device (4 bytes)
#define TXADDR {0x58, 0x6F, 0x2E, 0x10} // Address of device to send to (4 bytes)
#define PACKETPAUSE 1000 // Short Break after sending each data packet

//////////////////////////////////////////////
//LIBRARIES
//////////////////////////////////////////////
#include <nRF905.h>  //Library Author Zak Kemble, Web: http://blog.zakkemble.co.uk/nrf905-avrarduino-librarydriver/
#include <SPI.h>  //SPI Master Library for Arduino
#include <DS1302.h>//Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen, web: http://www.RinkyDinkElectronics.com/

// Init the DS1302
DS1302 rtc(4,5,6);//this is the pins - reset, data, CLK
// Init a Time-data structure
Time t;

//////////////////////////////////////////////
//SETUP
//////////////////////////////////////////////
void setup()
{
  ///////clock module setup
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
  //!!!!!!!!!!this will have to be uploaded the first time but then will have to be removed and the sketch will have -
  //!!!!!!!!! to be uploaded again otherwise the date will change every time the arduino resets/turnsoff
  //rtc.setDOW(SUNDAY);        // Set Day-of-Week to FRIDAY
  //rtc.setTime(17,36,00);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(21, 7, 2019);   // Set the date
 
  ///////setup pressure sensor
  pinMode(pressureSensorPin,INPUT);
  
  /////// Start serial comunication
  Serial.begin(9600); 
  Serial.println(F("Transmitter started"));

  /////// setup sundry
  pinMode(powerPin,OUTPUT);
  digitalWrite(powerPin,LOW);// make sure that the power is off to start with
}

/////////////////////////////////////////////
//LOOP
/////////////////////////////////////////////
void loop()
{
  //get data from clock module
  t = rtc.getTime();
  
  if(t.hour == 18 && t.min == 0)
  {
    digitalWrite(powerPin,HIGH);//provide power to the rf transmitter and the pressure sensor
    delay(10000);//turn on the power for 10 sec to warm up
    setUpRf();//set up the transciever, it has just been turned on
    delay(10000);//give some time for them to warm up
    //we send the depth 30 times incase a few of the transmissions dont get through,
    //the receiver sketch will take the first one it sees and then ignores the rest
    for(int i = 1; i < 30; i++)
    {
      pressureReading = analogRead(pressureSensorPin);//read the pressure sensor
      delay(50);//gives a little time to think, there is no hurry
      int data[1] = {pressureReading};// the stuff that is sent needs to be in a array, its just what the library wants
      Serial.println(data[0]);//debugging
      nRF905_setData(data, sizeof(data));//sets/loads the data that it is goind to send
    
      // Send data array payload (send fails if other transmissions are going on, keep trying until success)
      while(!nRF905_send())
      {
        //keep trying to send forever, once it has sent nRF905_send() is true and we exit thr loop
      }
      Serial.println("data sent");
      delay(PACKETPAUSE);
    }
    digitalWrite(powerPin,LOW);//turn off the pressure sensor and rf to save battery
    delay(millisecondsFor23Hr);// this will delay the arduino 23hr hours, might as well to prevent it reading the rtc so often.
    //if the arduino turns off during this time it will stop the 23hr delay and check the time every 15s because the sketch restarts
  }
  delay(15000);// 15 seconds between readings, dont want to be running too fast, wastes power
}
///////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////
void setUpRf()
{
  ///////setup rf
  nRF905_init();// Power up nRF905 and initialize 
  byte incoming[] = RXADDR;// Send this device address to nRF905
  nRF905_setRXAddress(incoming);// set the address
  byte outgoing[] = TXADDR;// Send remote device address to nRF905
  nRF905_setTXAddress(outgoing);//set the address
  nRF905_receive();// Put into receive mode
}