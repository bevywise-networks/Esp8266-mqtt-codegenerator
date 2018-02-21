#include <ESP8266WiFi.h>
#include <FS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
void setdata(char*,int,const char*,const char*,const char*,const char*,const char*,const char*,const char*,const char*);
void senddata(char*,const char*,const char*,const char*,const char*,const char*,const char*);
