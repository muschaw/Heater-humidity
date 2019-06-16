#include <SPI.h>
#include <Wire.h>
#include <BME280I2C.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Hardcoded HW setup 
#define OLED_RESET 4
float temp(NAN), hum(NAN), pres(NAN);
Adafruit_SSD1306 display(OLED_RESET);
RTC_DS1307 RTC;
BME280I2C bme;    
BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
BME280::PresUnit presUnit(BME280::PresUnit_Pa);
   

//Params 
#define relay1 50         // Relay heating digital pin on Arduino
#define relay2 52         // Relay humidity digital pin on Arduino  
#define configuretemp 28  // Desired temp in C
#define configurehum 50   // Desired humidity in %
#define humdiff 10        // Humedity % diff allowed 
#define tempdiff 2        // Temperture C diff allowed 
#define gettemprate 2500  // Interval for BME temp collection 
#define settemprate 5000  // Internal for Relay check
#define screenrefreshtimer 1000 // OLED refresh rate

//timers and counters
int alarmtemp = 0;
int humset = 0;
unsigned long temptimestart;
unsigned long temptimestop = 0;
unsigned long humtimestop = 0;
unsigned long humtimestart;

void setup()   {
                  
  Serial.begin(115200);
  pinMode(relay1, OUTPUT);
  digitalWrite(relay1, LOW);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay2, LOW);
  
  Wire.begin();
  bme.begin();
  delay(1000);
  
  bme.read(pres, temp, hum, tempUnit, presUnit);    // Read once to set the screen with values.

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);        // Screen I2C setup
  display.clearDisplay();                           // Clear display
  
  RTC.begin();                                      //Start clock
   if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));       // remove from IF on first setup of clock and reinstall with IF
  }
}

void loop() {
 
  printdisp();   // Print to desplay and Serial 
  gettemp();     // Get temp and hum 
  settemp();     // Set heater 
  sethum();      // Set humidifier
}

void sethum(void)  {

static unsigned long samplingTime = millis();  
  if(millis() - samplingTime > settemprate){
        
      if (hum  < configurehum - humdiff && humset == 0 ) {          
              digitalWrite(relay2, HIGH);                           
              humset = 1;    
              calculatehumrun();              
      }
          if (hum  > configurehum && humset == 1){                        
              digitalWrite(relay2, LOW);
              humset = 0;                             
              calculatehumrun();
           }
                samplingTime=millis(); 
     }

}

void calculatehumrun(void) {

  DateTime now = RTC.now();
  
      if (humset == 1){
          Serial.println("#######Humidifying#######");                   
          humtimestart = now.second() + ( now.minute() * 60 ) + ( now.hour() * 3600 );                    

          
              if ( humtimestop == 0){
                Serial.println("First run after reboot");
              }
              else {
                  Serial.print("\tTime since last run in seconds: ");
                  int humtimesince = humtimestart - humtimestop;
                  Serial.println(humtimesince);
              }
                        
                      Serial.print("Start time in seconds:\t");
                      Serial.println(humtimestart);
                      Serial.print("Time: ");
                      Serial.print(now.hour(), DEC);
                      Serial.print(':');
                      Serial.print(now.minute(), DEC);
                      Serial.print(':');
                      Serial.print(now.second(), DEC);
                      Serial.println();
                      Serial.print("Humidity: ");
                      Serial.println(hum); 
      }
      if(humset == 0){
        Serial.println("#######Humidifying-STOP#######");

          humtimestop = now.second() + ( now.minute() * 60 ) + ( now.hour() * 3600 );

          Serial.print("Stop time in seconds: ");
          Serial.println(humtimestop);
          Serial.print("\tRuntime: ");
          int runtimecount = ( humtimestop - humtimestart ) ;
          Serial.println(runtimecount);
          Serial.print("Time: ");
          Serial.print(now.hour(), DEC);
          Serial.print(':');
          Serial.print(now.minute(), DEC);
          Serial.print(':');
          Serial.print(now.second(), DEC);
          Serial.println();
          Serial.print("Humidity: ");
          Serial.println(hum); 
      }
 return;
}

