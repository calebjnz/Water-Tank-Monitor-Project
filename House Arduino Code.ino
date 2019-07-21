

/*
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

#include <nRF905.h>  //Library Author Zak Kemble, Web: http://blog.zakkemble.co.uk/nrf905-avrarduino-librarydriver/
#include <SPI.h>  //SPI Master Library for Arduino

#include <EEPROM.h>

/*
  Note in selection of the RXADDR and TXADDR.  Nordic recommends that the address length be 24 bits or higher
  in length.   Smaller lengths can lead to statistical failures due to the address bein repeated as part of the 
  data packet.  Each byte should be unique.  The address should have several level shifts (101010101). 
*/

#define RXADDR {0x58, 0x6F, 0x2E, 0x10} // Address of this device (4 bytes)
#define TXADDR {0xFE, 0x4C, 0xA6, 0xE5} // Address of device to send to (4 bytes)

#define PACKETPAUSE 250 // Short Break after receiving each data packet

void setup()
{
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
  Serial.print(F("Transmitter started....."));
}

void loop()
{
  // Make data array buffer
  int data[1];   //array size is 32 bytes defined by NRF905_MAX_PAYLOAD in library
          
  // Wait for data packet

  Serial.print(F("Waiting for data....."));
 
  while(!nRF905_getData(data, sizeof(data)));

  Serial.println(F("Data Received....."));

  Serial.print(data[0], DEC);
  
    
  // Create horizontal spacing and pause between data packets.      
  Serial.println();
  Serial.println();
  delay(PACKETPAUSE);
  
}  // Continue to receive further data if available.
