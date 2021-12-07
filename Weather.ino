// Based on GxEPD2_HelloWorld.ino by Jean-Marc Zingg
// Waveshare 7.5 800x480 e-paper
// Waveshare ESP8266 e-paper board

#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>


#include "bitmaps.h"

const char * ssid = ""; // your network SSID (name)
const char * pass = "!";  // your network password

const String APIKEY = "";  //your api key
const String CityID = ""; //your city id

//Week Days //Month names
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

//Display Settings
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_750_T7
#ifndef EPD_CS
#define EPD_CS SS
#endif
#define MAX_DISPLAY_BUFFER_SIZE (81920ul-34000ul-5000ul) // ~34000 base use, change 5000 to your application use
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=15*/ EPD_CS, /*DC=4*/ 4, /*RST=2*/ 2, /*BUSY=5*/ 5));

//OpenWeather API settings
char servername[]="api.openweathermap.org"; 
String result;
int  counter = 60;

String Description;
float Temperature;
String Humidity;
String Windspeed;

//Time Settings
String currentTime;
String daytime;
String N = "N";
String D = "D";
String Day;
String Number;
String Month;
String currentHour;
int Hour;

WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  }
  
  getWeatherData();
  getTimeData();
  display.init();
  DrawDisplay();
  display.hibernate();
  WiFi.mode(WIFI_OFF);
  Serial.print("I'm awake, but I'm going into deep sleep mode for 30 minutes");
  ESP.deepSleep(30 * 60 * 1000 * 1000); 
  
}

