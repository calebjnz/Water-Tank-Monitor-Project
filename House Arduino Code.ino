

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

int timeBurstOfSignalsBegan = 0;

int recvdMsgInterruptPin = 2;

#define RXADDR {0x58, 0x6F, 0x2E, 0x10} // Address of this device (4 bytes)
#define TXADDR {0xFE, 0x4C, 0xA6, 0xE5} // Address of device to send to (4 bytes)

#define PACKETPAUSE 250 // Short Break after receiving each data packet

void setup()
{
  //LCD setup//////////
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);

  pinMode(2,INPUT);
  pinMode(4,INPUT);
  
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

  EEPROM.write(1023,1);// this is just for testing purposes
  EEPROM.write(1,5);// this is just for testing purposes, will need to change back to 0 when real thing happens

}

void loop()//we are not allowed to have any delays in the loop at the moment because it has to be looking for an rf signal constantly
{
   // Make data array buffer
  int data[1];   //array size is 32 bytes defined by NRF905_MAX_PAYLOAD in library
          
  // Wait for data packet
 
  while(!nRF905_getData(data, sizeof(data)));
  {
    if(digitalRead(4) == HIGH)
    {
      
    }
  }

  Serial.println(F("Data Received....."));// it has de message

  Serial.print(data[0], DEC);
  
  writeToEEPROM(data[0]);
  // Create horizontal spacing and pause between data packets.      
  delay(PACKETPAUSE);
}

void showTodaysWaterLevel()// the equation for pressure vs water tank depth is d = 0.9821p +57.639
{
  int depth = (0.9821 * (EEPROM.read(EEPROM.read(1023))) + 57.639);//this uses the equation that shows the relationship between the depth and the pressure, although if this dosent work i might have to use an array
  lcd.clear();
  lcd.print("Todays depth =");
  Serial.println("Todays depth =");
  lcd.setCursor(2,0);
  lcd.print(depth + "m");
  Serial.println(depth + "m");
}

void showYesterdaysWaterLevel()
{
  
}

void runOutOfWaterEstimate()
{
  
}

void recevdMsg()
{
   // Make data array buffer
  int data[1];   //array size is 32 bytes defined by NRF905_MAX_PAYLOAD in library
          
  // Wait for data packet
 
  while(!nRF905_getData(data, sizeof(data)));
  {
    //do de nothing until it gets de message hehe
  }

  Serial.println(F("Data Received....."));// it has de message

  Serial.print(data[0], DEC);
  
  writeToEEPROM(data[0]);
  // Create horizontal spacing and pause between data packets.      
  delay(PACKETPAUSE);
}


void writeToEEPROM(int a)//records the recieved value to its day on the eeproms memory
{
  if(millis() - timeBurstOfSignalsBegan > 60000 || timeBurstOfSignalsBegan == 0)//either this is the first time the program has been run or that we have not recieved signals for a while and they are new day signals
  {
    //this ensures that it only records the first signal of the burst of signals sent from the water tank
    timeBurstOfSignalsBegan = millis();
    EEPROM.write(EEPROM.read(1023),a);//EEPROM.read(1023) is where we are going to store the day, this is because if the arduino resets it needs to know what day it was up to, first day is day 0
    EEPROM.write(1023,EEPROM.read(1023) + 1);// this adds one onto the date, the date changes when the module recieves a signal, this makes it also very dependant on receiving a signal from the rf module and receiving it at the right time  
    Serial.println("recorded the water tank level for today, it's  "+ a);// just prints the value recieved on the serial monitor
  }

}
