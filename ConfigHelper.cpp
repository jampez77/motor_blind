#include "Arduino.h"
#include "ConfigHelper.h"



ConfigHelper::ConfigHelper(){
  this->_configfile = "/config.json";
}

Config config;

boolean ConfigHelper::loadconfig(){
  // Open file for reading
  File configFile = SPIFFS.open(_configfile, "r");
  if (!configFile) {
    ///Serial.println("Failed to load config file");
    return false;
  } else {
     // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<250> doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
    if (error)
      Serial.println(F("Failed to read file, using default configuration"));
    // Close the file (Curiously, File's destructor doesn't close the file)
    configFile.close();
  }
  return true;
}

Config ConfigHelper::getconfig(){
  // Open file for reading
  File configFile = SPIFFS.open(_configfile, "r");

  
   // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<250> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
    Serial.println(F("Failed to get file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  config.currentPosition = doc["current"];
  config.minPosition = doc["min"];
  config.maxPosition = doc["max"];
  config.initialSetup = doc["setup"];


  strlcpy(config.deviceName,                
          doc["d_name"] | "",  
          sizeof(config.deviceName));    

  Serial.println("Printing config file: ");
  printfile();
  Serial.println(" ");
  strlcpy(config.mqttUser,                
          doc["mqtt_u"] | "",  
          sizeof(config.mqttUser));    
  
  strlcpy(config.mqttPassword,                
          doc["mqtt_pw"] | "",  
          sizeof(config.mqttPassword));    
          
  strlcpy(config.mqttServerIp,                
          doc["mqtt_ip"] | "",  
          sizeof(config.mqttServerIp));    
  
  strlcpy(config.mqttServerPort,                
          doc["mqtt_p"] | "1883",  
          sizeof(config.mqttServerPort));    

  // Close the file (Curiously, File's destructor doesn't close the file)
  configFile.close();


  
  return config;
}

void ConfigHelper::deletefile(){
  SPIFFS.remove(_configfile);
}

boolean ConfigHelper::saveconfig(Config config){
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(_configfile);

  // Open file for writing
  File configFile = SPIFFS.open(_configfile, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<250> doc;

  // Set the values in the document
  doc["current"] = config.currentPosition;
  doc["min"] = config.minPosition;
  doc["max"] = config.maxPosition;
  doc["setup"] = config.initialSetup;
  doc["d_name"] = config.deviceName;
  doc["mqtt_u"] = config.mqttUser;
  doc["mqtt_pw"] = config.mqttPassword;
  doc["mqtt_ip"] = config.mqttServerIp;
  doc["mqtt_p"] = config.mqttServerPort;
  
  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  configFile.close();
  
  return true;
}

// Prints the content of a file to the Serial
void ConfigHelper::printfile() {
  // Open file for reading
  File configFile = SPIFFS.open(_configfile, "r");
  if (!configFile) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (configFile.available()) {
    Serial.print((char)configFile.read());
  }
  Serial.println();

  // Close the file
  configFile.close();
}