void DrawDisplay()
{
  display.setRotation(1);
  display.setFont(&FreeSansBold18pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();

  //make X and Y coords center to the middle
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(Description, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;

  do
  {
    display.fillScreen(GxEPD_WHITE);

    //draw line
    display.fillRect(0, 770, 480, 2, GxEPD_BLACK);
    
    //Set text
    display.setTextSize(2);
    display.setCursor(30, 103);
    display.print(Day);   
    display.print(" ");  
    display.print(Number);   
    display.setCursor(30, 175);
    display.print(Month);   
    
    display.setCursor(60, 345);
    display.setTextSize(2);
    display.print(Temperature, 0);
    display.print("`");
    display.setTextSize(1);

    display.setCursor(x, 506);
    display.print(Description);

    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(75, 695);
    display.print("Windspeed");
    display.setCursor(105, 718);
    display.print(Windspeed);

    display.setCursor(325, 695);
    display.print("Humidity");
    display.setCursor(350, 718);
    display.print(Humidity);
    display.print("%");

    display.setCursor(145, 790);
    display.print("Last update on: ");
    display.print(currentTime);

    //Draw image - converted on https://javl.github.io/image2cpp/
      
      //Check if its day or night
      currentHour = currentTime.substring(0,2);
      Hour = currentHour.toInt();
      if (Hour >=18 or Hour <=5) {daytime = "N";} else {daytime = "D";}
      
      //ClearIcon
      if(Description.indexOf("clear sky") >=0 and daytime.equals(D)) {display.drawBitmap(210, 195, SunIcon, 256, 256, 0);}
      if(Description.indexOf("clear sky") >=0 and daytime.equals(N)) {display.drawBitmap(210, 195, MoonIcon, 256, 256, 0);}

      //CloudIcon
      if(Description.indexOf("few clouds") >=0 and daytime.equals(D)) {display.drawBitmap(210, 195, LightCloudIconD, 256, 256, 0);} 
      if(Description.indexOf("few clouds") >=0 and daytime.equals(N)) {display.drawBitmap(210, 195, LightCloudIconN, 256, 256, 0);}
      if(Description.indexOf("scattered clouds") >=0){display.drawBitmap(210, 195, CloudIcon, 256, 256, 0);}
      if(Description.indexOf("broken clouds") >=0){display.drawBitmap(210, 195, CloudsIcon, 256, 256, 0);}
      if(Description.indexOf("overcast clouds") >=0){display.drawBitmap(210, 195, CloudsIcon, 256, 256, 0);}
      
      //StormIcon
      if(Description.indexOf("thunderstorm with light rain") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      if(Description.indexOf("thunderstorm with rain") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      if(Description.indexOf("thunderstorm with heavy rain") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      if(Description.indexOf("light thunderstorm") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      if(Description.indexOf("thunderstorm") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      if(Description.indexOf("heavy thunderstorm") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      if(Description.indexOf("thunderstorm with light drizzle") >=0){display.drawBitmap(210, 190, StormIcon, 256, 256, 0);}
      if(Description.indexOf("ragged thunderstorm") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      if(Description.indexOf("thunderstorm with drizzle") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      if(Description.indexOf("thunderstorm with heavy drizzle") >=0){display.drawBitmap(210, 195, StormIcon, 256, 256, 0);}
      
      //RainIcon
      if(Description.equals("rain")){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("light intensity drizzle") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("drizzle") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("heavy intensity drizzle") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("light intensity drizzle rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("heavy intensity drizzle rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("drizzle rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("shower rain and drizzle") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("heavy shower rain and drizzle") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("light rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("moderate rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("heavy intensity rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("very heavy rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("extreme rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("freezing rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("light intensity shower rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("shower rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("heavy intensity shower rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      if(Description.indexOf("ragged shower rain") >=0){display.drawBitmap(210, 195, RainIcon, 256, 256, 0);}
      
      //SnowIcon
      if(Description.indexOf("light snow") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("snow") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("heavy snow") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("sleet") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("light shower sleet") >=0){display.drawBitmap(200, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("shower sleet") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("light rain and snow") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("rain and snow") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("light shower snow") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("shower snow") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      if(Description.indexOf("heavy shower snow") >=0){display.drawBitmap(210, 195, SnowIcon, 256, 256, 0);}
      
      //MistIcon
      if(Description.indexOf("mist") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("smoke") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("haze") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("sand/ dust whirls") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("fog") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("sand") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("dust") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("volcanic ash") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("squalls") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}
      if(Description.indexOf("tornado") >=0){display.drawBitmap(210, 195, MistIcon, 256, 256, 0);}

      //Small icons
      {display.drawBitmap(61, 550, SpeedIcon, 128, 128, 0);}
      {display.drawBitmap(293, 550, HumidIcon, 128, 128, 0);}
  }
  while (display.nextPage());
}

void getWeatherData() //GET request data
{
    if (client.connect(servername, 80)) {  
        client.println("GET /data/2.5/weather?id="+CityID+"&units=metric&APPID="+APIKEY);
        client.println("Host: api.openweathermap.org");
        client.println("User-Agent: ArduinoWiFi/1.1");
        client.println("Connection: close");
        client.println();
    } 
    else {
        Serial.println("connection failed"); //error message if no client connect
        Serial.println();
    }

    while(client.connected() && !client.available())
        delay(1); 

    while (client.connected() || client.available()) { 
        char c = client.read();
        result = result+c;
    }

    client.stop(); //stop client
    result.replace('[', ' ');
    result.replace(']', ' ');
    Serial.println(result);

    //Json part
    char jsonArray [result.length()+1];
    result.toCharArray(jsonArray,sizeof(jsonArray));
    jsonArray[result.length() + 1] = '\0';

    StaticJsonDocument<1024> root;
    DeserializationError error = deserializeJson(root, jsonArray);

if (error) {
  Serial.print("deserializeJson() failed with code ");
}

    float temperature = root["main"]["temp"];
    String humidity = root["main"]["humidity"];
    String description = root["weather"]["description"];
    String windspeed = root["wind"]["speed"];
    
    Temperature = round(temperature);
    Humidity = humidity;
    Description = description;
    Windspeed = windspeed;
}

void getTimeData()
{
  timeClient.begin();
  timeClient.setTimeOffset(3600);
  timeClient.update();
  delay(5); 
  timeClient.update();
  delay(5); 
  currentTime = timeClient.getFormattedTime();

  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  String weekDay = weekDays[timeClient.getDay()];
  
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;

  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Day = String(weekDay);
  Number = String(monthDay);
  Month = String(currentMonthName);
  
}

void loop() {
};
