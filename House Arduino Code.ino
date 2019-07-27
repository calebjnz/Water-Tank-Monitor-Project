/*
 * This program will receive the raw analog pressure readings from the arduino on the water tank.
 * This arduino will be constantly checking for rf signals and when it receives one it will record it 
 * to a specific eeprom slot which is dependant on its date. Every 4 seconds the lcd will change its screen
 * to show todays water level, yesterdays water level or how many days until the water runs out.
 * 
*/
////////////////////////////////////////////////////////////////
//LIBRARIES
///////////////////////////////////////////////////////////////
#include <nRF905.h>  //Library Author Zak Kemble, Web: http://blog.zakkemble.co.uk/nrf905-avrarduino-librarydriver/

#include <SPI.h>  //SPI Master Library for Arduino

#include <EEPROM.h>  //Arduino Library

#include <LiquidCrystal.h> //Arduino Library
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);//pins for RS, E DB4,DB5,DB6,DB7

///////////////////////////////////////////////////////////////
//GLOBAL VARIABLES
///////////////////////////////////////////////////////////////
long int timeOfLastMessage = 0;//time of last message in milliseconds since start
long int timeOfLastScreenChange = 0;//time of last message in milliseconds since start
int screenNumber = 1;//used for changing the screens of the lcd
bool rfNotWorking = false;//
long int maxGapBetweenRfMessages = 5400000; //normally its 24hrs and 20sec in milliseconds 86420000 but for testing purposes it is 1.5hr

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
  
}

