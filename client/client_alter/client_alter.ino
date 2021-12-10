/****************************************************************************************************************************
  ESP8266_WebSocketClientSocketIO.ino
  For ESP8266

  Based on and modified from WebSockets libarary https://github.com/Links2004/arduinoWebSockets
  to support other boards such as  SAMD21, SAMD51, Adafruit's nRF52 boards, etc.

  Built by Khoi Hoang https://github.com/khoih-prog/WebSockets_Generic
  Licensed under MIT license

  Originally Created on: 06.06.2016
  Original Author: Markus Sattler
*****************************************************************************************************************************/

// check board
#if !defined(ESP8266)
#error This code is intended to run only on the ESP8266 boards ! Please check your Tools->Board setting.
#endif

#define _WEBSOCKETS_LOGLEVEL_ 3

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient_Generic.h>
#include <SocketIOclient_Generic.h>

#include <Hash.h>
#include <string.h>

#define ledPin D2

ESP8266WiFiMulti WiFiMulti;
SocketIOclient socketIO;

// Select the IP address according to your local network
IPAddress serverIP(192, 168, 100, 17);
uint16_t serverPort = 8000;

// define a global mode value
// 0 means it is in manual mode, 1 means it is in auto mode
int automode = 0;

// function to make led automatically toggle
void autoLed(){
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
    delay(1000);
}

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case sIOtype_DISCONNECT:
        Serial.println("[IOc] Disconnected");
        break;
    case sIOtype_CONNECT:
        Serial.print("[IOc] Connected to url: ");
        Serial.println((char *)payload);

        // join default namespace (no auto join in Socket.IO V3)
        socketIO.send(sIOtype_CONNECT, "/");

        break;
    case sIOtype_EVENT:
        Serial.print("[IOc] Get event: ");
        Serial.printf("Here event: %c", (char *)payload);

        break;
    case sIOtype_ACK:
        Serial.print("[IOc] Get ack: ");
        Serial.println(length);

        hexdump(payload, length);
        break;
    case sIOtype_ERROR:
        Serial.print("[IOc] Get error: ");
        Serial.println(length);

        hexdump(payload, length);
        break;
    case sIOtype_BINARY_EVENT:
        Serial.print("[IOc] Get binary: ");
        Serial.println(length);

        hexdump(payload, length);
        break;
    case sIOtype_BINARY_ACK:
        Serial.print("[IOc] Get binary ack: ");
        Serial.println(length);

        hexdump(payload, length);
        break;

    default:
        break;
    }
}

// function for toggling the LED using socket.io
void toggleLED(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
    // check mode from socket 
    char index[4];
    char mode_value[4];
    memcpy(index, &payload[2], 4);
    index[4] = '\0';
    Serial.printf("tada %s\n", index);

    // check if it is mode then check the value
    if(strcmp(index, "mode") == 0){
        memcpy(mode_value, &payload[17], 4);
        mode_value[4] = '\0';
        Serial.printf("Mode value is %s\n", mode_value);
        if(strcmp(mode_value, "true") == 0){
            // update global automode value
            Serial.println("Automode is on");
            automode = 1;
            // call out autoled
            autoLed();
        }
    }
    else{
        // set automode back to false
        automode = false;
        // declare a subs with typee of char which will store up to 5 characters
        char subs[5];
        memcpy(subs, &payload[16], 4);
        subs[4] = '\0';
        Serial.printf("Original aftercut: %s\n", subs);
    
        if (strcmp(subs, "true") == 0)
        {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED is on");
        }
        else
        {
            digitalWrite(ledPin, LOW);
        }
        delay(500);
    }
    
}

void setup()
{
    pinMode(ledPin, OUTPUT);
    Serial.begin(115200);
    while (!Serial);

    // Serial.setDebugOutput(true);

    // disable AP
    if (WiFi.getMode() & WIFI_AP)
    {
        WiFi.softAPdisconnect(true);
    }

    WiFiMulti.addAP("Cormac", "+Ah(nstP7.U7+qz");

    // WiFi.disconnect();
    while (WiFiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.println();

    // Client address
    Serial.print("WebSockets Client started @ IP address: ");
    Serial.println(WiFi.localIP());

    // server address, port and URL
    Serial.print("Connecting to WebSockets Server @ IP address: ");
    Serial.print(serverIP);
    Serial.print(", port: ");
    Serial.println(serverPort);

    // setReconnectInterval to 10s, new from v2.5.1 to avoid flooding server. Default is 0.5s
    socketIO.setReconnectInterval(2000);

    socketIO.setExtraHeaders("Authorization: 1234567890");

    // server address, port and URL
    // void begin(IPAddress host, uint16_t port, String url = "/socket.io/?EIO=4", String protocol = "arduino");
    // To use default EIO=4 fron v2.5.1
    

    // event handler
    socketIO.onEvent(socketIOEvent);
    
    socketIO.begin(serverIP, serverPort);
    // toggle led event
    //socketIO.onEvent(toggleLed);
}

unsigned long messageTimestamp = 0;

void loop()
{   
    // looping the socketio
    socketIO.loop();
    
    // if it is not in auto mode, make it auto
    if(automode == 1){
        autoLed();
    }
    else{
        // else it is in manual mode, make it manual
        if(automode == 0){
            socketIO.onEvent(toggleLED);
        }
    }
}
