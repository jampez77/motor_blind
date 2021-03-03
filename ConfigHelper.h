#ifndef ConfigHelper_h
#define ConfigHelper_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "FS.h"


struct Config {
  long currentPosition;
  long maxPosition;
  long minPosition;
  boolean initialSetup;
  char deviceName[25];
  char mqttUser[25];
  char mqttPassword[25];
  char mqttServerIp[25];
  char mqttServerPort[25];
};


class ConfigHelper {
  public:
    ConfigHelper();
    boolean loadconfig();
    Config getconfig();
    boolean saveconfig(Config config);
    void deletefile();
    void printfile();

  private:
    JsonObject _config;
    String _configfile;
};

#endif
