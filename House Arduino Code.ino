
/* nrf functions derived from 
 *  Michael Welsh September 7, 2017 for implementation using the Arduino UNO
 * To be used with complementary program transmitter_sample_JMW_V2
 *  
 * This program is designed to integrate the UNO and nRF905.  It operates in a
 * RECEIVE mode to obtain data from the transmitter.  The transmitter is also integrated to 
 * a UNO and NRF905.  This program receives up to 16 integer data types (2 bytes each) for a 
 * maximum 32 byte packet.  Range for integer is -32768 through 32767. 
 *  
 * 
 * Program and Library adopted from the Rethink Tech Inc. - Tinkbox (nRF905) and the
 * Jeff Rowberg I2C device class
 *  
 *  
 * UNO to nRF905 BOARD PIN/Control Feature
 *
 * 7 -> CE     Standby - High = TX/RX mode, Low = Standby
 * 8 -> PWR    Power Up - High = On, Low = Off
 * 9 -> TXE    TX or RX mode - High = TX, Low = RX
 * 2 -> CD     Carrier Detect - High when RF signal detected, for collision avoidance 
 * 3 -> DR     Data Ready - High when finished transmitting/data received
 * 10 -> CSN   SPI SS
 * 12 -> SO    SPI MISO
 * 11 -> SI    SPI MOSI
 * 13 -> SCK   SPI SCK
 * GND -> GND  Ground

 In this version if i turn the arduino off while this is going then things will be messed up
 
*/
////////////////////////////////////////////////////////////////
//LIBRARIES
///////////////////////////////////////////////////////////////
#include <nRF905.h>  //Library Author Zak Kemble, Web: http://blog.zakkemble.co.uk/nrf905-avrarduino-librarydriver/
#include <SPI.h>  //SPI Master Library for Arduino

#include <EEPROM.h>

#include <LiquidCrystal.h>
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);//pins for RS, E DB4,DB5,DB6,DB7

/*
  Note in selection of the RXADDR and TXADDR.  Nordic recommends that the address length be 24 bits or higher
  in length.   Smaller lengths can lead to statistical failures due to the address bein repeated as part of the 
  data packet.  Each byte should be unique.  The address should have several level shifts (101010101). 
*/
///////////////////////////////////////////////////////////////
//VARIABLES
///////////////////////////////////////////////////////////////
long int timeOfLastMessage = 0;
long int timeOfLastScreenChange = 0;
int screenNumber = 1;

#define RXADDR {0x58, 0x6F, 0x2E, 0x10} // Address of this device (4 bytes)
#define TXADDR {0xFE, 0x4C, 0xA6, 0xE5} // Address of device to send to (4 bytes)

#define PACKETPAUSE 250 // Short Break after receiving each data packet

//////////////////////////////////////////////////////////////
//SETUP
/////////////////////////////////////////////////////////////
void setup()
{
  //LCD setup//////////
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("setup");
  
  // Power up nRF905 and initialize
  nRF905_init();
  
  // Send this device address to nRF905
  byte incoming[] = RXADDR;
  nRF905_setRXAddress(incoming);

  // Send remote device address to nRF905
  byte outgoing[] = TXADDR;
  nRF905_setTXAddress(outgoing);
 
  // Put into receive mode
  nRF905_receive();

  // Start serial coomunication with monitor.  Send start message.
  Serial.begin(9600); 
  Serial.print(F("Receiver started....."));

    /************/
    //this will need to be removed in the real thing
    EEPROM.write(1023,1);
}

///////////////////////////////////////////////////////////////
//LOOP
///////////////////////////////////////////////////////////////
void loop()//we are not allowed to have any delays in the loop at the moment because it has to be looking for an rf signal constantly
{
   // Make data array buffer
  int data[1];   //array size is 32 bytes defined by NRF905_MAX_PAYLOAD in library
          
  // Wait for data packet 
  while(!nRF905_getData(data, sizeof(data)))
  {
    if(millis()- timeOfLastScreenChange > 4000)
    {
      screenNumber++;
      timeOfLastScreenChange = millis();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("screen change");
      Serial.println("screen change");//for debugging
      changeScreen();
    }
  }
  //if we have got here then we have recieved somthing
  Serial.println("Data Received.....");// it has de message

  Serial.println(data[0], DEC);
  
  writeToEEPROM(data[0]);
  // Create horizontal spacing and pause between data packets.      
  delay(PACKETPAUSE);
}

//////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////
void changeScreen()//the screen number changes every 4sec which will make the lcd change screen
{
  switch(screenNumber)
  {
    case 1: 
      showTodaysWaterLevel();
      break;
    case 2: 
      showYesterdaysWaterLevel();
      break;
    case 3: 
      dateWaterRunsOut();
      break;
    case 4:
      screenNumber = 1;
      showTodaysWaterLevel();
      Serial.println("screen number is 4");
      break;
  }
}

void showTodaysWaterLevel()// the equation for pressure vs water tank depth is d = 0.9821p +57.639
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Todays depth =");
  lcd.setCursor(0,2);
  int day = EEPROM.read(1023);
  Serial.println("day");
  Serial.println(day);
  lcd.print(EEPROM.read(day),DEC);
  Serial.println(EEPROM.read(day),DEC);
  delay(100);
}

void showYesterdaysWaterLevel()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Yesterdays depth");
  lcd.setCursor(0,2);
  int day = EEPROM.read(1023);
  lcd.print((EEPROM.read(day - 1)),DEC);
  Serial.println((EEPROM.read(day - 1)),DEC);
  delay(100);
}

void dateWaterRunsOut()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("working on this");
}

void writeToEEPROM(int a)//records the recieved value to its day on the eeproms memory
{
  if(millis() - timeOfLastMessage > 60000 || timeOfLastMessage == 0)//either this is the first time the program has been run or that we have not recieved signals for a while and they are new day signals
  {
    //this ensures that it only records the first signal of the burst of signals sent from the water tank
    timeOfLastMessage = millis();
    //int depth = (0.9821 * ((a/256) * 1023) + 57.639); uncomment this when able to record values to eeprom
    int depth = a;
    int day = EEPROM.read(1023);
    EEPROM.write(day,depth);//EEPROM.read(1023) is where we are going to store the day, this is because if the arduino resets it needs to know what day it was up to, first day is day 0
    EEPROM.write(1023,day + 1);// this adds one onto the date, the date changes when the module recieves a signal, this makes it also very dependant on receiving a signal from the rf module and receiving it at the right time  
    Serial.println("recorded the water tank level for today, it");
    Serial.println(depth);// just prints the value recieved on the serial monitor
  }
}
