#include <Stepper_28BYJ_48.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "My_Helper.h"
#include "ConfigHelper.h"

ConfigHelper helper = ConfigHelper();

WiFiClient espClient;
PubSubClient client(espClient);
Stepper_28BYJ_48 small_stepper(D1, D3, D2, D5); //Initiate stepper driver
Config jsonConfig;
WiFiServer server(80);

boolean saveConfig() {
  jsonConfig.currentPosition = currentPosition;
  jsonConfig.maxPosition = maxPosition;
  jsonConfig.minPosition = minPosition;
  jsonConfig.initialSetup = initialSetup;

  strlcpy(jsonConfig.deviceName,                
          device_name,  
          sizeof(jsonConfig.deviceName));    
          
  strlcpy(jsonConfig.mqttUser,                
         mqtt_user,  
          sizeof(jsonConfig.mqttUser));    
  
  strlcpy(jsonConfig.mqttPassword,                
          mqtt_password,  
          sizeof(jsonConfig.mqttPassword));    
          
  strlcpy(jsonConfig.mqttServerIp,                
          mqtt_server_ip,  
          sizeof(jsonConfig.mqttServerIp));    
  
  strlcpy(jsonConfig.mqttServerPort,                
          mqtt_server_port,  
          sizeof(jsonConfig.mqttServerPort));    

  //publishDebugJson(json);

  return helper.saveconfig(jsonConfig);
}

void saveConfigCallback () {
  Serial.println("Setup Completed");
  initialSetup = false;
}

