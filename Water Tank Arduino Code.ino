//////////////////////////////////////////////
// DO NOT PUT THUMB OVER LIGHT SENSOR, IT DOES WIERD THINGS AND YOUR HAVE TO RE UPLOAD A SKETCH TO IT
//////////////////////////////////////////////
int lightSensorPin = A0;
int lightLevel = 0;

#include <nRF905.h>  //Library Author Zak Kemble, Web: http://blog.zakkemble.co.uk/nrf905-avrarduino-librarydriver/
#include <SPI.h>  //SPI Master Library for Arduino

/*
   Note in selection of the RXADDR and TXADDR.  Nordic recommends that the address length be 24 bits or higher
   in length.   Smaller lengths can lead to statistical failures due to the address bein repeated as part of the 
   data packet.  Each byte should be unique.  The address should have several level shifts (101010101). 
*/

#define RXADDR {0xFE, 0x4C, 0xA6, 0xE5} // Address of this device (4 bytes)
#define TXADDR {0x58, 0x6F, 0x2E, 0x10} // Address of device to send to (4 bytes)

#define PACKETPAUSE 500 // Short Break after sending each data packet

void setup()
{
  //set up light sensor
  pinMode(lightSensorPin,INPUT);
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
  Serial.println(F("Transmitter started"));
}

void loop()
{
/*  Put data to transmit in the data array.  Maximum packet data size is 32 bytes. The Arduino UNO
    has 2 byte integers. Since an integer data type has 2 bytes (16 bits), only 16 integer data items
    can be sent in a packet.  Any data beyond 32 bytes will be dropped and not readable at the receiver.
    Additional data must be sent in the next Packet.
  
    NRF905_MAX_PAYLOAD is defined in the nRF905 library as 32 bytes.
*/
  
  lightLevel = analogRead(lightSensorPin);
  delay(50);
  lightLevel = map(lightLevel,0,1023,0,100);
  int data[1] = {lightLevel};
  Serial.println(data[0]);
  nRF905_setData(data, sizeof(data));

/*  Debug printout of data array showing information transmitted.

  for (int i=0; i<=15; i++)
        {
          Serial.print("   ");
          Serial.print(data[i], DEC);
          delay(100);
        }
  Serial.println();
  Serial.println();
*/

  // Send data array payload (send fails if other transmissions are going on, keep trying until success)
 while(!nRF905_send())
 {
 }
 Serial.println("packet sent");
 delay(1000);

}
