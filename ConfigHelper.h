#ifndef ConfigHelper_h
#define ConfigHelper_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "FS.h"

class ConfigHelper {
  public:
    ConfigHelper();
    boolean loadconfig();
    JsonObject getconfig();
    boolean saveconfig(JsonObject json);
    void deletefile();
    void printfile();

  private:
    JsonObject _config;
    String _configfile;
};

#endif
