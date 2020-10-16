#include "Arduino.h"
#include "ConfigHelper.h"

struct Config {
  long currentPosition;
  long maxPosition;
  long minPosition;
};

ConfigHelper::ConfigHelper(){
  this->_configfile = "/config.json";
}

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
    StaticJsonDocument<50> doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
    if (error)
      Serial.println(F("Failed to read file, using default configuration"));
    // Close the file (Curiously, File's destructor doesn't close the file)
    configFile.close();
  }
  return true;
}

JsonObject ConfigHelper::getconfig(){
  StaticJsonDocument<50> configJsonDoc;
  JsonObject configJson = configJsonDoc.to<JsonObject>();
  
  // Open file for reading
  File configFile = SPIFFS.open(_configfile, "r");
  if (!configFile) {
    Serial.println("Failed to get config file");
    // Return default values if we have no config file.
    configJson["current"] = 0;
    configJson["min"] = -1;
    configJson["max"] = -1;
    return configJson;
  }

   // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
    Serial.println(F("Failed to get file, using default configuration"));
      
  // Copy values from the JsonDocument to the Config
  configJson["current"] = doc["current"];
  configJson["min"] = doc["min"];
  configJson["max"] = doc["max"];

  // Close the file (Curiously, File's destructor doesn't close the file)
  configFile.close();

  Serial.println("Printing config file: ");
  serializeJson(configJson, Serial);
  Serial.println(" ");
  
  return configJson;
}

void ConfigHelper::deletefile(){
  SPIFFS.remove(_configfile);
}

boolean ConfigHelper::saveconfig(JsonObject json){
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
  StaticJsonDocument<50> doc;

  // Set the values in the document
  doc["current"] = json["current"];
  doc["max"] = json["max"] ;
  doc["min"] = json["min"];
  
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
