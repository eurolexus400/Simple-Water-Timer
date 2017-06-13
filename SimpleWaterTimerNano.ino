#include<CountUpDownTimer.h>
#include<EEPROM.h>
#include "RTClib.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

RTC_DS1307 rtc;
CountUpDownTimer T(DOWN);

                      //Pin Allocation Table:
#define Relay0 2      //digital pin 0 = Water Relay 0
#define Relay1 3      //digital pin 1 = Water Relay 1
#define Relay2 4     //digital pin 2 = Water Relay 2
#define Relay3 5     //digital pin 3 = Water Relay 3
#define Relay4 6     //digital pin 4 = Water Relay 4
#define Relay5 7     //digital pin 5 = Water Relay 5

                        //EEPROM Address Table
#define WaterAddr 0     //holds address for water time

//length of time to water per valve                        
#define WaterLength 12     //this is for testing purposes only (change the following lines 24, 159, 160, 282, and 283 for working code)
//#define WaterLength 30  //this is working code

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()
#define num_pins 6

boolean WaterFlag;  //flag to start watering 1 = now watering, 0 = stop watering
int WaterValve;     //holds the valve we are watering
int WaterTime;      //holds the length of time to water each valve
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int chipSelect = 10;
boolean flag;
char filename[] = "LOGGER00.CSV";
int counter;

// the logging file
File logfile;
//DateTime now = rtc.now();

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  while(1);
}


void setup(void)
{
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  #if WAIT_TO_START
    Serial.println("Type any character to start");
    while (!Serial.available());
  #endif //WAIT_TO_START
  
    // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("Card Initialized.");
  
  // create a new file
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("Couldn't Create File");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC 
  Wire.begin();   
  if (!rtc.begin()) 
  { 
    logfile.println("RTC failed"); 
    #if ECHO_TO_SERIAL 
      Serial.println("RTC failed"); 
    #endif  //ECHO_TO_SERIAL 
  } 

  logfile.println("Timestamp: (Year, Month, Day, Hour, Minute, Second, Valve, Water Time)");
  logfile.println("This is for my garden watering.");
  #if ECHO_TO_SERIAL 
    Serial.println("Timestamp: (Year, Month, Day, Hour, Minute, Second, Valve, Water Time)"); 
  #endif //ECHO_TO_SERIAL 
  
  EEPROM.write(WaterAddr, WaterLength);
  
  for(counter = 0; counter < num_pins; counter++)
  {
    pinMode(counter + 2, OUTPUT);
    digitalWrite(counter + 2, LOW);
//    Serial.println("Pin:" + (String)counter + " is set and low.");
  }

  counter = 0;
  Serial.println("Systems Check...");  
  for(counter = 0; counter < num_pins; counter++)
  {
    digitalWrite(counter + 2, HIGH);
    delay(5000);
    digitalWrite(counter + 2, LOW); 
    Serial.println("Relay: " + (String)(counter) + " is now low.");
  }
  Serial.print("Systems Check Complete.");
  Serial.println("");

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  WaterTime = EEPROM.read(WaterAddr);
//  T.SetTimer(0,0,WaterTime);  //this is for testing purposes only
  T.SetTimer(0,WaterTime,0);  //this is working code
  WaterValve = 0;
  flag = 1;
  logfile.close();
}

