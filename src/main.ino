/*
 *   /$$$$$$                                  /$$               /$$      /$$                                      /$$$$$$$$ /$$           /$$
 *  /$$__  $$                                | $$              | $$$    /$$$                                     |__  $$__/|__/          | $$
 *  | $$  \__/  /$$$$$$  /$$   /$$  /$$$$$$  /$$$$$$    /$$$$$$ | $$$$  /$$$$  /$$$$$$  /$$$$$$$   /$$$$$$  /$$   /$$| $$    /$$  /$$$$$$$| $$   /$$  /$$$$$$   /$$$$$$
 *  | $$       /$$__  $$| $$  | $$ /$$__  $$|_  $$_/   /$$__  $$| $$ $$/$$ $$ /$$__  $$| $$__  $$ /$$__  $$| $$  | $$| $$   | $$ /$$_____/| $$  /$$/ /$$__  $$ /$$__  $$
 *  | $$      | $$  \__/| $$  | $$| $$  \ $$  | $$    | $$  \ $$| $$  $$$| $$| $$  \ $$| $$  \ $$| $$$$$$$$| $$  | $$| $$   | $$| $$      | $$$$$$/ | $$$$$$$$| $$  \__/
 *  | $$    $$| $$      | $$  | $$| $$  | $$  | $$ /$$| $$  | $$| $$\  $ | $$| $$  | $$| $$  | $$| $$_____/| $$  | $$| $$   | $$| $$      | $$_  $$ | $$_____/| $$
 *  |  $$$$$$/| $$      |  $$$$$$$| $$$$$$$/  |  $$$$/|  $$$$$$/| $$ \/  | $$|  $$$$$$/| $$  | $$|  $$$$$$$|  $$$$$$$| $$   | $$|  $$$$$$$| $$ \  $$|  $$$$$$$| $$
 *  \______/ |__/       \____  $$| $$____/    \___/   \______/ |__/     |__/ \______/ |__/  |__/ \_______/ \____  $$|__/   |__/ \_______/|__/  \__/ \_______/|__/
 *                    /$$  | $$| $$                                                                      /$$  | $$
 *                   |  $$$$$$/| $$                                                                     |  $$$$$$/
 *                    \______/ |__/                                                                      \______/
 *
 *  GitHub --> https://git.io/vN2d9
 *
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2018 Médéric NETTO
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include <ArduinoJson.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <mySD.h>

#include <TimeLib.h>

#include <Bounce2.h>

//#define LED_BUILTIN 22

#define TFT_CS   17  // TTGO_T4: 27
#define TFT_DC   16  //          26
#define TFT_RST   5

#define BUTTON_A  0  //          37 CENTRE
#define BUTTON_B 15  //          38 LEFT
#define BUTTON_C  4  //          39 RIGHT

#define SD_CS    26  //          13
#define SD_MOSI  14  //          15
#define SD_MISO  12  //           2
#define SD_SCLK  27  //          14

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

const char host[] = "api.coinmarketcap.com";
const int httpsPort = 443;

// Custom colors
int ILI9341_COLOR;
#define CUSTOM_DARK 0x3186 // Background color

// Bitmaps
extern uint8_t qrcode[];

extern uint8_t wifi_1[];
extern uint8_t wifi_2[];
extern uint8_t wifi_3[];

extern uint16_t bitcoin[];
extern uint16_t ethereum[];
extern uint16_t ripple[];
extern uint16_t litecoin[];
extern uint16_t dash[];

unsigned long previousMillis = 0;
long interval = 0;

File root;

#define NUM_BUTTONS 3
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {BUTTON_A, BUTTON_B, BUTTON_C};
Bounce * buttons = new Bounce[NUM_BUTTONS];

bool btnState1      = LOW;
bool btnState2      = LOW;
bool btnState3      = LOW;
bool antiFlickering = LOW;

// https://api.coinmarketcap.com/v2/listings/ for find the {id} of the currency
// Change the name of the currency and put the {id} in " "
#define BITCOIN     "1"
#define ETHEREUM "1027"
#define RIPPLE     "52"
#define LITECOIN    "2"
#define DASH      "131"

// and change again the name here
String crypto[] = {BITCOIN, ETHEREUM, RIPPLE, LITECOIN, DASH};
int coin = -1;
String oldPrice[5];

void setup() {

  Serial.begin(115200);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach(BUTTON_PINS[i] , INPUT_PULLUP);  //setup the bounce instance for the current button
    buttons[i].interval(25);  // interval in ms
  }

  tft.begin();
  tft.setRotation(0); // 0 = 0°  1 = 90°  2 = 180°  3 = 270°
  tft.fillScreen(CUSTOM_DARK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(0, 170);
  tft.setTextSize(2);

  tft.println(" Connecting to: ");
  tft.println(" ");
  tft.print(" "); tft.println(ssid);

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

  tft.println(" ");
  tft.println(" WiFi connected");
  //tft.print("IP address: ");
  //tft.println(WiFi.localIP());
  
  //Serial.print("Initializing SD card...");
  /* initialize SD library with SPI pins */
  if (!SD.begin(SD_CS, SD_MOSI, SD_MISO, SD_SCLK)) {
    //tft.setCursor(0, 170);
    tft.println(" ");
    tft.println(" SD Card failed!");
  }else{
    tft.println(" ");
    tft.println(" SD Card done.");
  }

  /* Begin at the root "/" */
  root = SD.open("/");
  if (root) {
    printDirectory(root, 0);
    root.close();
  } 
  /*
  else {
    //Serial.println("error opening data.csv");
  }
  */
  /* open "data.csv" for writing */
  root = SD.open("data.csv", FILE_WRITE);
  /* if open succesfully -> root != NULL
    then write something to it
  */
  if (root.size() == 0 ) {
    root.println("Date; Bitcoin; Ethereum; Ripple; Litecoin; Dash");
    root.flush();
    /* close the file */
   // root.close();
  } else {
    /* if the file open error, print an error */
    root.print("\n");
    root.flush();
    //Serial.println("File already done.");
  }

  delay(1500);
  tft.fillScreen(CUSTOM_DARK);
  tft.setCursor(20, 10);
  tft.println("CryptoMoneyTicker");
  tft.setCursor(95, 40);
  tft.println("v1.4");
  tft.drawBitmap(0, 60, qrcode, 240, 240, ILI9341_WHITE);
  delay(5000);
}

