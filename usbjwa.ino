/*
 * ESP32 Web server with Web Socket for touchscreen joystick buttons.
 */

/*
 * MIT License
 *
 * Copyright (c) 2021 touchgadgetdev@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define HOSTNAME "espjoystick"

#include <WiFi.h>
#include <WiFiManager.h>      // See README.md
#include <WebSocketsServer.h> // Install WebSockets by Markus Sattler from IDE Library manager
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>  // Install from IDE Library manager

#define DEBUG_ON  0
#if DEBUG_ON
#define DBG_begin(...)    Serial.begin(__VA_ARGS__)
#define DBG_print(...)    Serial.print(__VA_ARGS__)
#define DBG_println(...)  Serial.println(__VA_ARGS__)
#define DBG_printf(...)   Serial.printf(__VA_ARGS__)
#else
#define DBG_begin(...)
#define DBG_print(...)
#define DBG_println(...)
#define DBG_printf(...)
#endif

const unsigned int ESP32S2_ADC_BITS = 13;
const unsigned int MAX_ADC_VALUE = (1 << ESP32S2_ADC_BITS) - 1;
#ifdef ARDUINO_METRO_ESP32S2
const uint8_t X_AXIS_ANALOG_PIN = 15;
const uint8_t Y_AXIS_ANALOG_PIN = 16;
#else
const uint8_t X_AXIS_ANALOG_PIN = A14;
const uint8_t Y_AXIS_ANALOG_PIN = A15;
#endif
int X_ADC_Max = ((MAX_ADC_VALUE * 3) / 4) - 512;
int Y_ADC_Max = ((MAX_ADC_VALUE * 3) / 4) - 512;

MDNSResponder mdns;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#include "joystick_grid.h"
#include "index_html.h"
#include <USB.h>
#include "joystick_esp32.h"

TUJoystick Joystick;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  DBG_printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      DBG_printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        DBG_printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // Send touch grid in JSON format
        Json_touch_grid(num, Joystick_Page);
      }
      break;
    case WStype_TEXT:
      {
        DBG_printf("[%u] get Text: [%d] %s \r\n", num, length, payload);

        StaticJsonDocument<96> doc;
        DeserializationError error = deserializeJson(doc, payload);;

        if (error) {
          DBG_print(F("deserializeJson() failed: "));
          DBG_println(error.f_str());
          return;
        }
        const char* event = doc["event"];
        int row = doc["row"];
        if (row < 0) {
          DBG_printf("row negative %d\n", row);
          row = 0;
        }
        if (row >= MAX_ROWS) {
          DBG_printf("row too high %d\n", row);
          row = MAX_ROWS - 1;
        }

        int col = doc["col"];
        if (col < 0) {
          DBG_printf("col negative %d\n", col);
          col = 0;
        }
        if (col >= MAX_COLS) {
          DBG_printf("col too high %d\n", col);
          col = MAX_COLS - 1;
        }

        uint8_t joy_button = Joystick_Cells[row][col].joystick_control;
        if (strcmp(event, "touch start") == 0) {
          Joystick.press(joy_button);
        }
        else if (strcmp(event, "touch end") == 0) {
          Joystick.release(joy_button);
        }
      }
      break;
    case WStype_BIN:
      DBG_printf("[%u] get binary length: %u\r\n", num, length);
      //      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      DBG_printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void handleRoot()
{
  server.send(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup()
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  DBG_begin(115200);

//  USB.usbClass(0);
//  USB.usbSubClass(0);
//  USB.usbProtocol(0);
  USB.usbPower(100);        // 100 mA max
  USB.usbAttributes(0x80);  // Bus powered
  USB.usbVersion(0x0110);
  Joystick.begin();
  USB.begin();

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect(HOSTNAME);
  // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
      DBG_println(F("Failed to connect"));
      delay(1000);
      ESP.restart();
  }

  if (mdns.begin(HOSTNAME)) {
    DBG_println(F("MDNS responder started"));
    mdns.addService("http", "tcp", 80);
    mdns.addService("ws", "tcp", 81);
  }
  else {
    DBG_println(F("MDNS.begin failed"));
  }
  DBG_print(F("Connect to http://" HOSTNAME ".local or http://"));
  DBG_println(WiFi.localIP());

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void joystick_ADC()
{
  int x = analogRead(X_AXIS_ANALOG_PIN);
  int y = analogRead(Y_AXIS_ANALOG_PIN);
  if (x > X_ADC_Max) {
    X_ADC_Max = x;
    DBG_print("x max="); DBG_println(X_ADC_Max);
  }
  if (y > Y_ADC_Max) {
    Y_ADC_Max = y;
    DBG_print("y max="); DBG_println(Y_ADC_Max);
  }
  Joystick.xAxis(map(x, 0, X_ADC_Max, -32767, 32767));
  Joystick.yAxis(map(y, 0, Y_ADC_Max, 32767, -32767));
}

void loop()
{
  joystick_ADC();
  webSocket.loop();
  server.handleClient();

  if (Joystick.ready()) Joystick.loop();
}