void loop()
{
  DateTime now = rtc.now();
  T.Timer(); // run the timer
  
  if (! SD.exists(filename)) { //setup datalogger file if erased
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      //break;  // leave the loop!
  }

  
  {
  //  if (now.second() == 18)   //this is for testing purposes only
    if (now.hour() == 18)     //this is working code
    {
      WaterFlag = 1; //WaterFlag = True
      
      while (flag == 1)
      {
        Serial.print(now.year(), DEC);
        Serial.print("/");
        Serial.print(now.month(), DEC);
        Serial.print("/");
        Serial.print(now.day(), DEC);
        Serial.print(" ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.print(now.minute(), DEC);
        Serial.print(":");
        Serial.println(now.second(), DEC);
        
        flag = 0;
      }
    }
  }

  if (WaterFlag == 1)
  {
    switch (WaterValve){
      case 0:
        digitalWrite(Relay0, HIGH);  //On
        digitalWrite(Relay1, LOW); //Off
        digitalWrite(Relay2, LOW); //Off
        digitalWrite(Relay3, LOW); //Off
        digitalWrite(Relay4, LOW); //Off
        digitalWrite(Relay5, LOW); //Off
        T.StartTimer(); 
        break;
      case 1:
        digitalWrite(Relay0, LOW); //Off
        digitalWrite(Relay1, HIGH);  //On
        digitalWrite(Relay2, LOW); //Off
        digitalWrite(Relay3, LOW); //Off
        digitalWrite(Relay4, LOW); //Off
        digitalWrite(Relay5, LOW); //Off
        //Serial.println("Water Valve 1 is ON.");
        T.StartTimer();
        break;
      case 2:
        digitalWrite(Relay0, LOW); //Off
        digitalWrite(Relay1, LOW); //Off
        digitalWrite(Relay2, HIGH);  //On
        digitalWrite(Relay3, LOW); //Off
        digitalWrite(Relay4, LOW); //Off
        digitalWrite(Relay5, LOW); //Off
        //Serial.println("Water Valve 2 is ON.");
        T.StartTimer();
        break;
      case 3:
        digitalWrite(Relay0, LOW); //Off
        digitalWrite(Relay1, LOW); //Off
        digitalWrite(Relay2, LOW); //Off
        digitalWrite(Relay3, HIGH);  //On
        digitalWrite(Relay4, LOW); //Off
        digitalWrite(Relay5, LOW); //Off
//        Serial.println("Water Valve 3 is ON.");
        T.StartTimer();
        break;
      case 4:
        digitalWrite(Relay0, LOW); //Off
        digitalWrite(Relay1, LOW); //Off
        digitalWrite(Relay2, LOW); //Off
        digitalWrite(Relay3, LOW); //Off
        digitalWrite(Relay4, HIGH);  //On
        digitalWrite(Relay5, LOW); //Off
//        Serial.println("Water Valve 4 is ON.");
        T.StartTimer();
        break;
      case 5:
        digitalWrite(Relay0, LOW); //Off
        digitalWrite(Relay1, LOW); //Off
        digitalWrite(Relay2, LOW); //Off
        digitalWrite(Relay3, LOW); //Off
        digitalWrite(Relay4, LOW); //Off
        digitalWrite(Relay5, HIGH);  //On
//        Serial.println("Water Valve 5 is ON.");
        T.StartTimer();
        break;
    }
  }
    
  if (T.TimeHasChanged() ) // this prevents the time from being constantly shown.
  {
    Serial.print(T.ShowHours());
    Serial.print(":");
    Serial.print(T.ShowMinutes());
    Serial.print(":");
    Serial.print(T.ShowSeconds());
    Serial.print(":");
    Serial.print(T.ShowMilliSeconds());
    Serial.print(":");
    Serial.println(T.ShowMicroSeconds());
    // This DOES NOT format the time to 0:0x when seconds is less than 10.
    // if you need to format the time to standard format, use the sprintf() function.
  }

  if (T.TimeCheck(0,0,0) == 1) //if timer is finished for this valve
  {
//    T.SetTimer(0,0,WaterTime);  //this is for testing purposes only
    T.SetTimer(0,WaterTime,0);  //this is working code
    
    logfile = SD.open(filename, FILE_WRITE); 

    if(logfile)
    {
      //Year, Month, Day, Hour, Minute, Second, Valve, Water Time
      logfile.print('"');   
      logfile.print(now.year(), DEC); 
      logfile.print("/");
      logfile.print(now.month(), DEC);
      logfile.print("/");
      logfile.print(now.day(), DEC);
      logfile.print(" ");
      logfile.print(now.hour(), DEC);
      logfile.print(":");
      logfile.print(now.minute(), DEC);
      logfile.print(":");
      logfile.print(now.second(), DEC);
      logfile.print(" ");   
      logfile.print("Valve: " + (String)WaterValve);
      logfile.print(",");
      logfile.print("Watering Time: " + (String)WaterLength) + " min";;
      logfile.println('"');
      Serial.println("Printed to logfile: " + (String)filename);
      //Serial.println("Water Valve " + (String)WaterValve + " has finished.");
      logfile.close();
   }
   else 
   {
    Serial.println("Error opening datalog file: " + (String)filename); 
   }
    #if ECHO_TO_SERIAL
    Serial.print('"');   
    Serial.print(now.year(), DEC); 
    Serial.print("/");
    Serial.print(now.month(), DEC);
    Serial.print("/");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.print(" ");
    Serial.print("Valve: " + (String)WaterValve);
    Serial.print(",");
    Serial.print("Watering Time: " + (String)WaterLength) + " min";
    Serial.println('"');
    #endif //ECHO_TO_SERIAL
    
    Serial.println("Water Valve " + (String)WaterValve + " has finished.");
    ++WaterValve; //increment through watering cycle
    if (WaterValve == 6) //if the watering cycle is complete
    {
      WaterValve = 0;   //reset to valve 0
      WaterFlag = 0;    //turn watering off
      T.StopTimer();
      digitalWrite(Relay5, LOW); //Off
      flag = 1;
      Serial.println("");
    }
  }
}