void loop() {

  buttonCheck();

  //unsigned long currentMillis = millis();

  if (btnState2 == HIGH || btnState3 == HIGH || (millis() - previousMillis) > interval) {
    previousMillis = millis();
    interval = 60000;

    if(btnState2 == LOW && btnState3 == LOW && btnState1 == LOW){
     coin++;
    }else{
     antiFlickering = HIGH;
    }

    if(coin > 4) {
     coin = 0;
    }else if(coin < 0){
      coin = 4;
    }

    //Serial.print(">>> Connecting to ");
    //Serial.println(host);

    WiFiClientSecure client;

    if (!client.connect(host, httpsPort)) {
    tft.fillScreen(CUSTOM_DARK);
    tft.println(" Connection failed");
    return;
    }

    //Serial.print("Requesting URL: ");
    //Serial.println("Connected to server!");
    client.println("GET /v2/ticker/" + crypto[coin] + "/ HTTP/1.1");
    client.println("Host: api.coinmarketcap.com");
    client.println("Connection: close");
    client.println();

   unsigned long timeout = millis();
   while (client.available() == 0) {
     if (millis() - timeout > 5000) {
       tft.fillScreen(CUSTOM_DARK);
       tft.println(" Client Timeout!");
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

    const size_t bufferSize = JSON_OBJECT_SIZE(21) + 400;
    DynamicJsonBuffer jsonBuffer(bufferSize);

    JsonObject& root = jsonBuffer.parseObject(buffer);

    if (!root.success()) {
     tft.println("parseObject() failed");
     return;
    }

    JsonObject& data0 = root["data"];
    String name = data0["name"];     // "Bitcoin"
    String symbol = data0["symbol"]; // "BTC"

    JsonObject& data1 = data0["quotes"]["USD"];
    String price = data1["price"];                          // "573.137"
    String percent_change_1h = data1["percent_change_1h"];  // "0.04"
    String last_updated = data0["last_updated"];            // "1472762067" <-- Unix Time Stamp
    String error = root["error"];   // id not found 

    client.stop();

    printTransition();
    printLogo(name);
    printName(name, symbol);
    printPrice(price);
    printChange(percent_change_1h);
    printTime(last_updated);
    printPagination();
    printError(error);
    
    oldPrice[coin] = price;

    btnState2 = LOW;
    btnState3 = LOW;

    datalooger(last_updated, price); 
  }
  printPagination();
}

void printLogo(String name) {

  if ((name == "Bitcoin") || (name == "Ethereum") || (name == "XRP") || (name == "Litecoin") || (name == "Dash")) {
   int h = 50, w = 50, row, col, buffidx = 0;
    for (row=5; row < h; row++) { // For each scanline...
     for (col=5; col < w; col++) { // For each pixel...
       //To read from Flash Memory, pgm_read_XXX is required.
       //Since image is stored as uint16_t, pgm_read_word is used as it uses 16bit address
      if (name == "Bitcoin"){
         tft.drawPixel(col, row, pgm_read_word(bitcoin + buffidx));
      }else if (name == "Ethereum") {
         tft.drawPixel(col, row, pgm_read_word(ethereum + buffidx));
      }else if (name == "XRP") {
         tft.drawPixel(col, row, pgm_read_word(ripple + buffidx));
      }else if (name == "Litecoin") {
         tft.drawPixel(col, row, pgm_read_word(litecoin + buffidx));
      }else if (name == "Dash") {
         tft.drawPixel(col, row, pgm_read_word(dash + buffidx));
      }
      buffidx++;
      } // end pixel
     }
  }else{
    tft.drawRect(5, 5, 45, 45, ILI9341_WHITE);
    tft.drawLine(5, 49, 49, 5, ILI9341_WHITE);
    tft.drawLine(5, 5, 49, 49, ILI9341_WHITE);
  }
}

void printName(String name, String symbol) {

  int StringLength = name.length();

  if (StringLength < 10){
    tft.setTextSize(3);
  }else if (StringLength >= 10 && StringLength < 15) {
    tft.setTextSize(2);
  }else if (StringLength >= 15 && StringLength < 30) {
    tft.setTextSize(1);
  }else{
  tft.setTextSize(2);
  name = "Text too long!";
  }

  tft.setCursor(65, 10);
  tft.println(name);

  tft.setTextSize(2);
  tft.setCursor(65, 33);
  tft.print(symbol);

  tft.drawLine(65, 54, 240, 54, ILI9341_WHITE);
}

void printPrice(String price) {

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(5, 75);
  tft.print("Price:");

  if(price != oldPrice[coin]){
    if(price > oldPrice[coin]){
      ILI9341_COLOR = ILI9341_GREENYELLOW;
    }else{
      ILI9341_COLOR = ILI9341_RED;
    }
  }

  tft.setTextSize(3);
  tft.setTextColor(ILI9341_COLOR);
  tft.setCursor(40, 110);
  tft.print("$");
  fixDecimal(price);
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
  printDigits(hour(t) + 2); // UTC +2 for the French summer time
  tft.print(":");
  printDigits(minute(t));
  //tft.print(":");
  //printDigits(second(t));
}

void printPagination() {

  if(btnState1 == LOW){
    for (byte i = 98; i <= 138; i += 10) {
     tft.fillCircle( i, 300, 2, ILI9341_WHITE);
    }
    if(coin == 0) {
      tft.fillCircle( 98, 300, 3, ILI9341_WHITE);
    }else if (coin == 1) {
      tft.fillCircle(108, 300, 3, ILI9341_WHITE);
    }else if (coin == 2) {
       tft.fillCircle(118, 300, 3, ILI9341_WHITE);
    }else if (coin == 3) {
       tft.fillCircle(128, 300, 3, ILI9341_WHITE);
    }else if (coin == 4) {
     tft.fillCircle(138, 300, 3, ILI9341_WHITE);
    }
  }else if(btnState1 == HIGH && antiFlickering == HIGH){
    tft.fillCircle(118, 293, 5, ILI9341_WHITE);
    tft.fillCircle(118, 294, 4, CUSTOM_DARK);
    tft.fillRoundRect(111, 293, 15, 14, 3, ILI9341_WHITE);  // Rectangle 
    tft.fillCircle(118, 299, 2, CUSTOM_DARK); // Cercle du centre
    antiFlickering = LOW;
  }
}

void printError(String error) {

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(65, 22);
  tft.println(error);
}

void printTransition(){

  tft.fillScreen(CUSTOM_DARK);
  delay(10);
  tft.fillScreen(ILI9341_RED);
  delay(10);
  tft.fillScreen(ILI9341_GREEN);
  delay(10);
  tft.fillScreen(ILI9341_BLUE);
  delay(10);
  tft.fillScreen(CUSTOM_DARK);
  delay(10);
}

void printDigits(int digits) {

  if ( digits < 10)
    tft.print('0'); 
    tft.print(digits);
}

void printDigitsSD(int digitsSD) {

  if ( digitsSD < 10 )
    root.print('0'); 
    root.print(digitsSD);
}

void fixDecimal(String price) {

  int reduceDecimalPrice = price.toInt();

  if(reduceDecimalPrice >= 1000 && reduceDecimalPrice < 100000){
    tft.println(price.substring(0, 7));
  }else if(reduceDecimalPrice >= 100){
   tft.println(price.substring(0, 6));
  }else if(reduceDecimalPrice >= 10){
   tft.println(price.substring(0, 5));
  }else if(reduceDecimalPrice >= 1){
   tft.println(price.substring(0, 4));
  }else{
   tft.println(price.substring(0, 8));
  }
}

void buttonCheck() {

  for (int i = 0; i < NUM_BUTTONS; i++)  {
    // Update the Bounce instance :
    buttons[i].update();
    if ( buttons[i].fell() && i == 0) {
      btnState1 = !btnState1;
      antiFlickering = HIGH;
      tft.fillRect(95, 297, 47, 7, CUSTOM_DARK);
      tft.fillRect(111, 287, 15, 20, CUSTOM_DARK);
      Serial.print("Bouton 1 est préssé "); Serial.println(btnState1);
    }else if(buttons[i].fell() && i == 1) {
      btnState2 = HIGH;
      Serial.print("Bouton 2 est préssé "); Serial.println(btnState2);
       if(btnState2 == HIGH){
        coin--;  
      }
    }else if(buttons[i].fell() && i == 2) {
      btnState3 = HIGH;
      Serial.print("Button 3 est préssé "); Serial.println(btnState3);
       if(btnState3 == HIGH){
        coin++;
      }
    }
  }
}

void datalooger(String last_updated, String price) {

  long timeDatalogger = last_updated.toInt();
  long oldTimeDatalogger;
  time_t y = timeDatalogger;
 
  if(coin == 0){
  root.print(year(y)); root.print("/"); printDigitsSD(month(y)); root.print("/"); printDigitsSD(day(y)); 
  root.print(" "); printDigitsSD(hour(y) + 2); root.print(":"); printDigitsSD(minute(y)); 
  root.print(";"); root.print(price);
  }else if(coin ==1){
    root.print(";");root.print(price);
  }else if(coin ==2){
    root.print(";");root.print(price);
  }else if(coin ==3){
    root.print(";");root.print(price);
  }else if(coin ==4){
    root.print(";");root.println(price);
  }
  root.flush(); 
}

void printDirectory(File dir, int numTabs) {
  
  while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');   // we'll have a nice indentation
     }
     // Print the name
     Serial.print(entry.name());
     /* Recurse for directories, otherwise print the file size */
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       /* files have sizes, directories do not */
       Serial.print("\t\t");
       Serial.println(entry.size());
     }
     entry.close();
   }
}