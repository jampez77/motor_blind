#ifndef MY_HELPER_H
#define MY_HELPER_H

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_user = "YOUR_MQTT_USER";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_SERVER";
const char* coverStateTopic = "homeassistant/cover/middleFloor/studyBlind/state";
const char* coverDebugTopic = "homeassistant/cover/middleFloor/studyBlind/debug";
const char* coverCommandTopic = "homeassistant/cover/middleFloor/studyBlind/set";
const char* coverConfigTopic = "homeassistant/cover/middleFloor/studyBlind/config";
const char* coverAvailabilityTopic = "homeassistant/cover/middleFloor/studyBlind/availability";
String mqttCoverDeviceClientId = "Blinds";
const char* mqttCoverDeviceName = "Study Blinds";
const char* mqttCoverDeviceClass = "blind";

const char* resetCommandTopic = "homeassistant/switch/middleFloor/studyBlind/set";
const char* resetStateTopic = "homeassistant/switch/middleFloor/studyBlind/state";
const char* resetConfigTopic = "homeassistant/switch/middleFloor/studyBlind/config";
String mqttResetDeviceClientId = "RstBlinds";
const char* mqttResetDeviceName = "Reset";

const char* minCommandTopic = "homeassistant/switch/middleFloor/studyBlindMin/set";
const char* minStateTopic = "homeassistant/switch/middleFloor/studyBlindMin/state";
const char* minConfigTopic = "homeassistant/switch/middleFloor/studyBlindMin/config";
String mqttMinDeviceClientId = "BlindsMin";
const char* mqttMinDeviceName = "Set Min";

const char* maxCommandTopic = "homeassistant/switch/middleFloor/studyBlindMax/set";
const char* maxStateTopic = "homeassistant/switch/middleFloor/studyBlindMax/state";
const char* maxConfigTopic = "homeassistant/switch/middleFloor/studyBlindMax/config";
String mqttMaxDeviceClientId = "BlindsMax";
const char* mqttMaxDeviceName = "Set Max";

const char* doorStatus = "";
const char* prevDoorStatus = "";

const char* softwareVersion = "1.0";
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
