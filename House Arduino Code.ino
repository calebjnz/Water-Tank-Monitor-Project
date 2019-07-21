/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.
*/

//sd card stuff/////////////////////////////////////////////////////////
#include <SPI.h>
#include <SdFat.h>
SdFat SD;
String dataString = "";
const int SDchipSelect = 6;

//lcd stuff/////////////////////////////////////////////////////////////
#include <LiquidCrystal.h>
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);//pins for RS, E DB4,DB5,DB6,DB7


//code stuff///////////////////////////////////////////////////////////
String buffer;

//rf stuff/////////////////////////////////////////////////////////////
#include <nRF905.h>  //Library Author Zak Kemble, Web: http://blog.zakkemble.co.uk/nrf905-avrarduino-librarydriver/
#include <SPI.h>
#define RXADDR {0x58, 0x6F, 0x2E, 0x10} // Address of this device (4 bytes)
#define TXADDR {0xFE, 0x4C, 0xA6, 0xE5} // Address of device to send to (4 bytes)
#define PACKETPAUSE 250 // Short Break after receiving each data packet


void setup() {
  Serial.begin(9600);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("hi");
  
  pinMode(10,OUTPUT);
  digitalWrite(SDchipSelect,HIGH);
  digitalWrite(10,LOW);
  Serial.println("hi");
  //LCD setup//////////
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);

  //rf setup////////////

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
  // Open serial communications and wait for port to open:
  digitalWrite(10,HIGH);
  digitalWrite(SDchipSelect,LOW);

  Serial.print("Initializing SD card...");
  lcd.clear();
  lcd.print("Initializing SD");
  // see if the card is present and can be initialized:
  digitalWrite(10,HIGH);//this is to end spi with rf 
  digitalWrite(SDchipSelect,LOW);//this begins spi with sd
  if (!SD.begin(SDchipSelect))
  {
    Serial.println("Card failed, or not present");
    lcd.clear();
    lcd.print("Card failed");
    // don't do anything more:
    while (1){};
  }
  //if the card is not instalised it will get trapped by the infinit while loop
  //if it gets to here it has been instalised, thus we print
  Serial.println("card initialized."); 
  lcd.clear();
  lcd.print("Card initialized");
  digitalWrite(SDchipSelect,HIGH);//ends spi with sd card
  digitalWrite(10,LOW);//this starts spi communication with the rf module again
}


void loop() {
  nRF905_receive();
  Serial.println("loop");
  int data[1];   //array size is 32 bytes defined by NRF905_MAX_PAYLOAD in library
  //this will keeping on asking the rf module to get data, if there is now data the while loop 
  //with nothing in it will keep on going on forever
  digitalWrite(SDchipSelect,HIGH);//turns off spi with sd
  digitalWrite(10,LOW);//ensures spi communication with the rf module
  Serial.println("loop");
  while(!nRF905_getData(data, sizeof(data)));
  {
  }
  //if you get to here you have gotten out of the while loop and you have data stored in the 
  //data array
  Serial.println(data[0]);
  Serial.println("loope");
  printToSD(data[0]);
  lcd.clear();
  lcd.print("got stuff");
  Serial.println("got stuff");
}


void printToSD(int z){
  dataString = String(z);
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  digitalWrite(10,HIGH);//this ensures end of spi communication with rf module
  digitalWrite(SDchipSelect,LOW);//begins spi with sd card
  File dataFile = SD.open("test2.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) 
  {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
    lcd.clear();
    lcd.print(dataString);
  }
  // if the file isn't open, pop up an error:
  else 
  {
    Serial.println("error opening datalog.txt");
    lcd.clear();
    lcd.print("error opening");
  }
  dataString = "";
  digitalWrite(SDchipSelect,HIGH);//this ends spi with sd card
  digitalWrite(10,LOW);//this starts spi communication with the rf module again
}
