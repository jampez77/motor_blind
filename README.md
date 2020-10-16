# Home Assistant Simple Motorised Roller Blind

![Home Assistant Motorised Roller Blind](files/repo_image.png#center)

This project was inspired by the excellent work of [nidayand](https://github.com/nidayand) and their [motor-on-roller-blind-ws](https://github.com/nidayand/motor-on-roller-blind-ws) project. Basically I have stripped that project back to it's bare bones and implemented MQTT configuration to allow my [Home Assistant](https://home-assistant.io) to discovery the entities.

Additionally I wanted all the configuration to be done in Home Assistant so I created a set min / max switches that disappear once the blinds are set up.

## Prerequisites ##

**Software**
* [Arduino IDE](https://www.arduino.cc/en/main/software) - I'm using v1.8.13
* Stepper_28BYJ_48 - v1.0.0
* PubSubClient - v2.7.0
* ESP8266WiFi
* ArduinoOTA

**Hardware**

* [ULN2003 Board Driver with Stepper Motor](https://www.amazon.co.uk/gp/product/B07RLLKFGK/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1)
* [Node MCU](https://www.amazon.co.uk/AZDelivery-NodeMcu-Amica-Development-including/dp/B06Y1LZLLY)
* [Motor on a roller blind 3D printed holder](https://www.thingiverse.com/thing:2392856)
* 9V - 12V DC power supply.

## Setup ##

**Flashing project to the NodeMcu**

Once you have setup your software, you first need to download this project, by clicking `clone -> Download ZIP`.

Unzip the file and navigate to the contents. you should see a file named `My_Helper_sample.h`, go ahead and rename it to `My_Helper.h`.

Now open `motor_blind.ino`. Doing so should open it in the [Arduino IDE](https://www.arduino.cc/en/main/software).

You do not need to make any changes to the `motor_blind` script itself!

Click on the `My_Helper.h` tab, that is where you will add your WiFi credentials for your home network & your MQTT details.

**Wiring**

Wiring is very similar to the original version described [here](https://www.thingiverse.com/thing:2392856) with one key difference. We connect `IN4` to `D5` as I `D4` on the NodeMCU is the builtin LED, that is used to show any connection problem.

Connect 9v directly to Vin on NodeMCU board (validate first that your version has a built in regulator for 9V) and to the ULN2003 board.

5V is simply not enough to be able to control the blinds, even with the higher gear ratio, so ULN2003 requires 9V.

Connect the board to the NodeMCU as follows:

`D1` = `IN1`, `D2` = `IN3`, `D3` = `IN2`, `D5` = `IN4`


![Home Assistant Sommer Conex wiring diagram](files/motor_blind_fritz.png)

## Upload ##

The next step, once you're happy with the details is to upload the project to the Wemos D1 Mini.

To do that you select `Tools` from the top menu within the IDE.

Then you need to set the upload parameters to the following;

![Arduino IDE Upload settings](files/upload_settings.png)

The next step is to simply plug your device in the computer, Go to `Tools -> Port` and select it.

For me this usually says something `dev/usbserial...`

You will only need the device plugged in on your first upload, after that you should see your devices name and it's IP address in the port list.

Now you just click on the upload button in the top left corner of the IDE, it's the right-facing arrow.

After a bit of time compiling, the script should be uploaded to the device and begin scanning for the car.

You can see what the device is doing by going to `Tools -> Serial Monitor`

If the upload doesn't work the most likely cause is that you forgot to rename the `My_Helper_sample.h` file or import the above libraries.

## Integrating with Home Assistant ##

The simplest way to integrate with [Home Assistant](https://home-assistant.io) is to turn on [MQTT discovery](https://www.home-assistant.io/docs/mqtt/discovery/). With this activated the device will be added automatically.

## Configuring up the blinds ##

One first connection you will see 4 entities `set min`, `set max` `reset` & `blinds`.

![Home Assistant Configuration](files/ha_config.png)

You should manually set the blinds as open, to the level you are happy to regard as fully open. Once this is done you should `turn on` the `Set Min` switch. This will save the current position as the open value, then that entity will disappear front Home Assistant.

Next step is to close the blinds using the cover entity. Stop the blinds once they reach a position that you're happy to regard as closed. Now `turn on` the `Set Max` switch to save the closed position.

The setup is complete once the `Set Max` entity has disappeared. If you ever need to change the open or close values you can simply `turn on` the `Reset` switch. That will erase the memory on the NodeMCU and initiate a fresh install.

## Changelog ##

### Version 1.0 - Initial Release ###

#### LED status indicator ####
* Solid light = No WiFi connection
* Flashing Light = no MQTT connection
* No light = connection established

ArduinoOTA for wireless code updates.

Added MQTT config on boot to integrate seamlessly with Home Assistant.
