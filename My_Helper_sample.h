#ifndef MY_HELPER_H
#define MY_HELPER_H

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_user = "YOUR_MQTT_USER";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";
const char* mqtt_server_ip = "YOUR_MQTT_SERVER_IP_ADDRESS";
const char* mqtt_server_port = 1883;

//change this name (MAX 25 CHARACTERS!!!)
//Device name in HA will be appended with " Blinds"
String deviceName = "testing"; 

/*
 *  DO NOT CHANGE ANY VALUES BELOW THIS SECTION, UNLESS YOU KNOW WHAT YOU'RE DOING!
 *  
 *  Sorry, that's a bit dramatic, nothing bad will happen but things might not work.
 *  
 *  If you would like a longer deviceName then you will need to increase the amount of
 *  allocated memory in the `DynamicJsonDocument` in `sendConfigDetailsToHA`
 *  
 *  if you want to change the names shown in Home Assistant then you should do this 
 *  in HA by editing the entity names in the MQTT integrations section.
 */
 
const String coverStateTopic = "homeassistant/cover/middleFloor/" + deviceName + "/state";
const String coverDebugTopic = "homeassistant/cover/middleFloor/" + deviceName + "/debug";
const String coverCommandTopic = "homeassistant/cover/middleFloor/" + deviceName + "/set";
const String coverConfigTopic = "homeassistant/cover/middleFloor/" + deviceName + "/config";
const String coverAvailabilityTopic = "homeassistant/cover/middleFloor/" + deviceName + "/availability";
const String mqttCoverDeviceClientId = deviceName + "Blind";
const char* mqttCoverDeviceName = "Blinds";
const char* mqttCoverDeviceClass = "blind";

const String resetCommandTopic = "homeassistant/switch/middleFloor/" + deviceName + "/set";
const String resetStateTopic = "homeassistant/switch/middleFloor/" + deviceName + "/state";
const String resetConfigTopic = "homeassistant/switch/middleFloor/" + deviceName + "/config";
const String mqttResetDeviceClientId = deviceName + "RstBlinds";
const char* mqttResetDeviceName = "Reset";

const String minCommandTopic = "homeassistant/switch/middleFloor/" + deviceName + "Min/set";
const String minStateTopic = "homeassistant/switch/middleFloor/" + deviceName + "Min/state";
const String minConfigTopic = "homeassistant/switch/middleFloor/" + deviceName + "Min/config";
const String mqttMinDeviceClientId = deviceName + "BlindsMin";
const char* mqttMinDeviceName = "Set Min";

const String maxCommandTopic = "homeassistant/switch/middleFloor/" + deviceName + "Max/set";
const String maxStateTopic = "homeassistant/switch/middleFloor/" + deviceName + "Max/state";
const String maxConfigTopic = "homeassistant/switch/middleFloor/" + deviceName + "Max/config";
const String mqttMaxDeviceClientId = deviceName + "BlindsMax";
const char* mqttMaxDeviceName = "Set Max";

const char* doorStatus = "";
const char* prevDoorStatus = "";

const char* softwareVersion = "1.1.1";
const char* manufacturer = "NandPez";
const char* model = "Node MCU D1 Roller Blind Motor";
boolean configDetailsSent = false;

long currentPosition = 0;  
long minPosition = -1;  
long maxPosition = -1;  

int CLOSE = 1;
int OPEN = 0;
int STOP = -1;
int motorDirection = STOP;
const int ledPin = LED_BUILTIN;

//Statuses
const char* opened = "open";
const char* closed = "closed";
const char* closing = "closing";
const char* opening = "opening";
const char* stopped = "stopped";

//pay loads
const char* payloadOpen = "OPEN";
const char* payloadClose = "CLOSE";
const char* payloadStop = "STOP";
const char* payloadAvailable = "online";
const char* payloadNotAvailable = "offline";

#endif