void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  stopPowerToCoils();

  deviceId = String(ESP.getChipId()).c_str();

  coverStateTopic = "homeassistant/cover/middleFloor/" + deviceId + "/state";
  coverDebugTopic = "homeassistant/cover/middleFloor/" + deviceId + "/debug";
  coverCommandTopic = "homeassistant/cover/middleFloor/" + deviceId + "/set";
  coverConfigTopic = "homeassistant/cover/middleFloor/" + deviceId + "/config";
  coverAvailabilityTopic = "homeassistant/cover/middleFloor/" + deviceId + "/availability";
  mqttCoverDeviceClientId = deviceId + "Blind";

  resetCommandTopic = "homeassistant/switch/middleFloor/" + deviceId + "/set";
  resetStateTopic = "homeassistant/switch/middleFloor/" + deviceId + "/state";
  resetConfigTopic = "homeassistant/switch/middleFloor/" + deviceId + "/config";
  mqttResetDeviceClientId = deviceId + "RstBlinds";
  
  minCommandTopic = "homeassistant/switch/middleFloor/" + deviceId + "Min/set";
  minStateTopic = "homeassistant/switch/middleFloor/" + deviceId + "Min/state";
  minConfigTopic = "homeassistant/switch/middleFloor/" + deviceId + "Min/config";
  mqttMinDeviceClientId = deviceId + "BlindsMin";

  maxCommandTopic = "homeassistant/switch/middleFloor/" + deviceId + "Max/set";
  maxStateTopic = "homeassistant/switch/middleFloor/" + deviceId + "Max/state";
  maxConfigTopic = "homeassistant/switch/middleFloor/" + deviceId + "Max/config";
  mqttMaxDeviceClientId = deviceId + "BlindsMax";

  if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      client.publish(coverDebugTopic.c_str(), "Critical Error!", false);
      return;
  }

  if(helper.loadconfig()){
    Serial.print("Config Details: ");
    jsonConfig = helper.getconfig();

    Serial.print("currentPosition: ");
    currentPosition = jsonConfig.currentPosition;
    Serial.println(currentPosition);

    Serial.print("minPosition: ");
    minPosition = jsonConfig.minPosition;
    Serial.println(minPosition);

    Serial.print("maxPosition: ");
    maxPosition = jsonConfig.maxPosition;
    Serial.println(maxPosition);

    Serial.print("Device Name: ");
    strcpy(device_name, jsonConfig.deviceName);
    Serial.println(device_name);

    Serial.print("MQTT User: ");
    strcpy(mqtt_user, jsonConfig.mqttUser);
    Serial.println(mqtt_user);

    Serial.print("MQTT Password: ");
    strcpy(mqtt_password, jsonConfig.mqttPassword);
    Serial.println(mqtt_password);

    Serial.print("MQTT IP: ");
    strcpy(mqtt_server_ip, jsonConfig.mqttServerIp);
    Serial.println(mqtt_server_ip);

    Serial.print("MQTT Port: ");
    strcpy(mqtt_server_port, jsonConfig.mqttServerPort);
    Serial.println(mqtt_server_port);

    Serial.print("Initial Setup: ");
    initialSetup = jsonConfig.initialSetup;
    Serial.println(initialSetup);

  } else {
    client.publish(coverDebugTopic.c_str(), "No config found, using default configuration", false);
  }

  if(initialSetup){

    Serial.println("Setup Required! Starting Access Point");

    Serial.print("Creating access point: ");
    Serial.println(deviceId);
    String apName = "Blinds-"+ deviceId;
    
    WiFiManagerParameter custom_device_name("d_name", "Device Name", device_name, 25, " required onkeyup='this.value = this.value.replace(/(^\\w{1})|(\\s+\\w{1})/g, letter => letter.toUpperCase());'");
    WiFiManagerParameter custom_text("<p><b>MQTT server parameters:</b></p>");
    WiFiManagerParameter custom_mqtt_server("mqtt_ip", "MQTT Server IP", mqtt_server_ip, 15, " required");
    WiFiManagerParameter custom_mqtt_port("mqtt_p", "MQTT Port", mqtt_server_port, 6, " required");
    WiFiManagerParameter custom_mqtt_user("mqtt_u", "MQTT Username", mqtt_user, 40, " required");
    WiFiManagerParameter custom_mqtt_password("mqtt_pw", "MQTT Password", mqtt_password, 40, " required type='password'");
    //Setup WIFI Manager


    WiFiManager wifiManager;
    wifiManager.setClass("invert"); 
    wifiManager.setSaveConfigCallback(saveConfigCallback);


    wifiManager.addParameter(&custom_device_name);    
    wifiManager.addParameter(&custom_text);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port); 
    wifiManager.addParameter(&custom_mqtt_user);    
    wifiManager.addParameter(&custom_mqtt_password);    
       
    wifiManager.autoConnect(apName.c_str(), appass);
    
    if(!initialSetup){
        Serial.println("Setup Finished.");
        //read updated parameters

        char temp_dn[25];
        strcpy(temp_dn, custom_device_name.getValue());
        
        memcpy(device_name, temp_dn, 25);

        char temp_mp[25];
        strcpy(temp_mp, custom_mqtt_port.getValue());

        memcpy(mqtt_server_port, temp_mp, 25);

        char temp_ms[25];
        strcpy(temp_ms, custom_mqtt_server.getValue());
        

        memcpy(mqtt_server_ip, temp_ms, 25);

        char temp_mpw[25];
        strcpy(temp_mpw, custom_mqtt_password.getValue());
        
 
        memcpy(mqtt_password, temp_mpw, 25);
        
        char temp_mu[25];
        strcpy(temp_mu, custom_mqtt_user.getValue());
        
        memcpy(mqtt_user, temp_mu, 25);
        
        saveConfig();
        ESP.restart();
    }

    Serial.println("Connected.");
  
    server.begin();
    
  } else {
    Serial.println("Setup cofig found");
    Serial.println("Connecting to HA");
    setup_wifi();
    client.setServer(mqtt_server_ip, atoi(mqtt_server_port));
    client.setCallback(callback);
    client.setBufferSize(1024);
  
    while(!configDetailsSent){
      if (connectClient()) {
        sendConfigDetailsToHA();
      }
    }
  }
  
  
  //Setup OTA
  {
    ArduinoOTA.setHostname(device_name);

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

void handleRoot() {
  WiFiClient client = server.available(); 
  if (client) {   
    Serial.println("New Client."); 
    String currentLine = ""; 
        while (client.connected()) {
          char c = client.read();             // read a byte, then
            Serial.write(c);               // loop while the client's connected
          if (client.available()) {             // if there's bytes to read from the client,
                             // print it out the serial monitor
            header += c;
            if (c == '\n') {   
               // if the current line is blank, you got two newline characters in a row.
               // that's the end of the client HTTP request, so send a response:
               // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              // CSS to style the on/off buttons 
              // Feel free to change the background-color and font-size attributes to fit your preferences
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".button2 {background-color: #77878A;}</style></head>");
              
              // Web Page Heading
              client.println("<body>");

              client.println("</body></html>");
              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
              break;
            } else {
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
  }
  
}


void loop() {
  //OTA client code
  ArduinoOTA.handle();

  if(initialSetup){
    handleRoot();
  } else {
  
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
                (minPosition != -1 && maxPosition != -1 && stepsBetweenMinMax == 0)){
              stopAndPublishState(motorDirection);
            } else {
              small_stepper.step(1);
              currentPosition = currentPosition + 1;
            }
            
          } else if (motorDirection == CLOSE) {
          if((maxPosition != -1 && currentPosition == maxPosition) ||
              (minPosition != -1 && maxPosition != -1 && stepsBetweenMinMax == 0)){
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
            if(saveConfig()){
              //client.publish(maxConfigTopic, "", false);
            } 
          }
          
        }
        
      } else {
        connectClient();
      }
  }

}

