# Esp8266-mqtt-codegenerator

## Introduction

This is a java based code generated that can be used to generate code for flashing into the ESP8266 on a mass scale or for a single device. This code generation is only for the MQTT based data collection from the edge device. 

## Data Format 

The code by default send data as JSON from the edge device to the Server.  The example format will be 

{ 
“Temperature” : “22”
“Humidity” : “47”
“Time” : “1519286030”
“Client_id”:”humidity-sensor-production-floor”
} 

The number of fields and the value type can be specified by the developer during the generation of the code.  The time and the client field can be automatically added baed on the option yes or no. 

## Code Generation 

You will be able to specify the following details as inputs for the code generation. 

1. Parameters to be send on the message and the data type
2. Topic to which the message to be published 
3. Time interval between each publish
4. MQTT Server Name and Remote Port
5. Wireless Access point details if behind the firewall. 

The code generation follows two modes 
1. Command line 
2. Automated Generation 

### Command line
This option can be mostly used for a single device code generation 

### Automated Generation 
This option can be used when you are doing mass production of sensors and you can integrated the code generation into your system for the code generation by varying the client id, authentication, wifi parameters  and the topic name as needed. 

The java API document will provide the necessary API for the auto generation  available under the [JavaDoc](https://github.com/bevywise-networks/Esp8266-mqtt-codegenerator/tree/master/javadoc) folder  or available online at [Bevywise Code Generation API](https://www.bevywise.com/esp8266-code-generation/com/bevywise/iot/client/CodeGenForESP8266.html) . 

## Dependencies 

The below packages should be added to the  Arduino->Libraries/ Folder

1.pubsubclient Library - https://github.com/knolleary/pubsubclient
2.ArduinoJson Library -  https://github.com/bblanchon/ArduinoJson 
3.WiFiManager Library -   Go to https://github.com/tzapu/WiFiManager 
4.NPClient Library -  https://github.com/arduino-libraries/NTPClient


## How to Manual

1. [Set up](https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon) your Arduino IDE and the required tools for the ESP8266 
2. Download the dependencies and paste it in Arduino->Libraries Folder
3. Clone the repo - Go to Source/ folder  
4. Compile the Java Code - javac - d . com/bevywise/iot/client/CodeGenForESP8266.java
5. Run the code - java com.bevywise.iot.client.CodeGenForESP8266
6. Provide the details needed and get the code generator. 
7. Copy paste the Instrumentation library in the Arduino / Libraries folder of Arduino. 
8. Once the code is generated, Create a new file in Arduino and copy paste the content of ESPMain.cpp
9. If you are going to send the time data from the agent, copy paste the time 
10. You may need to customise the code to read the data from the GPIO and use the necessary logic to convert the data. 


## Support 
If you have any questions you can connect with us via support
