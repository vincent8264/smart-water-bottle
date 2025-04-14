#include <Wire.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  // Adafruit_SSD1306
#include <OneWire.h>
#include <DallasTemperature.h>

//sensor settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  
#define ONE_WIRE_BUS 4 //temp
OneWire oneWire(ONE_WIRE_BUS);	
DallasTemperature sensors(&oneWire);
ThreeWire myWire(9, 10, 8);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
byte statusLed = 13;
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin = 2;
float calibrationFactor = 9;
volatile byte pulseCount;
float flowRate;
int temp;
unsigned int flowMilliLitres;
unsigned int lastseconddrink;
unsigned int totalMilliLitres;
unsigned int dailytotal;
unsigned long oldTime;
bool logging = false;
unsigned int showingconsumed;

void setup()
{
  sensors.begin();
  Serial.begin(9600);
  pinMode(statusLed, OUTPUT);
  pinMode(7,OUTPUT);
  digitalWrite(statusLed, HIGH);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0; //volume per second
  totalMilliLitres = 0; //volume per consumption
  dailytotal = 0; 
  oldTime = 0;
  showingconsumed = 0;
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  Rtc.Begin();
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //Default state
  displaydefault(dailytotal,20);
}

//Main loop, loops every second
void loop()
{
  if ((millis() - oldTime) > 1000)
  {
    //Get water flow 
    detachInterrupt(sensorInterrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    //Get time
    RtcDateTime now = Rtc.GetDateTime();
    //Get temperature
    sensors.requestTemperatures(); 
    temp=sensors.getTempCByIndex(0);

    //Default state
    if (!logging && showingconsumed==0){
      displaydefault(dailytotal,temp);
    }

    //If flow rate started to increase, switch to logging state 
    if (flowMilliLitres >= 0 && !logging) { 
      logging = true;
      
      // Switch back to Default state if started drinking while still in Display state
      if (showingconsumed >0){
        showingconsumed=0;
        // Default state
        displaydefault(dailytotal,temp);
      }
    }
    //If in Logging state, keep updating consumed volume
    if (logging) {
      totalMilliLitres += flowMilliLitres;
    }
    //Drinking stops if flow rate is less than 0 for 2 consecutive seconds
    if (flowMilliLitres < 0 && logging && lastseconddrink==0) {
      logging = false;
  
      printDateTime(now);

      // Add volume consumed to daily total
      dailytotal+=totalMilliLitres;
      
      // Switch to Display state and ring buzzer
      digitalWrite(7,HIGH); //buzzer
      displaydrank(totalMilliLitres,dailytotal,now);
      showingconsumed=1; 
      totalMilliLitres = 0;
      
    }
    lastseconddrink=flowMilliLitres;
    pulseCount = 0;
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    
    
    //Display state will last for 7 seconds after drinking
    if (showingconsumed>0){
      if (showingconsumed==2){
        digitalWrite(7,LOW);
      }
      
      showingconsumed+=1; 
      if (showingconsumed>7){
        showingconsumed=0;
        //Switch back to default state
        displaydefault(dailytotal,temp);
      }
    }

      
    //Notification criterias
    if (now.Second()==0 && showingconsumed==0){
      if(now.Hour()==22){//Reset everyday at 6
        dailytotal=0;
      }
      if(now.Hour()>=3 && now.Hour()<=7){//11~15
        if (dailytotal<=600){
          notification(dailytotal,600);
        }  
      }      
      if(now.Hour()>=7 && now.Hour()<=11){//15~19
        if (dailytotal<=1000){
          notification(dailytotal,1000);
        }
      }
      if(now.Hour()>=11 && now.Hour()<=16){//20~24
        if (dailytotal<=1500){
          notification(dailytotal,1500);
        }
      }
      if(now.Hour()>=16){//24
        if (dailytotal<=2000){
          notification(dailytotal,2000);
        }
      }  
      displaydefault(dailytotal,temp);
    }
  printDateTime(now);
  }
}

void pulseCounter() //Function for flow meter
{
  pulseCount++;
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{ //Function for displaying datetime
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u:%02u:%02u"),
             dt.Hour()+8,
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);
  Serial.print("\n");
}

void printDateTimeOnDisplay(Adafruit_SSD1306& display, const RtcDateTime& dt) {
  char timeString[20];

  snprintf_P(timeString,
             countof(timeString),
             PSTR("%02u:%02u:%02u"),
             dt.Hour() + 8,
             dt.Minute(),
             dt.Second());
  display.print(timeString);
}

//Screen after drinking
void displaydrank(int drank, int total, const RtcDateTime& dt) {
  display.clearDisplay(); //
  display.setTextColor(1); // 
  display.setCursor(10, 8); // 
  display.print("Drank ");
  display.print(drank);
  display.print("mL"); // 
  display.setCursor(10, 20);
  display.print("At ");
  printDateTimeOnDisplay(display, dt);
  display.setCursor(10, 40); // 
  display.print("Total today: ");
  display.print(total);
  display.print("mL"); // 
  display.display();
}

//Screen in idle state
void displaydefault(int total, int temp) {
  display.clearDisplay();

  display.setTextColor(2);

  display.setCursor(10, 40); 
  display.print("Total today: ");
  display.print(total);
  display.print("mL"); 
  display.setCursor(10, 50);
  display.print("Temperature:");
  display.print(temp);
  display.print("C");
  display.display();
}
//Screen in notification state
void notification(int total, int target) {
  for(int i=0;i<3;i++){
    display.clearDisplay();
    display.setTextColor(1);  
    display.setCursor(10, 8); 
    display.print("DRINK!!!!!!!");
    display.setCursor(10, 20);
    display.print("You only drank");
    display.print(total);
    display.print("mL");
    display.setCursor(10, 32);
    display.print("You should drink");
    display.setCursor(10, 40);
    display.print(target);
    display.print("mL");
    display.display();
    digitalWrite(7,HIGH);
    delay(1500);
    display.clearDisplay();
    digitalWrite(7,LOW);
    delay(1000);
  }
}