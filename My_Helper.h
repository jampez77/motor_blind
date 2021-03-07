#ifndef MY_HELPER_H
#define MY_HELPER_H

/*
 *  DO NOT CHANGE ANY VALUES BELOW THIS SECTION, UNLESS YOU KNOW WHAT YOU'RE DOING!
 *  
 *  Sorry, that's a bit dramatic, nothing bad will happen but things might not work.
 *  
 *  If you would like a longer deviceId then you will need to increase the amount of
 *  allocated memory in the `DynamicJsonDocument` in `sendConfigDetailsToHA`
 *  
 *  if you want to change the names shown in Home Assistant then you should do this 
 *  in HA by editing the entity names in the MQTT integrations section.
 */

String coverStateTopic;
String coverDebugTopic;
String coverCommandTopic;
String coverConfigTopic;
String coverAvailabilityTopic;
String mqttCoverDeviceClientId;
const char* mqttCoverdeviceId = "Blinds";
const char* mqttCoverDeviceClass = "blind";

String resetCommandTopic;
String resetStateTopic;
String resetConfigTopic;
String mqttResetDeviceClientId;

String resetLimitsCommandTopic;
String resetLimitsStateTopic;
String resetLimitsConfigTopic;
String mqttResetLimitsDeviceClientId;

String wifiAPCommandTopic;
String wifiAPStateTopic;
String wifiAPConfigTopic;
String mqttWifiAPDeviceClientId;

String minCommandTopic;
String minStateTopic;
String minConfigTopic;
String mqttMinDeviceClientId;

String maxCommandTopic;
String maxStateTopic;
String maxConfigTopic;
String mqttMaxDeviceClientId;

const char* doorStatus = "";
const char* prevDoorStatus = "";

const char* softwareVersion = "2.1";
const char* manufacturer = "NandPez";
const char* model = "Roller Blind Motor";
boolean configDetailsSent = false;

long currentPosition = 0;  
long minPosition = -1;  
long maxPosition = -1;  

int CLOSE = 1;
int OPEN = 0;
int STOP = -1;
int motorDirection = STOP;
const int ledPin = LED_BUILTIN;
int motor1pin1 = D2;
int motor1pin2 = D8;

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

//AP Details
String header;
boolean initialSetup = true;
char* appass = "nandpezblinds";
char ssid;
char password;

char mqtt_user[25];
char mqtt_password[25];
char mqtt_server_ip[25];
char mqtt_server_port[25];
char device_name[25];
String deviceId;

#endif
