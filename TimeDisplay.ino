#include <WiFi.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "time.h"
#include <stdlib.h>
#include "esp_wpa2.h"   // wpa2 for connection to enterprise networks

#define EAP_IDENTITY "zcabgzh@ucl.ac.uk"                
#define EAP_PASSWORD "Jiemin76!"

const char* essid = "eduroam";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;
const uint16_t color_arr[5] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_YELLOW, TFT_BLUE};


TFT_eSPI tft = TFT_eSPI();

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    tft.println("Failed to obtain time");
    return;
  }
  int random = rand() % 6;  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(color_arr[random],TFT_BLACK); 
  tft.setTextSize(2);
  tft.println(&timeinfo, "%A");

  tft.setCursor(0, 30, 2);
  tft.setTextColor(color_arr[random],TFT_BLACK);
  tft.setTextSize(1.5);
  tft.println(&timeinfo, "%B %d %Y");

  tft.setCursor(0, 50, 2);
  tft.setTextColor(color_arr[random],TFT_BLACK); 
  tft.setTextSize(1.5);
  tft.println(&timeinfo,"%H:%M:%S");
}

void setup() {
  tft.init();
  tft.setRotation(0);
  bool eduroamFound = false;
  // put your setup code here, to run once:
  tft.begin(115200);
  delay(10);

// Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Repeatedly scan until we see eduroam
  //
  while (!eduroamFound) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.setTextSize(1);
    tft.println("scan start");
    int n = WiFi.scanNetworks(); // WiFi.scanNetworks returns the number of networks found
    tft.println("scan done");
    
    if (n == 0) {
        tft.println("no networks found");
    } else {
        tft.print(n);
        tft.println(" networks found");
        
        for (int i = 0; i < n; ++i) {
            String ssid = WiFi.SSID(i);
            int    rssi = WiFi.RSSI(i);
          
            // Print SSID and RSSI for each network found
            tft.print(i + 1);
            tft.print(": ");
            tft.print(ssid);
            tft.print(" (");
            tft.print(rssi);
            tft.print(")");
            tft.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
            
            ssid.trim();
            if (ssid == essid) {
              tft.print(" <==== eduroam found");
              eduroamFound = true;
            }
            tft.println("");
        }
    }
    tft.println("");

    // Wait a bit before scanning again
    if (!eduroamFound)
      delay(5000);
  }

  ////////////////////////////////////////////////////////////////////////////////
  //
  // If we come here, we've successfully found eduroam. Try to connect to it.
  //
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setTextSize(1);
  tft.print("Connecting to ");
  tft.println(essid);
  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // This is where the wpa2 magic happens to allow us to connect to eduroam
  //
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  
  WiFi.begin(essid);       //connect to eduroam
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
  }
  tft.println("");
  tft.print("WiFi is connected to ");
  tft.println(essid);
  tft.print("IP address: ");
  tft.println(WiFi.localIP()); //print LAN IP

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  printLocalTime();
}