///////////////////////////////////////////////////////////////
//LOOP
///////////////////////////////////////////////////////////////
void loop()//we are not allowed to have any delays in the loop at the moment because it has to be looking for an rf signal constantly
{
  // Make data array buffer
  int data[1];   //array size is 1 byte
          
  // Wait for data packet 
  while(!nRF905_getData(data, sizeof(data)))
  {
    if(millis()- timeOfLastScreenChange > 4000)// the while loop should be looping most of the time if the if statment looks to see if the screen needs changed
    {
      screenNumber++;//this changes the screen that is going to be displayed, each screen has a number
      timeOfLastScreenChange = millis();//time of last screen change is the current time
      Serial.println("screen change");//for debugging
      changeScreen();// runs the change screen function with the new screenNumber
    }
    if((millis() - timeOfLastMessage) > maxGapBetweenRfMessages)//this makes sure that rf signals are still happenning every 24hrs
    {
      //we have not receiced a rf signal in 24hrs and 20 sec
      rfNotWorking = true;
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
  // this is to ensure that we only go to screen 4 if rf is not working
  // and that the screen number will never be 5;
  if((screenNumber >= 4 && rfNotWorking == false) || screenNumber >= 5 )
  {
    screenNumber = 1;
  }
  switch(screenNumber)//choose the screen that will be displayed from the screen number
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
    case 4://we will never get to screen 4 if rf is working
      rfErrorMessage();//displays an error message on this screen
      // when this screens time is up the screen number will be 5 but it will go back to 1 because of the if statment     
  }
}


void showTodaysWaterLevel()// the equation for pressure vs water tank depth is d = 0.9821p +57.639
{
  float depth = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Todays depth =");
  lcd.setCursor(0,2);
  long day = EEPROM.read(1023);
  Serial.print("today ");
  Serial.println(day);
  Serial.print("depth ");
  depth = EEPROM.read(day);//because we multiplied i by 50 to get it to fit in the eeprom
  lcd.print(depth);
  checkDepthIsGood(depth);
  Serial.println(depth);
  delay(30);
}


void rfErrorMessage()
{
  //to get here the we know that rfNotWorking == true
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("rf is not");
  lcd.setCursor(0,2);
  lcd.print("working");
}


void showYesterdaysWaterLevel()
{
  float depth = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Yesterdays depth");
  lcd.setCursor(0,2);
  long day = EEPROM.read(1023) - 1;
  Serial.print("yesterday date ");
  Serial.println(day);
  Serial.print("depth ");
  depth = EEPROM.read(day);//because we multiplied i by 50 to get it to fit in the eeprom
  lcd.print(depth);
  checkDepthIsGood(depth);
  Serial.println(depth);
  delay(30);
}


void dateWaterRunsOut()
{
  float changeTotal = 0;
  float numberChanges = 5;
  float changeAverage = 0;
  float day = EEPROM.read(1023);
  float currentWaterLevel;
  float daysLeft = 0;
  if (day > 5)
  {
    for(int j = day; j >= (day-5);j--)// this goes through the past 5 days and sees how much it has increased/decreased from its previuos day
    {
      int change = EEPROM.read(j) - EEPROM.read(j-1);//this adds to the total the amount that the water level has decreased or increased from the previous day of the sample day
      changeTotal += change;
      Serial.println(change);
    }
    changeAverage = changeTotal / numberChanges;//finds the average that the water level changes between each day
    Serial.print("change average = ");
    Serial.println(changeAverage);
    currentWaterLevel = EEPROM.read(day);//gets todays water level
    Serial.print("current water level = ");
    Serial.println(currentWaterLevel);
    changeAverage = changeAverage * -1;//convert from negative to positive number so the days left becomes a positive number
    daysLeft = currentWaterLevel / changeAverage;//finds out how many days until the water runs out
    
    if (daysLeft > 0 && changeAverage != 0)//we cant have a negative days left and if the change average == 0 then daysleft is infinite
    {
      //displays the days left on the lcd and serial monitor
      Serial.print("days left = ");
      Serial.println(daysLeft);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("days till empty");
      lcd.setCursor(0,2);
      lcd.print(daysLeft);
    }
    else
    {
      Serial.println("water level is increasing");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("water level");
      lcd.setCursor(0,2);
      lcd.print("is increasing");
    }
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("not enough");
    lcd.setCursor(0,2);
    lcd.print("data yet");
    Serial.println("not enough data yet to predict");
  }
  delay(30);
}


float convertAnalogReadToCm(float b)
{
  b = b - 138;
  b = b * 0.0049; // convert analog reading to a voltage
  b = ((b/3.30 - 0.04) / 0.018);// this converts the voltage into kPa
  b = (b*1000)/ 9780.60;//converts kPa to Pa by * 1000 and then makes converts that to depth by / 9780.6
  b = b * 100; //converts from m to cm
  if (b < 0)
  {
    b = 0;//can have negative depths if it is less than 0 then it probably 0  
  }
  return b;
}


void checkDepthIsGood(int c)
{
  //this function adds onto the end of whatever that is displayed "fix" this - 
  //if the depth is less than 0 or greater than the depth of the tank.
  if(c < 0 || c > 250)
  {
    lcd.print("fix this");
  }
}


void restartEEPROM()//this function will clear up the eeprom but keep the previous 30 days
{
  for(int originalDay = 255; originalDay >= 225; originalDay--)
  {
    int depth = EEPROM.read(originalDay);//gets the depth of the
    int newDay = 30 - (255-originalDay);//day 255 becomes day 30
    EEPROM.write(newDay,depth);//reasigns the originalday to its newday
  }
  EEPROM.write(1023,30);//this makes the current day 30  
  // i dont need to clear the whole eeprom, there is no harm in keeping those values there, the will be changed
}


void writeToEEPROM(int a)//records the recieved value to its day on the eeproms memory
{
  if(millis() - timeOfLastMessage > 120000 || timeOfLastMessage == 0)
  {
    //either this is the first time the program has been run or that we have not recieved signals for a while and they are new day signals
    //this ensures that it only records the first signal of the burst of signals sent from the water tank
    timeOfLastMessage = millis();
    rfNotWorking = false;// we have been receiving rf signals
    Serial.print("raw analog recived = ");
    Serial.println(a);
    int depth = convertAnalogReadToCm(a);//finds the depth in cm from the rf value received
    if(EEPROM.read(1023) >= 255)//ensures that the day value dosent exceed 255 because eeprom slot cant exceed 255
    {
      restartEEPROM();
    }
    int today = EEPROM.read(1023) + 1;// date changes when the house arduino receieves the first signal in a while, eeprom(1023) now holds the current date
    EEPROM.write(1023,today);//EEPROM.read(1023) is where we are going to store the day, this is because if the arduino resets it needs to know what day it was up to, first day is day 0
    EEPROM.write(today,depth);//writes the depth to that days slot
    Serial.println("recorded the water tank depth for today in cm, it");
    Serial.println(depth);// just prints the value recieved on the serial monitor
  }
  delay(30);
}