void settemp(void)  {

static unsigned long samplingTime = millis();  
  if(millis() - samplingTime > settemprate){
        
      if (temp  < configuretemp - tempdiff && alarmtemp == 0 ) {          
              digitalWrite(relay1, HIGH);
              alarmtemp = 1;                            
              calculatetemprun();                   
      }
          if (temp  > configuretemp && alarmtemp == 1){                        
              digitalWrite(relay1, LOW);
              alarmtemp = 0;           
              calculatetemprun();                                
           }
                samplingTime=millis(); 
     }

}

void calculatetemprun(void) {

  DateTime now = RTC.now();
  
      if (alarmtemp == 1){
          Serial.println("#######HEATING#######");
          temptimestart = now.second() + ( now.minute() * 60 ) + ( now.hour() * 3600 );                    
          
              if ( temptimestop == 0){
                Serial.println("First run after reboot");
              }
              else {                       
                  Serial.print("\tTime since last run in seconds: ");
                  int temptimesince = temptimestart - temptimestop;
                  Serial.println(temptimesince);
              }
          
                      Serial.print("Start time in seconds: ");
                      Serial.println(temptimestart);
                      Serial.print("Time: ");
                      Serial.print(now.hour(), DEC);
                      Serial.print(':');
                      Serial.print(now.minute(), DEC);
                      Serial.print(':');
                      Serial.print(now.second(), DEC);
                      Serial.println();
                      Serial.print("Temp: ");
                      Serial.println(temp); 
      }
      if (alarmtemp == 0){
        Serial.println("#######HEATING-STOP#######");

          temptimestop = now.second() + ( now.minute() * 60 ) + ( now.hour() * 3600 );

          Serial.print("Stop time in seconds: ");
          Serial.println(temptimestop);
          Serial.print("\tRuntime: ");
          int runtimecount = ( humtimestop - humtimestart ) ;
          Serial.println(runtimecount);
          Serial.print("Time: ");
          Serial.print(now.hour(), DEC);
          Serial.print(':');
          Serial.print(now.minute(), DEC);
          Serial.print(':');
          Serial.print(now.second(), DEC);
          Serial.println();
          Serial.print("Temp: ");
          Serial.println(temp); 
      }
 return;
}


void gettemp(void) {
      
static unsigned long samplingTime = millis();
  if(millis() - samplingTime > gettemprate){
  
    bme.read(pres, temp, hum, tempUnit, presUnit);
    samplingTime=millis(); 
  }

}


void printdisp(void) {
  
static unsigned long samplingTime = millis();
  if(millis() - samplingTime > screenrefreshtimer){
      
      DateTime now = RTC.now(); 
  
      //Serial.print(now.hour(), DEC);
      //Serial.print(':');
      //Serial.print(now.minute(), DEC);
      //Serial.print(':');
      //Serial.print(now.second(), DEC);
      //Serial.println(); 
            
      //Serial.print("Temp: ");
      //Serial.print(temp);
      //Serial.print("\t\tHumidity: ");
      //Serial.println(hum);
      
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print(now.hour(), DEC);
      display.print(':');
      display.print(now.minute(), DEC);
      display.print(':');
      display.print(now.second(), DEC);
    
      display.setCursor(56,0);
      display.print(now.day(), DEC);
      display.print('/');
      display.print(now.month(), DEC);
      display.print('/');
      display.print(now.year(), DEC);

      display.setTextSize(1);
      display.setTextColor(BLACK, WHITE);
      display.setCursor(0,8);
      display.print("Temp");
      display.setCursor(50,24);
      display.setTextColor(WHITE);
      display.print("C");
      display.setCursor(56,8);
      display.setTextColor(BLACK, WHITE);
      display.print("Humidity");
      display.setCursor(110,24);
      display.setTextColor(WHITE);
      display.print("%");
        
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,16);
      display.print(temp, 1);
      display.print(" ");
      display.print(hum, 1);

  if (alarmtemp == 1){    // Screen alarm im settemp is running
      display.display();
      display.setTextSize(1);
      display.setTextColor(WHITE,BLACK);
      display.setCursor(30,8);
      display.print("SET");
  }      
  
      display.display();
      display.clearDisplay();
      samplingTime=millis(); 
 }
}



