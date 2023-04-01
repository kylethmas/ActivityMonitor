# CENSIS JacNVicta ESP32
This repository contains all the code which runs on the ESP32 microcontroller.  
It's responsible for sampling audio data from an i2s microphone, preprocessing
any recorded data, running the classification algorithm, and sending the results 
to a remote webserver for monitoring.

## Building
This project makes use of the Arduino platform, which provides a standard 
library and manages the compiler toolchain.

To build the project download and install the 
[Arduino IDE](https://www.arduino.cc/en/software) (note that there is also a 
command line version of Arduino which can be used but won't be detailed here). 
VSCode or CLion can also be used as they have compatibility for Arduino projects 
with an extension.
 - In the Arduino IDE open the `SensorESP32.ino` file.
 - Now to install the board profile, go to `File->Preferences` and then in the "Additional board manage URLs" field add the following:
   - `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` If you're using the ESP LyraT dev kit
 - Close the settings window and go to `Tools->Board: ->Board Manager...`
 - Search for "ESP32" and install whichever package is listed
 - Now select the board:
   - `Tools->Board: ->esp32->Heltec WiFi Kit 32` If you're using the Heltec WiFi Kit 32 dev kit
   - `Tools->Board: ->esp32->ESP32 Wrover Kit (all versions)` If you're using the ESP LyraT dev kit
 - Install the following libraries by going to `Sketch->Include Library->Manage Libraries...` and search for and install the following:
   - `ES8388` (also available at: [https://github.com/vanbwodonk/es8388arduino])
   - `TensorFlowLite_ESP32`
   - `Adafruit SSD1306` (only required when building for the Heltec Wifi Kit 32)
 - Now connect the devkit to your computer and select the COM port it's connected to:
   - `Tools->Port: ->COM3` The number will depend on what COM port it's connected to normally there's only one option in this list anyway
 - Change partition scheme to Huge APP (3MB No OTA)
   - `Tools->Partition Scheme: ->Huge APP (3MB No OTA)`
 - Finally click upload to compile and upload your code

## Project Organisation
The project is organised as follows:
```
SensorESP32 (root)
├─── SensorESP32.ino                        | The entrypoint to the application
├─── README.md                              | This file
├─── .vscode
|    ├─── arduino.json                      | VSCode specific settings for the Arduino platform config
|    ├─── settings.json                     | VSCode specific settings for syntax highlighting
├─── src
     ├─── modules
          ├─── audio_input.cpp/.h           | 
          ├─── classification.cpp/.h        |
          ├─── communication.cpp/.h         |
          ├─── config.h                     |
          ├─── debug.h                      |
          ├─── dsp.cpp/.h                   |
          ├─── utils.h                      |
```
