/*
The MIT License (MIT)

Copyright © 2018 Médéric NETTO

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <ArduinoJson.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include <TimeLib.h>

#define TFT_CS   27
#define TFT_DC   26
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  5
#define TFT_MISO 12
//#define TFT_LED   5  // GPIO not managed by library

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

/*
#define TFT_CS  22
#define TFT_DC  21
#define TFT_RST  5

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
*/
const char* ssid = "Your WiFI ID";
const char* password = "Your WiFi password";

const char host[] = "api.coinmarketcap.com";

 // Colors
 int ILI9341_COLOR;
 #define CUSTOM_DARK 0x4228 // Background color

 // Bitmap_WiFi
 extern uint8_t wifi_1[];
 extern uint8_t wifi_2[];
 extern uint8_t wifi_3[];

 // Bitmap_CryptoMonnaies
 extern uint8_t bitcoin[];
 extern uint8_t ethereum[];
 extern uint8_t litecoin[];
 extern uint8_t ripple[];
 extern uint8_t cardano[];

 unsigned long previousMillis = 0;
 long interval = 0;

 int coin = 0;
 String crypto[] = {"bitcoin", "ethereum", "ripple", "litecoin", "cardano"};
 String oldPrice[5];

 void setup() {

  Serial.begin(115200);

  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(CUSTOM_DARK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(0, 170);
  tft.setTextSize(2);
 
  tft.println(">>> Connecting to: ");
  tft.println(" ");
  tft.println(ssid);
  Serial.println("start");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      tft.drawBitmap(70, 50, wifi_1, 100, 100, ILI9341_WHITE);
      delay(200);
      tft.drawBitmap(70, 50, wifi_2, 100, 100, ILI9341_WHITE);
      delay(200);
      tft.drawBitmap(70, 50, wifi_3, 100, 100, ILI9341_WHITE);
      delay(200);
      tft.fillRect(70, 50, 100, 100, CUSTOM_DARK);
  }
  //Serial.println("start");
  tft.println(" ");
  tft.print(">>> WiFi connected");
  //tft.print("IP address: ");
  //tft.println(WiFi.localIP());

  delay(1500);
  tft.fillScreen(CUSTOM_DARK); // Clear Screen
  tft.setTextColor(ILI9341_BLUE);
  tft.setCursor(0, 150);
  tft.setTextSize(2);
  tft.println("CryptoMoneyTicker_v1");
  tft.drawLine(0,130,240,130, ILI9341_WHITE);
  tft.drawLine(0,185,240,185, ILI9341_WHITE);

  tft.setTextSize(1);
  tft.setCursor(5, 307);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("Data from: CoinMarketCap.com");
}

void loop() {

tft.setTextColor(ILI9341_WHITE);
tft.setCursor(1, 10);

unsigned long currentMillis = millis();

if (currentMillis - previousMillis >= interval) {

  previousMillis = currentMillis;
  interval = 60000; //                              <<<--------------------

  if(coin == 5 ){
    coin = 0;
  }

//Serial.print(">>> Connecting to ");
//Serial.println(host);

WiFiClientSecure client;

const int httpsPort = 443;
if (!client.connect(host, httpsPort)) {
  tft.fillScreen(CUSTOM_DARK);
  tft.println(">>> Connection failed");
  return;
}

//Serial.print("Requesting URL: ");
//Serial.println("Connected to server!");

client.println("GET /v1/ticker/" + crypto[coin] + "/ HTTP/1.1");
client.println("Host: api.coinmarketcap.com");
client.println("Connection: close");
client.println();

unsigned long timeout = millis();
while (client.available() == 0) {
  if (millis() - timeout > 5000) {
    tft.fillScreen(CUSTOM_DARK);
    tft.println(">>> Client Timeout!");
    client.stop();
    return;
}
}

String data;
while(client.available()) {
  data = client.readStringUntil('\r');
  //Serial.println(data);
}

data.replace('[', ' ');
data.replace(']', ' ');

char buffer[data.length() + 1];
data.toCharArray(buffer, sizeof(buffer));
buffer[data.length() + 1] = '\0';

const size_t bufferSize = JSON_OBJECT_SIZE(15) + 110;
DynamicJsonBuffer jsonBuffer(bufferSize);

JsonObject& root = jsonBuffer.parseObject(buffer);

if (!root.success()) {
  tft.println("parseObject() failed");
  return;
}

String name = root["name"]; // "Bitcoin"
String symbol = root["symbol"]; // "BTC"
String price_usd = root["price_usd"]; // "573.137"
String percent_change_1h = root["percent_change_1h"]; // "0.04"
String last_updated = root["last_updated"]; // "1472762067" <-- Unix Time Stamp
String error = root["error"]; // id not found

printTransition();

switch (coin) {

  case 0 : // Bitcoin
  tft.drawBitmap(5, 5, bitcoin, 45, 45, ILI9341_ORANGE);
  printName(name, symbol);
  printPrice(price_usd);
  printChange(percent_change_1h);
  printTime(last_updated);
  printPagination();
  printError(error);
  tft.fillCircle(98, 300, 4, ILI9341_WHITE);
  break;

  case 1 : // Ethereum
  tft.drawBitmap(5, 5, ethereum, 45, 45, ILI9341_BLACK);
  printName(name, symbol);
  printPrice(price_usd);
  printChange(percent_change_1h);
  printTime(last_updated);
  printPagination();
  printError(error);
  tft.fillCircle(108, 300, 4, ILI9341_WHITE);
  break;

  case 2 : // Ripple
  tft.drawBitmap(5, 5, ripple, 45, 45, ILI9341_NAVY);
  printName(name, symbol);
  printPrice(price_usd);
  printChange(percent_change_1h);
  printTime(last_updated);
  printPagination();
  printError(error);
  tft.fillCircle(118, 300, 4, ILI9341_WHITE);
  break;

  case 3 : // Litecoin
  tft.drawBitmap(5, 5, litecoin, 45, 45, ILI9341_LIGHTGREY);
  printName(name, symbol);
  printPrice(price_usd);
  printChange(percent_change_1h);
  printTime(last_updated);
  printPagination();
  printError(error);
  tft.fillCircle(128, 300, 4, ILI9341_WHITE);
  break;

  case 4 : // Cardano
  tft.drawBitmap(5, 5, cardano, 45, 45, ILI9341_CYAN);
  printName(name, symbol);
  printPrice(price_usd);
  printChange(percent_change_1h);
  printTime(last_updated);
  printPagination();
  printError(error);
  tft.fillCircle(138, 300, 4, ILI9341_WHITE);
  break;

  }

  oldPrice[coin] = price_usd;
  coin++;

  }
}