void publishDebugJson(JsonObject json){
  char mqttJson[250];
  serializeJsonPretty(json, mqttJson);
  client.publish(coverDebugTopic.c_str(), mqttJson, false);
}

void stopAndPublishState(int finishedState){
  stopPowerToCoils();
  motorDirection = STOP;
  const char* endState = opened;
  if(finishedState == CLOSE){
    endState = closed;
  }
  client.publish(coverStateTopic.c_str(), endState, true);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(&ssid);

  //Set WiFi mode so we don't create an access point.
  WiFi.mode(WIFI_STA);
  WiFi.begin(&ssid, &password);

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
    Serial.println("Attempting MQTT connection...");
    // Check connection
    if (client.connect(mqttCoverDeviceClientId.c_str(), mqtt_user, mqtt_password, coverAvailabilityTopic.c_str(), 0, true, payloadNotAvailable)) {
      // Make an announcement when connected
      Serial.println("connected");
      client.publish(coverAvailabilityTopic.c_str(), payloadAvailable, true);

      client.subscribe(coverCommandTopic.c_str());
      client.subscribe(coverAvailabilityTopic.c_str());
      client.subscribe(resetCommandTopic.c_str());
      client.subscribe(minCommandTopic.c_str());
      client.subscribe(maxCommandTopic.c_str());

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
    client.publish(coverStateTopic.c_str(), endState, true);
  }

  if (String(topic) == resetCommandTopic) {
    Serial.print("Home Assistant Config Reset Blinds: ");
    Serial.println(messageStr);

    if(messageStr == "ON"){
      helper.deletefile();
      WiFiManager wifiManager;
      wifiManager.disconnect();
      delay(500);
      wifiManager.erase();
      delay(500);
      ESP.reset();
    } 
  }

  if(String(topic) == minCommandTopic){
    Serial.print("Home Assistant Config Set Blinds Min: ");
    Serial.println(messageStr);

    if(messageStr == "ON"){
      minPosition = currentPosition;
      client.publish(coverDebugTopic.c_str(), "Min Position Set", false);
      if(saveConfig()){
        client.publish(minConfigTopic.c_str(), "", false);
        client.publish(coverStateTopic.c_str(), opened, true);
      }
    }
  }

  if(String(topic) == maxCommandTopic){
    Serial.print("Home Assistant Config Set Blinds Max: ");
    Serial.println(messageStr);

    if(messageStr == "ON"){
      maxPosition = currentPosition;
      client.publish(coverDebugTopic.c_str(), "Max Position Set", false);
      if(saveConfig()){
        client.publish(maxConfigTopic.c_str(), "", false);
        client.publish(coverStateTopic.c_str(), closed, true);
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

    DynamicJsonDocument mqttDevConfig(300);
    mqttDevConfig["name"] = device_name;
    mqttDevConfig["mf"] = manufacturer;
    mqttDevConfig["mdl"] = model;
    mqttDevConfig["sw"] = softwareVersion;
    mqttDevConfig["ids"][0] = mqttCoverDeviceClientId;
    mqttDevConfig["ids"][1] = mqttResetDeviceClientId;
    mqttDevConfig["ids"][2] = mqttMinDeviceClientId;
    mqttDevConfig["ids"][3] = mqttMaxDeviceClientId;
    
    DynamicJsonDocument mqttCoverConfig(650);
    mqttCoverConfig["name"] = device_name;
    mqttCoverConfig["dev_cla"] = mqttCoverDeviceClass;
    mqttCoverConfig["stat_t"] = coverStateTopic;
    mqttCoverConfig["cmd_t"] = coverCommandTopic;
    mqttCoverConfig["opt"] = false;
    mqttCoverConfig["ret"] = true;
    mqttCoverConfig["qos"] = 2;
    mqttCoverConfig["avty_t"] = coverAvailabilityTopic;
    mqttCoverConfig["uniq_id"] = mqttCoverDeviceClientId;
    mqttCoverConfig["dev"] = mqttDevConfig;

    char coverJson[650];
    serializeJsonPretty(mqttCoverConfig, coverJson);
    client.publish(coverConfigTopic.c_str(), coverJson, true);
    
    DynamicJsonDocument mqttResetConfig(555);
    mqttResetConfig["name"] = "Erase & Reset";
    mqttResetConfig["ic"] = "mdi:lock-reset";
    mqttResetConfig["cmd_t"] = resetCommandTopic;
    mqttResetConfig["stat_t"] = resetStateTopic;
    mqttResetConfig["avty_t"] = coverAvailabilityTopic;
    mqttResetConfig["uniq_id"] = mqttResetDeviceClientId;
    mqttResetConfig["dev"] = mqttDevConfig;

    char resetJson[555];
    serializeJsonPretty(mqttResetConfig, resetJson);
    client.publish(resetConfigTopic.c_str(), resetJson, true);

    if(minPosition == -1){
      DynamicJsonDocument mqttMinConfig(565);
      mqttMinConfig["name"] = "Set Open";
      mqttMinConfig["ic"] = "mdi:blinds-open";
      mqttMinConfig["cmd_t"] = minCommandTopic;
      mqttMinConfig["stat_t"] = minStateTopic;
      mqttMinConfig["avty_t"] = coverAvailabilityTopic;
      mqttMinConfig["uniq_id"] = mqttMinDeviceClientId;
      mqttMinConfig["dev"] = mqttDevConfig;
  
      char minJson[565];
      serializeJsonPretty(mqttMinConfig, minJson);
      client.publish(minConfigTopic.c_str(), minJson, true);
    } else {
      client.publish(minConfigTopic.c_str(), "", true);
    }
    

    if(maxPosition == -1){
      DynamicJsonDocument mqttMaxConfig(565);
      mqttMaxConfig["name"] = "Set Closed";
      mqttMaxConfig["ic"] = "mdi:blinds";
      mqttMaxConfig["cmd_t"] = maxCommandTopic;
      mqttMaxConfig["stat_t"] = maxStateTopic;
      mqttMaxConfig["avty_t"] = coverAvailabilityTopic;
      mqttMaxConfig["uniq_id"] = mqttMaxDeviceClientId;
      mqttMaxConfig["dev"] = mqttDevConfig;
  
      char maxJson[565];
      serializeJsonPretty(mqttMaxConfig, maxJson);
      client.publish(maxConfigTopic.c_str(), maxJson, true);
    } else {
      client.publish(maxConfigTopic.c_str(), "", true);
    }
    configDetailsSent = true;
}
