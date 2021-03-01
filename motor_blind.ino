#include <Stepper_28BYJ_48.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
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
WiFiServer server(80);
WiFiManager wifiManager;

void setup() {
  deviceId = String(ESP.getChipId()).c_str();
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  stopPowerToCoils();

  if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      client.publish(coverDebugTopic.c_str(), "Critical Error!", false);
      return;
  }

  if(helper.loadconfig()){
  
    jsonConfig = helper.getconfig();

    currentPosition = jsonConfig["current"];
    minPosition = jsonConfig["min"];
    maxPosition = jsonConfig["max"];
    deviceName = jsonConfig["d_name"];
    ssid = jsonConfig["ssid"];
    password =jsonConfig["pw"];
    mqtt_user =jsonConfig["mqtt_u"];
    mqtt_password =jsonConfig["mqtt_pw"];
    mqtt_server_ip =jsonConfig["mqtt_ip"];
    mqtt_server_port =jsonConfig["mqtt_p"];

  } else {
    client.publish(coverDebugTopic.c_str(), "No config found, using default configuration", false);
  }

  
  if(initialSetupMode){

    Serial.println("Setup Required! Starting Access Point");

    Serial.print("Creating access point: ");
    Serial.println(deviceId);
    String apName = "Blinds-"+ deviceId;
    
    wifiManager.autoConnect(apName.c_str(), appass);

    Serial.println("Connected.");
  
    server.begin();
    
  } else {
    setup_wifi();
    client.setServer(mqtt_server_ip, mqtt_server_port);
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
    ArduinoOTA.setHostname(deviceName);

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
              client.println("<body><h1>ESP8266 Web Server</h1>");

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

  if(initialSetupMode){
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

}

void publishDebugJson(JsonObject json){
  char mqttJson[50];
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
      client.publish(coverDebugTopic.c_str(), "Min Position Set", false);
      publishDebugJson(minJson);
      if(helper.saveconfig(minJson)){
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
      DynamicJsonDocument maxDoc(50);
      JsonObject maxJson = maxDoc.to<JsonObject>();
      maxJson["min"] = minPosition;
      maxJson["max"] = maxPosition;
      maxJson["current"] = currentPosition;
      client.publish(coverDebugTopic.c_str(), "Max Position Set", false);
      publishDebugJson(maxJson);
      if(helper.saveconfig(maxJson)){
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
    mqttDevConfig["name"] = deviceName;
    mqttDevConfig["mf"] = manufacturer;
    mqttDevConfig["mdl"] = model;
    mqttDevConfig["sw"] = softwareVersion;
    mqttDevConfig["ids"][0] = mqttCoverDeviceClientId;
    mqttDevConfig["ids"][1] = mqttResetDeviceClientId;
    mqttDevConfig["ids"][2] = mqttMinDeviceClientId;
    mqttDevConfig["ids"][3] = mqttMaxDeviceClientId;
    
    DynamicJsonDocument mqttCoverConfig(650);
    mqttCoverConfig["name"] = deviceName;
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
    mqttResetConfig["name"] = "Reset";
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
      mqttMinConfig["name"] = "Set Min";
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
      mqttMaxConfig["name"] = "Set Max";
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

void printWiFiStatus() {

  // print the SSID of the network you're attached to:

  Serial.print("SSID: ");

  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:

  IPAddress ip = WiFi.localIP();

  Serial.print("IP Address: ");

  Serial.println(ip);

  // print where to go in a browser:

  Serial.print("To see this page in action, open a browser to http://");

  Serial.println(ip);

}
