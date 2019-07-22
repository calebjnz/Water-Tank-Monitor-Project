
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
int screenNumber = 0;

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

}

///////////////////////////////////////////////////////////////
//LOOP
///////////////////////////////////////////////////////////////
void loop()//we are not allowed to have any delays in the loop at the moment because it has to be looking for an rf signal constantly
{
   // Make data array buffer
  int data[1];   //array size is 32 bytes defined by NRF905_MAX_PAYLOAD in library
          
  // Wait for data packet
 
  while(!nRF905_getData(data, sizeof(data)));
  {
    if(millis()- timeOfLastScreenChange > 4000)
    {
      screenNumber++;
      timeOfLastScreenChange = millis();
      changeScreen();
    }
  }
  //if we have got here then we have recieved somthing
  Serial.println(F("Data Received....."));// it has de message

  Serial.print(data[0], DEC);
  
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
  lcd.print((EEPROM.read(EEPROM.read(1023))),DEC);
  Serial.println((EEPROM.read(EEPROM.read(1023))),DEC);
}

void showYesterdaysWaterLevel()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Yesterdays depth");
  lcd.setCursor(0,2);
  lcd.print((EEPROM.read(EEPROM.read(1023)-1)),DEC);
  Serial.println((EEPROM.read(EEPROM.read(1023)-1)),DEC);
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
    int depth = (0.9821 * ((a/256) * 1023) + 57.639);
    EEPROM.write(EEPROM.read(1023),depth);//EEPROM.read(1023) is where we are going to store the day, this is because if the arduino resets it needs to know what day it was up to, first day is day 0
    EEPROM.write(1023,EEPROM.read(1023) + 1);// this adds one onto the date, the date changes when the module recieves a signal, this makes it also very dependant on receiving a signal from the rf module and receiving it at the right time  
    Serial.println("recorded the water tank level for today, it's  "+ depth);// just prints the value recieved on the serial monitor
  }
}