void printName(String name, String symbol) {

  tft.setTextSize(3);
  tft.setCursor(65, 10);
  tft.println(name);

  tft.setTextSize(2);
  tft.setCursor(65, 33);
  tft.print(symbol);

  tft.drawLine(65, 54, 240, 54, ILI9341_WHITE);
}

void printPrice(String price_usd) {

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(5, 75);
  tft.print("Price:");

  if(price_usd != oldPrice[coin]){
    if(price_usd > oldPrice[coin]){

    ILI9341_COLOR = ILI9341_GREENYELLOW;
    }else{

    ILI9341_COLOR = ILI9341_RED;
    }
  }

  tft.setTextSize(3);
  tft.setTextColor(ILI9341_COLOR);
  tft.setCursor(40, 110);
  tft.print("$");
  tft.println(price_usd);
}

void printChange(String percent_change_1h) {

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(5, 150);
  tft.print("Change(1h):");

  if(percent_change_1h >= "0"){

    tft.setTextColor(ILI9341_GREENYELLOW);

    }else{

    tft.setTextColor(ILI9341_RED);
  }

  tft.setTextSize(3);
  tft.setCursor(40, 180);
  tft.print(percent_change_1h);
  tft.print("%");
}

void printTime(String last_updated) {

  long int timeData = last_updated.toInt();
  time_t t = timeData;
  //time_t Z = 1515675789;

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(5, 220);
  tft.print("Last Updated:");

  tft.setTextSize(3);
  tft.setCursor(40, 250);
  /*
  printDigits(day(t));
  tft.print("/");
  printDigits(month(t));
  tft.print("/");
  tft.print(year(t));
  tft.print(" ");
  */
  printDigits(hour(t) + 1); // +1 for the French time
  tft.print(":");
  printDigits(minute(t));
  //tft.print(":");
  //printDigits(second(t));
}

void printPagination() {

  tft.drawCircle( 98, 300, 4, ILI9341_WHITE);
  tft.drawCircle(108, 300, 4, ILI9341_WHITE);
  tft.drawCircle(118, 300, 4, ILI9341_WHITE);
  tft.drawCircle(128, 300, 4, ILI9341_WHITE);
  tft.drawCircle(138, 300, 4, ILI9341_WHITE);
}

void printError(String error) {

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(65, 22);
  tft.println(error);
}

void printTransition(){

  tft.fillScreen(CUSTOM_DARK);
  yield();
  tft.fillScreen(ILI9341_RED);
  yield();
  tft.fillScreen(ILI9341_GREEN);
  yield();
  tft.fillScreen(ILI9341_BLUE);
  yield();
  tft.fillScreen(CUSTOM_DARK);
  yield();
}

void printDigits(int digits) {
 // utility function for digital clock display: prints preceding colon and leading 0
 if (digits < 10)
 tft.print('0');
 tft.print(digits);
}
