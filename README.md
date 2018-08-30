# ESP32-TTGO-T4 CryptoMoneyTicker v1.3



![image](https://github.com/LilyGO/TTGO-T4-DEMO/blob/master/image/T4%20V1.1.jpg)

![image](https://github.com/LilyGO/TTGO-T4-DEMO/blob/master/image/image1.jpg)

![Demo_CrytptoMoneyTicker](https://github.com/mnett0/CryptoMoneyTicker/blob/master/medias/demo.gif)

## Update v1.3

Correction of the price display which had 9 decimal places.

Replacement of the Cardano crypto-currency for Dash.

## Update v1.2

Switch to version 2 of the API [CoinMarketCap](https://coinmarketcap.com).

## Update v1.1

Now you can follow 5 crypto-currency of your choice but without bitmap, you can always add it. They are already available for Bitcoin, Ethereum, Ripple, Litecoin and Dash.

Bitmap support in 16 bit.

## Description

This a small screen for your desktrop or somewhere else, it allows to follow 5 of your favorites crypto-currency.

Data are collected via the API of the site [CoinMarketCap](https://coinmarketcap.com) and are updated every 5 minutes.

The display changes of crypto-currency every minute. You can change this delay but do not exceed 30 updates per minute.

### Materials

* ESP32
* LCD TFT ILI9341 2.2"

![Diagram_CryptoMoneyTicker](https://github.com/mnett0/CryptoMoneyTicker/blob/master/medias/Schema_Cryptho_Ticker.jpg)

### Library required

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [WiFi](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
* [WiFiClientSecure](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure)
* [SPI](https://github.com/espressif/arduino-esp32/tree/master/libraries/SPI)
* [Adafruit-GFX](https://github.com/adafruit/Adafruit-GFX-Library)
* [Adafruit-ILI9341](https://github.com/adafruit/Adafruit_ILI9341)
* [TimeLib](https://github.com/PaulStoffregen/Time)

### To begin

* Enter the IDs of your WiFi :

      50  const char* ssid = "yourSSID";
      51  const char* password = "yourPASSWORD";

* Enter the 5 crypto-currency of your choice:

Go to [listings](https://api.coinmarketcap.com/v2/listings/) then CTRL+F to launch the search tool, then type the name of the desired currency, take its {id}.
And finally follow the instructions from line 73

![listings_picture](https://github.com/mnett0/CryptoMoneyTicker/blob/master/medias/listings.jpg)

* Choose the screen change interval *(do not exceed 30 changes/minute)*

      145  interval = yourInterval; // in milliseconds


Do you like this ? Please :star: this project!

### License

[MIT](https://github.com/mnett0/CryptoMoneyTicker/blob/master/LICENSE.md)
