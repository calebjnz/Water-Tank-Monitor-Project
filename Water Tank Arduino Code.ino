
/* this sketch will send the presssure data from the water tank to the house arduino so the user can read it.
 *  
 *  
 *  
 *  
 * DO NOT PUT THUMB OVER LIGHT SENSOR, IT DOES WIERD THINGS AND YOUR HAVE TO RE UPLOAD A SKETCH TO IT
 */


//////////////////////////////////////////////
//GLOBAL VARIABLES
//////////////////////////////////////////////
int pressureSensorPin = A0;
int pressureReading = 0;
int powerPin = A1;// this pin controls the transistor that turns on/off the power
long millisecondsFor23Hr = 82800000;
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
  //!!!!!!!!!!this will have to be uploaded the first time but then this will have to be removed and the sketch will have -
  //!!!!!!!!! to be uploaded again otherwise the date will change every time the arduino resets/turnsoff
// rtc.setDOW(SUNDAY);        // Set Day-of-Week to FRIDAY
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
  
  // the time that I set is irrelevant, the main point is that it triggers exactly every 24hrs
  if(t.min == 0)
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
    digitalWrite(powerPin,LOW);//turn off the power to save battery
    delay(3300000);// this will delay the arduino for 23 hours, might as well to prevent it reading the rtc so often
  }
}
///////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////
void setUpRf()
{
  ///////setup rf
  nRF905_init();// Power up nRF905 and initialize 
  byte incoming[] = RXADDR;// Send this device address to nRF905
  nRF905_setRXAddress(incoming);
  byte outgoing[] = TXADDR;// Send remote device address to nRF905
  nRF905_setTXAddress(outgoing);
  nRF905_receive();// Put into receive mode
}