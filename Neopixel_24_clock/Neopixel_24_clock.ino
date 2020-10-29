/**
 *BOARD configuration in Arduino IDE: 
 *  Lolin(Wemos) D1 R2 & mini on....
 *  Upload speed: 921600
 *  CPU freq 80MHz
 *  Erase Flash: Only Sketch - (if erase all the flash, then make sure that 
 *              the flash config info from the software is written with good values!)
 *  
 */
 
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>              // https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <EEPROM.h>
#include <PageBuilder.h>

#include "index.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define HOUR_PIN D2
#define MINUTES_PIN D3

#define STRIP_HOUR_DISPLACEMENT (12)
#define STRIP_MINUTES_DISPLACEMENT (30)


// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
// Even if my HW type is FC16, I am using the ICSTATION because it seemed that 
// I installed it rotated with a 180 degrees.... using ICSATATION it will display
// the text as it is required
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 1

#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D6

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel hoursStrip = Adafruit_NeoPixel(24, HOUR_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel minutesStrip = Adafruit_NeoPixel(60, MINUTES_PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


WiFiUDP ntpUdp;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
// ro - time - 30 minutes ntp query
NTPClient timeClient(ntpUdp, "europe.pool.ntp.org", 3*3600, 30*60000);

uint8_t crtHour, prevHour, crtMin;

// variables used for automatic night mode
uint8_t prevItsH, prevItsM, prevItsD; // previous intensities of H, M and D displays, used to restore from night-mode
volatile bool prevBrightnessSaved;


struct config_t {
  bool hoursEnabled;    // hours stripe enabled?
  uint8_t hoursBrightness;  // hour strip brightness
  bool minutesEnabled;  // minutes stripe enabled?
  uint8_t minutesBrightness; // minutes strip brightness
  bool displayEnabled;  // central display enabled?
  uint8_t displayBrightness; // display brightness
  bool displayTime;     // used to decide if time is displayed
  bool displayDate;     // used to decide if date is displayed
  uint16_t scrollSpeed; // scrolling speed
  uint16_t utcTimeOffset; // UTC time offset in minutes
  
  bool autoNightModeEnabled;  // automatic brightness dimming on the specified hours
  String autoNightStartTime;
  String autoNightEndTime;
};

config_t eepromConfig;

ESP8266WebServer server(80);



// Get the hours strip enabled status
String getHourStripEnabled(PageArgument& args) {
  return (eepromConfig.hoursEnabled) ? String("checked") : String("");
}
String getHourStripIntensityValue(PageArgument& args) {
  return String(eepromConfig.hoursBrightness);
}
String getMinutesStripEnabled(PageArgument& args) {
  return (eepromConfig.minutesEnabled) ? String("checked") : String("");
}
String getMinutesStripIntensityValue(PageArgument& args) {
  return String(eepromConfig.minutesBrightness);
}

String getDisplayEnabled(PageArgument& args) {
  return (eepromConfig.displayEnabled) ? String("checked") : String("");
}
String getDisplayTimeEnabled(PageArgument& args) {
  return (eepromConfig.displayTime) ? String("checked") : String("");
}
String getDisplayDateEnabled(PageArgument& args) {
  return (eepromConfig.displayDate) ? String("checked") : String("");
}
String getDisplayIntensityValue(PageArgument& args) {
  return String(eepromConfig.displayBrightness);
}
String getDisplayScrollSpeedValue(PageArgument& args) {
  return String(eepromConfig.scrollSpeed);
}

String getUTCTimeOffset(PageArgument& args) {
  return String(eepromConfig.utcTimeOffset);
}

String getAutoNightEnabled(PageArgument& args) {
  return (eepromConfig.autoNightModeEnabled) ? String("checked") : String("");
}
String getAutoNightStartValue(PageArgument& args) {
  return String(eepromConfig.autoNightStartTime);
}
String getAutoNightEndValue(PageArgument& args) {
  return String(eepromConfig.autoNightEndTime);
}

// Page construction
PageElement MainPage(webPage, {
  {"CLOCK_ENABLE_CHECKED", getHourStripEnabled},
  {"HOUR_STRIP_INTENSITY_VALUE", getHourStripIntensityValue},
  {"MINUTES_ENABLED_CHECKED", getMinutesStripEnabled},
  {"MINUTES_STRIP_INTENSITY_VALUE", getMinutesStripIntensityValue},
  {"DISPLAY_ENABLED_CHECKED", getDisplayEnabled},
  {"DISPLAY_ENABLED_TIME_CHECKED", getDisplayTimeEnabled},
  {"DISPLAY_ENABLED_DATE_CHECKED", getDisplayDateEnabled},
  {"DISPLAY_INTENSITY_VALUE", getDisplayIntensityValue},
  {"SCROLL_SPEED_VALUE", getDisplayScrollSpeedValue},
  {"UTC_TIME_OFFSET", getUTCTimeOffset},
  
  {"AUTO_NIGHT_MODE_CHECKED", getAutoNightEnabled},
  {"AUTO_TIME_START_TIME", getAutoNightStartValue},
  {"AUTO_TIME_END_TIME", getAutoNightEndValue}
});
PageBuilder NTPPage("/", {MainPage});



void setup() {
  Serial.begin(115200); // just in case... 
    
  EEPROM.begin(512);

  /*
  for (int i = 0; i < 0xF; i++)
    EEPROM.write(0x0F+i, 0); EEPROM.commit(); // only to force eeprom rewrite "reset"
  */
  
  EEPROM.get(0x0F, eepromConfig);

  // default settings if the scroll speed is zero?
  if (eepromConfig.scrollSpeed == 0) {  
    eepromConfig.hoursEnabled = true;
    eepromConfig.hoursBrightness = 25;
    eepromConfig.minutesEnabled = true;
    eepromConfig.minutesBrightness = 25;
    eepromConfig.displayEnabled = true;
    eepromConfig.displayBrightness = 5;
    eepromConfig.displayTime = true;
    eepromConfig.displayDate = false;
    eepromConfig.autoNightModeEnabled = true;
    eepromConfig.scrollSpeed = 40;
    eepromConfig.utcTimeOffset = 3*3600;
    eepromConfig.autoNightStartTime = "20:00";
    eepromConfig.autoNightEndTime = "07:00";

    // now write those back to the EEPROM
    EEPROM.put(0x0F, eepromConfig);
    EEPROM.commit(); 
  }


  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  
  //WiFi.begin(ssid, password);
  WiFiManager wm;


  // wipe settings for testing purposes!
  //wm.resetSettings();
  
  mx.begin();

  timeClient.begin();  
  
  hoursStrip.begin();
  hoursStrip.setBrightness(eepromConfig.hoursBrightness);
  hoursStrip.clear();  
  hoursStrip.show(); // Initialize all pixels to 'off'

  minutesStrip.begin();
  minutesStrip.setBrightness(eepromConfig.minutesBrightness);
  minutesStrip.clear();  
  minutesStrip.show();

  mx.control(MD_MAX72XX::INTENSITY, eepromConfig.displayBrightness);
  
  scrollText("electroPi NTP Clock V1.0");

  bool res = wm.autoConnect("electroPi_NTP_clock","electroPi"); // password protected ap
  if(!res) {
    scrollText("Failed to connect to network! Resetting... ");
    delay(2000);
    ESP.reset();
  } 
  else {
        //if you get here you have connected to the WiFi    
      //scrollText("connected to the network.");
  }
    
  // default init values
  crtHour = 0;
  crtMin = 0;
  prevHour = 0;

  prevBrightnessSaved = false;

  

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    scrollText("Connecting to WiFi...");
  }  

  /* OTA Setup  */
  //ArduinoOTA.setHostname("ePi_NTP_clk");
  //ArduinoOTA.setPassword("electroPi");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    // stop everything else!
    eepromConfig.hoursEnabled = false;
    eepromConfig.minutesEnabled = false;
    eepromConfig.displayEnabled = false;
    eepromConfig.displayTime = false;
    eepromConfig.displayDate = false;

    
    //String s = "Starting firmware update " + type;
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    // do not waste the time with scrolling! it is critical due to communication timeouts!
    //scrollText(s.c_str());
  });
  ArduinoOTA.onEnd([]() {
    scrollText("FW Updated!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char msg[10];
    sprintf(msg, "%u%%", (progress / (total / 100)) );
    //scrollText(msg);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) {
      scrollText("FW Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      scrollText("FW Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      scrollText("FW Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      scrollText("FW Receive Failed");
    } else if (error == OTA_END_ERROR) {
      scrollText("FW End Failed");
    }
  });
  
  ArduinoOTA.begin();
  



  /* WebServer related stuff */

  if (MDNS.begin("ePi_NTP_clk")) {
    scrollText("MDNS responder started");
  }

  NTPPage.insert(server);
  
//  server.on("/", [](){
//    server.send(200, "text/html", webPage);
//    });


  server.on("/reboot", []() {
    server.send(200, "text/html", "OK! Restarting now.... <br> <button onclick='window.history.back();'>Back</button>");
    delay(1000);
    ESP.restart();
  });
  
  server.on("/poweroff", []() {
    server.send(200, "text/html", "OK! System will be powered off! <br> <button onclick='window.history.back();'>Back</button>");
    // force to true, then call the toggle functions to clear up the displays
    eepromConfig.hoursEnabled = true;
    eepromConfig.minutesEnabled = true;
    eepromConfig.displayEnabled = true;
    hoursEnabledToggle();
    minutesEnabledToggle();
    displayEnabledToggle();
    ESP.deepSleep(0);
  });

  server.on("/postUTCTimeOffset/", []() {
    server.send(200, "text/html", "OK! <br> <button onclick='window.history.back();'>Back</button>");
    eepromConfig.utcTimeOffset = server.arg(0).toInt();
    timeClient.setTimeOffset(eepromConfig.utcTimeOffset);
    timeClient.forceUpdate();
  });


  server.on("/saveEEPROM", []() {
    server.send(200, "text/html", "OK! <button onclick='window.history.back();'>Back</button>");
    EEPROM.put(0x0F, eepromConfig);
    EEPROM.commit();    
  });

  server.on("/toggleHours", []() {
    server.send(200, "text/html", "OK! <button onclick='window.history.back();'>Back</button>");
    hoursEnabledToggle();
  });

  server.on("/toggleMinutes", []() {
    server.send(200, "text/html",  "OK! <button onclick='window.history.back();'>Back</button>");
    minutesEnabledToggle();
  });

  server.on("/toggleDisplay", []() {
    server.send(200, "text/html",  "OK! <button onclick='window.history.back();'>Back</button>");
    displayEnabledToggle();
  });
  
  server.on("/postMinutesIntensity/", []() {
    server.send(200, "text/html",  "OK! <button onclick='window.history.back();'>Back</button>");
    String msg = server.arg(0);
    eepromConfig.minutesBrightness = msg.toInt();
    minutesStrip.setBrightness(eepromConfig.minutesBrightness);
  });

  server.on("/postHoursIntensity/", []() {
    server.send(200, "text/html",  "OK! <button onclick='window.history.back();'>Back</button>");
    String msg = server.arg(0);
    eepromConfig.hoursBrightness = msg.toInt();
    hoursStrip.setBrightness(eepromConfig.hoursBrightness);
  });

  server.on("/postDisplayIntensity/", []() {
    server.send(200, "text/html",  "OK! <button onclick='window.history.back();'>Back</button>");
    String msg = server.arg(0);
    eepromConfig.displayBrightness = msg.toInt();
    mx.control(MD_MAX72XX::INTENSITY, eepromConfig.displayBrightness);
  });

  server.on("/toggleDisplayEnableTime", []() {
    server.send(200, "text/html",  "OK! <button onclick='window.history.back();'>Back</button>");
    eepromConfig.displayTime = !eepromConfig.displayTime;
  });

  server.on("/toggleDisplayEnableDate", []() {
    server.send(200, "text/html",  "OK! <button onclick='window.history.back();'>Back</button>");
    eepromConfig.displayDate = !eepromConfig.displayDate;
  });


  server.on("/postAutoDim/", []() {
    eepromConfig.autoNightStartTime = server.arg(0);
    eepromConfig.autoNightEndTime = server.arg(1);
    String responseString ;

    if (eepromConfig.autoNightStartTime.equals(eepromConfig.autoNightEndTime) ) {
      eepromConfig.autoNightModeEnabled = false;
      responseString = "OK! Auto Night Mode is DISABLED!";
    } else {
      eepromConfig.autoNightModeEnabled = true;
      responseString = "OK! Display will be dimmed from: " + eepromConfig.autoNightStartTime + " until: "+ eepromConfig.autoNightEndTime;
    }

    responseString += "<br> <button onclick='window.history.back();'>Back</button>";
    server.send(200, "text/html",  responseString);
  });
  
  server.on("/postDisplayScrollSpeed/", []() {
    server.send(200, "text/html",  "OK! <button onclick='window.history.back();'>Back</button>");
    String msg = server.arg(0);
    eepromConfig.scrollSpeed = msg.toInt();
  });



  server.begin();

  String ip = "IP: " + WiFi.localIP().toString(); 
  scrollText(ip.c_str());  

  timeClient.setTimeOffset(eepromConfig.utcTimeOffset);
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  MDNS.update();
  timeClient.update();

  
  if (eepromConfig.hoursEnabled == true) {
    crtHour = timeClient.getHours();
    crtHour = (crtHour+STRIP_HOUR_DISPLACEMENT) % 24;
    hoursStrip.setPixelColor(crtHour, hoursStrip.Color(0, 0, 255));
    hoursStrip.show();
  }
  server.handleClient();

  if (eepromConfig.minutesEnabled == true) {
    crtMin = timeClient.getMinutes();
    crtMin = (crtMin + STRIP_MINUTES_DISPLACEMENT) % 60;

    minutesStrip.setPixelColor(crtMin, minutesStrip.Color(0,0,255) );
    minutesStrip.show();
  }
  server.handleClient();

  rainbowTime(crtHour, crtMin, 10);

  server.handleClient();  
  if (eepromConfig.displayEnabled == true) {
    String timeString, dayString, fullString;
    fullString = timeClient.getFormattedDate();
    // 2018-05-28T16:00:13Z
    // extract date
    int t = fullString.indexOf("T");
    dayString = fullString.substring(0, t);

    // Extract time
    timeString = fullString.substring(t+1, fullString.length()-1);

    if (eepromConfig.displayTime == true) {
      timeString = "Time: " + timeString;
      scrollText(timeString.c_str());
    }
    
    server.handleClient();
    if (eepromConfig.displayDate == true) {
      dayString = "Date: " + dayString;
      scrollText(dayString.c_str());
    }
  }

  
  if (prevHour != crtHour) {
    if (eepromConfig.hoursEnabled == true) {
      hoursStrip.clear();
      hoursStrip.show();
    }

    if (eepromConfig.minutesEnabled == true) {
      minutesStrip.clear();
      minutesStrip.show();
    }
    
    prevHour = crtHour;
  }

  if (eepromConfig.autoNightModeEnabled == true) {
    autoNightModeHandler();    
  }
  
}

void rainbowTime(uint8_t hourPos, uint8_t minutesPos, uint8_t wait) {
  uint16_t i, j;
  uint8_t hourMax = ( (hourPos+STRIP_HOUR_DISPLACEMENT) % 24 ) ;
  uint8_t minutesMax = ( (minutesPos+STRIP_MINUTES_DISPLACEMENT) % 60);

  for(j = 0; j < 256; j++) {
    if (eepromConfig.hoursEnabled == true) {
      for(i = 1; i <  hourMax; i++) {
        hoursStrip.setPixelColor((i+STRIP_HOUR_DISPLACEMENT) % 24, Wheel((i+j) & 255));
      }
      hoursStrip.show();
    }

    if (eepromConfig.minutesEnabled == true) {
      for(i = 0; i < minutesMax ; i++) {
        minutesStrip.setPixelColor((i+STRIP_MINUTES_DISPLACEMENT) % 60, Wheel((i+j) & 255));
      }
      minutesStrip.show();
    }
    
    delay(wait);
  }  
}
  
void scrollText(const char *p) {
  uint8_t charWidth;
  uint8_t cBuf[8];  // this should be ok for all built-in fonts

  mx.clear();

  while (*p != '\0')
  {
    charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
    
    for (uint8_t i=0; i<=charWidth; i++)  // allow space between characters
    {
      mx.transform(MD_MAX72XX::TSL);
      if (i < charWidth)
        mx.setColumn(0, cBuf[i]);
      delay(eepromConfig.scrollSpeed);
    }
  }
  mx.clear();  
}

void hoursEnabledToggle() {
  eepromConfig.hoursEnabled = !eepromConfig.hoursEnabled;

  if (eepromConfig.hoursEnabled == false) {
    for(int8_t i= hoursStrip.numPixels(); i >= 0; i--) {
      hoursStrip.setPixelColor(i, hoursStrip.Color(0, 0, 0));
      hoursStrip.show();
      delay(50);
    }
  }
}

void minutesEnabledToggle() {
  eepromConfig.minutesEnabled = !eepromConfig.minutesEnabled;

  if (eepromConfig.minutesEnabled == false) {
    for(int8_t i= minutesStrip.numPixels(); i >= 0; i--) {
      minutesStrip.setPixelColor(i, minutesStrip.Color(0, 0, 0));
      minutesStrip.show();
      delay(50);
    }
  }
}

void displayEnabledToggle() {
  eepromConfig.displayEnabled = !eepromConfig.displayEnabled;
  mx.clear();
}

void autoNightModeHandler() {
  String nowS = timeClient.getFormattedTime();
  bool condition = false;

  if (eepromConfig.autoNightStartTime < eepromConfig.autoNightEndTime) {
    condition = (nowS > eepromConfig.autoNightStartTime ) && (nowS < eepromConfig.autoNightEndTime);
  }

  if (eepromConfig.autoNightStartTime > eepromConfig.autoNightEndTime) {
    condition = !( (nowS < eepromConfig.autoNightStartTime ) && (nowS > eepromConfig.autoNightEndTime) );
  }
  //if ( (nowS > eepromConfig.autoNightStartTime ) && (nowS < eepromConfig.autoNightEndTime) ) {
  //if ( !( (nowS < eepromConfig.autoNightStartTime ) && (nowS > eepromConfig.autoNightEndTime) ) ) {
  if (condition) {

    if (prevBrightnessSaved == false) {
      prevItsH = hoursStrip.getBrightness();
      prevItsM = minutesStrip.getBrightness();
      prevItsD = 5; //mx.getIntensity(); we don't have a routine in this lib, so use the default one
      prevBrightnessSaved = true;

      // put everything on minimum luminosity/brightness
      minutesStrip.setBrightness(1);
      hoursStrip.setBrightness(1);
      mx.control(MD_MAX72XX::INTENSITY, 1);
    }

  } else {      
    if (prevBrightnessSaved == true) {
      minutesStrip.setBrightness(prevItsM);
      hoursStrip.setBrightness(prevItsH);
      mx.control(MD_MAX72XX::INTENSITY, prevItsD);
      prevBrightnessSaved = false;
      prevItsM = 0;
      prevItsH = 0;
      prevItsD = 0;
    }
  }
}




























// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return hoursStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return hoursStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return hoursStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
