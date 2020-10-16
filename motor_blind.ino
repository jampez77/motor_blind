#include <Stepper_28BYJ_48.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "My_Helper.h"
#include "ConfigHelper.h"

//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//

ConfigHelper helper = ConfigHelper();

WiFiClient espClient;
PubSubClient client(espClient);
Stepper_28BYJ_48 small_stepper(D1, D3, D2, D5); //Initiate stepper driver
JsonObject jsonConfig;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  stopPowerToCoils();

  if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      client.publish(coverDebugTopic, "Critical Error!", false);
      return;
  }

  if(helper.loadconfig()){
  
    jsonConfig = helper.getconfig();

    currentPosition = jsonConfig["current"];
    minPosition = jsonConfig["min"];
    maxPosition = jsonConfig["max"];

  } else {
    client.publish(coverDebugTopic, "No config found, using default configuration", false);
  }
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.setBufferSize(1024);

  while(!configDetailsSent){
    if (connectClient()) {
      sendConfigDetailsToHA();
    }
  }
  //Setup OTA
  {
    ArduinoOTA.setHostname((mqttCoverDeviceClientId + "-" + String(ESP.getChipId())).c_str());

    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
  }
}

void loop() {
  //OTA client code
  ArduinoOTA.handle();

  //attempt connection to WiFi if we don't have it
  if(!espClient.connected()){
    setup_wifi();
  }

  //while connected we send the current door status
  //and trigger relay if we need to
  if (client.connected()) {
    client.loop();
    digitalWrite(ledPin, HIGH);
    //only activate motor if we 
    if(motorDirection == OPEN || motorDirection == CLOSE){
      int stepsBetweenMinMax = maxPosition - minPosition;

      if (motorDirection == OPEN) {
        if((minPosition != -1 && currentPosition == minPosition) ||
            stepsBetweenMinMax == 0){
          stopAndPublishState(motorDirection);
        } else {
          small_stepper.step(1);
          currentPosition = currentPosition + 1;
        }
        
      } else if (motorDirection == CLOSE) {
        if((maxPosition != -1 && currentPosition == maxPosition) ||
            stepsBetweenMinMax == 0){
          stopAndPublishState(motorDirection);
        } else {
          small_stepper.step(-1);
          currentPosition = currentPosition - 1;
        }
      }

      Serial.print("Current Position: ");
      Serial.println(currentPosition);
     
      if(motorDirection == STOP){
        stopPowerToCoils();
        DynamicJsonDocument doc(50);
        JsonObject currJson = doc.to<JsonObject>();
        currJson["min"] = minPosition;
        currJson["max"] = maxPosition;
        currJson["current"] = currentPosition;
        publishDebugJson(currJson);
        if(helper.saveconfig(currJson)){
          //client.publish(maxConfigTopic, "", false);
        } 
      }
      
    }
    
  } else {
    connectClient();
  }
}

void publishDebugJson(JsonObject json){
  char mqttJson[50];
  serializeJsonPretty(json, mqttJson);
  client.publish(coverDebugTopic, mqttJson, false);
}

void stopAndPublishState(int finishedState){
  stopPowerToCoils();
  motorDirection = STOP;
  const char* endState = opened;
  if(finishedState == CLOSE){
    endState = closed;
  }
  client.publish(coverStateTopic, endState, true);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //Set WiFi mode so we don't create an access point.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(ledPin, LOW);
  }

  digitalWrite(ledPin, HIGH);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

boolean connectClient() {
  // Loop until we're connected
  while (!client.connected()) {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    Serial.print("Attempting MQTT connection...");
    // Check connection
    if (client.connect(mqttCoverDeviceClientId.c_str(), mqtt_user, mqtt_password, coverAvailabilityTopic, 0, true, payloadNotAvailable)) {
      // Make an announcement when connected
      Serial.println("connected");
      client.publish(coverAvailabilityTopic, payloadAvailable, true);

      client.subscribe(coverCommandTopic);
      client.subscribe(coverAvailabilityTopic);
      client.subscribe(resetCommandTopic);
      client.subscribe(minCommandTopic);
      client.subscribe(maxCommandTopic);

      Serial.println("Subscribed to: ");
      Serial.println(coverCommandTopic);
      Serial.println(coverAvailabilityTopic);
      Serial.println(resetCommandTopic);
      Serial.println(minCommandTopic);
      Serial.println(maxCommandTopic);
      return true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      return false;
    }
  }
  return true;
}

void callback(char* topic, byte* message, unsigned int length) {

  String messageStr;

  for (int i = 0; i < length; i++) {
    messageStr += (char)message[i];
  }

  if (String(topic) == coverCommandTopic) {
    Serial.print("Home Assistant Command: ");
    Serial.println(messageStr);

    if (messageStr == payloadStop) {
      stopPowerToCoils();
      motorDirection = STOP;
    }

    if (messageStr == payloadOpen) {
      motorDirection = OPEN;
    }

    if (messageStr == payloadClose) {
      motorDirection = CLOSE;
    }
    const char* endState = opened;
    if(motorDirection == CLOSE){
      endState = closed;
    } else if(motorDirection == STOP){
      endState = stopped;
    }
    client.publish(coverStateTopic, endState, true);
  }

  if (String(topic) == resetCommandTopic) {
    Serial.print("Home Assistant Config Reset Blinds: ");
    Serial.println(messageStr);

    if(messageStr == "ON"){
      helper.deletefile();
      ESP.reset();
    } 
  }

  if(String(topic) == minCommandTopic){
    Serial.print("Home Assistant Config Set Blinds Min: ");
    Serial.println(messageStr);

    if(messageStr == "ON"){
      minPosition = currentPosition;
      DynamicJsonDocument minDoc(50);
      JsonObject minJson = minDoc.to<JsonObject>();
      minJson["min"] = minPosition;
      minJson["max"] = maxPosition;
      minJson["current"] = currentPosition;
      client.publish(coverDebugTopic, "Min Position Set", false);
      publishDebugJson(minJson);
      if(helper.saveconfig(minJson)){
        client.publish(minConfigTopic, "", false);
        client.publish(coverStateTopic, opened, true);
      }
    }
  }

  if(String(topic) == maxCommandTopic){
    Serial.print("Home Assistant Config Set Blinds Max: ");
    Serial.println(messageStr);

    if(messageStr == "ON"){
      maxPosition = currentPosition;
      DynamicJsonDocument maxDoc(50);
      JsonObject maxJson = maxDoc.to<JsonObject>();
      maxJson["min"] = minPosition;
      maxJson["max"] = maxPosition;
      maxJson["current"] = currentPosition;
      client.publish(coverDebugTopic, "Max Position Set", false);
      publishDebugJson(maxJson);
      if(helper.saveconfig(maxJson)){
        client.publish(maxConfigTopic, "", false);
        client.publish(coverStateTopic, closed, true);
      }
    }
  }
}

void stopPowerToCoils() {
  Serial.println("Stop Motors");
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D5, LOW);
}

void sendConfigDetailsToHA(){
  //Send cover entity details to home assistant on initial connection
    //for auto discovery

    DynamicJsonDocument mqttDevConfig(250);
    mqttDevConfig["name"] = mqttCoverDeviceClientId;
    mqttDevConfig["mf"] = manufacturer;
    mqttDevConfig["mdl"] = model;
    mqttDevConfig["sw"] = softwareVersion;
    mqttDevConfig["ids"][0] = mqttCoverDeviceClientId;
    mqttDevConfig["ids"][1] = mqttResetDeviceClientId;
    mqttDevConfig["ids"][2] = mqttMinDeviceClientId;
    mqttDevConfig["ids"][3] = mqttMaxDeviceClientId;
    
    DynamicJsonDocument mqttCoverConfig(600);
    mqttCoverConfig["name"] = mqttCoverDeviceName;
    mqttCoverConfig["dev_cla"] = mqttCoverDeviceClass;
    mqttCoverConfig["stat_t"] = coverStateTopic;
    mqttCoverConfig["cmd_t"] = coverCommandTopic;
    mqttCoverConfig["opt"] = false;
    mqttCoverConfig["ret"] = true;
    mqttCoverConfig["qos"] = 2;
    mqttCoverConfig["avty_t"] = coverAvailabilityTopic;
    mqttCoverConfig["uniq_id"] = mqttCoverDeviceClientId;
    mqttCoverConfig["dev"] = mqttDevConfig;

    char coverJson[600];
    serializeJsonPretty(mqttCoverConfig, coverJson);
    client.publish(coverConfigTopic, coverJson, true);
    
    DynamicJsonDocument mqttResetConfig(505);
    mqttResetConfig["name"] = mqttResetDeviceName;
    mqttResetConfig["ic"] = "mdi:lock-reset";
    mqttResetConfig["cmd_t"] = resetCommandTopic;
    mqttResetConfig["stat_t"] = resetStateTopic;
    mqttResetConfig["avty_t"] = coverAvailabilityTopic;
    mqttResetConfig["uniq_id"] = mqttResetDeviceClientId;
    mqttResetConfig["dev"] = mqttDevConfig;

    char resetJson[505];
    serializeJsonPretty(mqttResetConfig, resetJson);
    client.publish(resetConfigTopic, resetJson, true);

    if(minPosition == -1){
      DynamicJsonDocument mqttMinConfig(515);
      mqttMinConfig["name"] = mqttMinDeviceName;
      mqttMinConfig["ic"] = "mdi:blinds-open";
      mqttMinConfig["cmd_t"] = minCommandTopic;
      mqttMinConfig["stat_t"] = minStateTopic;
      mqttMinConfig["avty_t"] = coverAvailabilityTopic;
      mqttMinConfig["uniq_id"] = mqttMinDeviceClientId;
      mqttMinConfig["dev"] = mqttDevConfig;
  
      char minJson[515];
      serializeJsonPretty(mqttMinConfig, minJson);
      client.publish(minConfigTopic, minJson, true);
    } else {
      client.publish(minConfigTopic, "", true);
    }
    

    if(maxPosition == -1){
      DynamicJsonDocument mqttMaxConfig(515);
      mqttMaxConfig["name"] = mqttMaxDeviceName;
      mqttMaxConfig["ic"] = "mdi:blinds";
      mqttMaxConfig["cmd_t"] = maxCommandTopic;
      mqttMaxConfig["stat_t"] = maxStateTopic;
      mqttMaxConfig["avty_t"] = coverAvailabilityTopic;
      mqttMaxConfig["uniq_id"] = mqttMaxDeviceClientId;
      mqttMaxConfig["dev"] = mqttDevConfig;
  
      char maxJson[515];
      serializeJsonPretty(mqttMaxConfig, maxJson);
      client.publish(maxConfigTopic, maxJson, true);
    } else {
      client.publish(maxConfigTopic, "", true);
    }
    configDetailsSent = true;
}
