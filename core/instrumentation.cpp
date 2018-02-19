#include<instrumentation.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
bool shouldSaveConfig = false;
const int sleepTimeS = 20;
unsigned int localPort = 2390;      
IPAddress timeServer(129, 6, 15, 28); 
IPAddress timeServerIP; 
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; 
byte packetBuffer[ NTP_PACKET_SIZE]; 
WiFiUDP udp;
int i=0;
unsigned long epoch;
WiFiClient espClient;
PubSubClient client(espClient);


void saveConfigCallback () 
{
	Serial.println("Should save config");
	shouldSaveConfig = true;

}

int stringToNumber(String thisString) 
{
	int i, value, length;
	length = thisString.length();
	char blah[(length + 1)];
	for (i = 0; i < length; i++) 
	{
		blah[i] = thisString.charAt(i);
	}
	blah[i] = 0;
	value = atoi(blah);
	return value;

}

unsigned long sendNTPpacket(IPAddress& address)
{
	Serial.println("sending NTP packet...");
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	packetBuffer[0] = 0b11100011;   
	packetBuffer[1] = 0;     
	packetBuffer[2] = 6;     
	packetBuffer[3] = 0xEC;  
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;
	udp.beginPacket(address, 123); 
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}

void reconnect(char* data,const char* CLIENT_ID,const char* user_name,const char* pass_word,const char* topic) 
{
	if (!client.connected()) 
	{
		Serial.print("Attempting MQTT connection...");
		Serial.println(user_name);
		Serial.println(pass_word);
		if (client.connect(CLIENT_ID,user_name,pass_word)) 
		{
			Serial.println("connected");
			client.subscribe(topic);
			client.publish(topic,data);
		} 
		else 
		{

			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			delay(5000);
		}
	}
}


void callback(char* topic, byte* payload, unsigned int length) 
{
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++)
	{
		Serial.print((char)payload[i]);
	}
	Serial.println();
}

void setdata(char* data,int buttonpin ,const char* WIFI_SSID,const char* WIFI_PASSWORD,const char* mqtt_server,const char* mqtt_port,const char* user_name,const char* pass_word,const char* CLIENT_ID,const char* topic) 
{


	if(digitalRead(buttonpin) == LOW)
	{ 

		Serial.println("mounting FS");
		if (SPIFFS.begin()) 
		{
			Serial.println("mounted file system");
			if (SPIFFS.exists("/config.json")) 
			{

				Serial.println("reading config file");
				File configFile = SPIFFS.open("/config.json", "r");
				if (configFile) 
				{
					Serial.println("opened config file");
					size_t size = configFile.size();
					std::unique_ptr<char[]> buf(new char[size]);
					configFile.readBytes(buf.get(), size);
					DynamicJsonBuffer jsonBuffer;
					JsonObject& json = jsonBuffer.parseObject(buf.get());
					json.printTo(Serial);
					if (json.success()) 
					{
						Serial.println("\nparsed json");
						user_name=json["user_name"];
						pass_word=json["pass_word"];
						mqtt_server=json["mqtt_server"];
						mqtt_port=json["mqtt_port"];
					} 
					else 
					{
						Serial.println("failed to load json config");
					}
				}
			}
		} 
		else 
		{
			Serial.println("failed to mount FS");
		}
		WiFiManagerParameter custom_user_name("user_name", "user name", user_name, 40);
		WiFiManagerParameter custom_pass_word("pass_word", "pass word", pass_word, 40);  
		WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
		WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
		WiFiManager wifiManager;
		wifiManager.setSaveConfigCallback(saveConfigCallback);
		wifiManager.addParameter(&custom_user_name);
		wifiManager.addParameter(&custom_pass_word);
		wifiManager.addParameter(&custom_mqtt_server);
		wifiManager.addParameter(&custom_mqtt_port);
		Serial.begin(115200);
		Serial.println("\n Starting");
		WiFi.printDiag(Serial);  
		if (!wifiManager.startConfigPortal()) 
		{
			Serial.println("failed to connect and hit timeout");

		}

		Serial.println("connected... :)");
		user_name=custom_user_name.getValue();
		pass_word=custom_pass_word.getValue();
		mqtt_server=custom_mqtt_server.getValue();
		mqtt_port=custom_mqtt_port.getValue();
		if (shouldSaveConfig) 
		{
			Serial.println("saving config");
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			json["user_name"]= user_name;
			json["pass_word"]= pass_word;
			json["mqtt_server"] = mqtt_server;
			json["mqtt_port"] = mqtt_port;

			File configFile = SPIFFS.open("/config.json", "w");
			if (!configFile) 
			{
				Serial.println("failed to open config file for writing");
			}

			json.printTo(Serial);
			json.printTo(configFile);
			configFile.close();
		}

		Serial.println("local ip");
		Serial.println(WiFi.localIP());
		udp.begin(localPort);
		Serial.print("Local port: ");
		Serial.println(udp.localPort());
		WiFi.hostByName(ntpServerName, timeServerIP); 

		sendNTPpacket(timeServerIP); 
		delay(1000);

		int cb = udp.parsePacket();
		if (!cb) {
			Serial.println("no packet yet");
		}
		else {
			Serial.print("packet received, length=");
			Serial.println(cb);
			udp.read(packetBuffer, NTP_PACKET_SIZE);

			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

			unsigned long secsSince1900 = highWord << 16 | lowWord;
			Serial.print("Seconds since Jan 1 1900 = " );
			Serial.println(secsSince1900);

			Serial.print("Unix time = ");
			const unsigned long seventyYears = 2208988800UL;
			epoch = secsSince1900 - seventyYears;
			Serial.println(epoch);
		}





		client.setServer(mqtt_server, 1883);
		reconnect(data,CLIENT_ID,user_name,pass_word,topic);
		client.setCallback(callback);



	}

	if(WiFi.SSID()!="")
	{
		delay(10000);

		WiFi.mode(WIFI_STA);

		Serial.print(WiFi.SSID());

		unsigned long startedAt = millis();
		Serial.print("After waiting ");
		int connRes = WiFi.waitForConnectResult();
		float waited = (millis()- startedAt);
		Serial.print(waited/1000);
		Serial.print(" secs in setup() connection result is ");
		Serial.println(connRes);

		pinMode(buttonpin, INPUT_PULLUP);

		if (WiFi.status()!=WL_CONNECTED)
		{
			Serial.println("failed to connect, finishing setup anyway");
		} 
		else
		{

			Serial.print("local ip: ");
			Serial.println(WiFi.localIP());
			if (SPIFFS.begin()) {
				Serial.println("mounted file system");
				if (SPIFFS.exists("/config.json")) {

					Serial.println("reading config file");
					File configFile = SPIFFS.open("/config.json", "r");
					if (configFile) 
					{
						Serial.println("opened config file");
						size_t size = configFile.size();

						std::unique_ptr<char[]> buf(new char[size]);
						configFile.readBytes(buf.get(), size);
						DynamicJsonBuffer jsonBuffer;
						JsonObject& json = jsonBuffer.parseObject(buf.get());
						json.printTo(Serial);
						if (json.success()) 
						{
							Serial.println("\nparsed json");
							user_name=json["user_name"];
							pass_word=json["pass_word"];
							mqtt_server=json["mqtt_server"];
							mqtt_port, json["mqtt_port"];
							udp.begin(localPort);
							Serial.print("Local port: ");
							Serial.println(udp.localPort());
							WiFi.hostByName(ntpServerName, timeServerIP); 

							sendNTPpacket(timeServerIP); 

							delay(1000);

							int cb = udp.parsePacket();
							if (!cb) {
								Serial.println("no packet yet");
							}
							else {
								Serial.print("packet received, length=");
								Serial.println(cb);

								udp.read(packetBuffer, NTP_PACKET_SIZE); 


								unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
								unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

								unsigned long secsSince1900 = highWord << 16 | lowWord;
								Serial.print("Seconds since Jan 1 1900 = " );
								Serial.println(secsSince1900);

								Serial.print("Unix time = ");

								const unsigned long seventyYears = 2208988800UL;

								epoch = secsSince1900 - seventyYears;

								Serial.println(epoch);
							}



							client.setServer(mqtt_server, 1883);
							reconnect(data,CLIENT_ID,user_name,pass_word,topic);
							client.setCallback(callback);




						}
					}
				}
			}
		}
	}

	if(WiFi.SSID()=="") 
	{
		Serial.println("mounting FS...");
		if (SPIFFS.begin()) 
		{
			Serial.println("mounted file system");
			if (SPIFFS.exists("/config.json")) 
			{

				Serial.println("reading config file");
				File configFile = SPIFFS.open("/config.json", "r");
				if (configFile) 
				{
					Serial.println("opened config file");
					size_t size = configFile.size();
					std::unique_ptr<char[]> buf(new char[size]);
					configFile.readBytes(buf.get(), size);
					DynamicJsonBuffer jsonBuffer;
					JsonObject& json = jsonBuffer.parseObject(buf.get());
					json.printTo(Serial);
					if (json.success()) 
					{
						Serial.println("\nparsed json");
						user_name=json["user_name"];
						pass_word=json["pass_word"];
						mqtt_server=json["mqtt_server"];
						mqtt_port=json["mqtt_port"];

					} 
					else 
					{
						Serial.println("failed to load json config");
					}
				}
			}
		} 
		else 
		{
			Serial.println("failed to mount FS");
		}

		WiFiManagerParameter custom_user_name("user_name", "user name", user_name, 40);
		WiFiManagerParameter custom_pass_word("pass_word", "pass word", pass_word, 40);
		WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
		WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
		WiFiManager wifiManager;
		wifiManager.setSaveConfigCallback(saveConfigCallback);

		wifiManager.addParameter(&custom_user_name);
		wifiManager.addParameter(&custom_pass_word);
		wifiManager.addParameter(&custom_mqtt_server);
		wifiManager.addParameter(&custom_mqtt_port);

		Serial.begin(115200);
		Serial.println("\n Starting");
		WiFi.printDiag(Serial); 

		if (!wifiManager.startConfigPortal()) 
		{
			Serial.println("failed to connect and hit timeout");

		}


		Serial.println("connected)");
		user_name=custom_user_name.getValue();
		pass_word=custom_pass_word.getValue();
		mqtt_server=custom_mqtt_server.getValue();
		mqtt_port=custom_mqtt_port.getValue();
		if (shouldSaveConfig) 
		{
			Serial.println("saving config");
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			json["user_name"]= user_name;
			json["pass_word"]=pass_word;
			json["mqtt_server"] = mqtt_server;
			json["mqtt_port"] = mqtt_port;

			File configFile = SPIFFS.open("/config.json", "w");
			if (!configFile) 
			{
				Serial.println("failed to open config file for writing");
			}
			json.printTo(Serial);
			json.printTo(configFile);
			configFile.close();

		}

		Serial.println("local ip");
		Serial.println(WiFi.localIP());
		udp.begin(localPort);
		Serial.print("Local port: ");
		Serial.println(udp.localPort());
		WiFi.hostByName(ntpServerName, timeServerIP); 

		sendNTPpacket(timeServerIP); 

		delay(1000);

		int cb = udp.parsePacket();
		if (!cb) {
			Serial.println("no packet yet");
		}
		else {
			Serial.print("packet received, length=");
			Serial.println(cb);

			udp.read(packetBuffer, NTP_PACKET_SIZE); 


			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

			unsigned long secsSince1900 = highWord << 16 | lowWord;
			Serial.print("Seconds since Jan 1 1900 = " );
			Serial.println(secsSince1900);

			Serial.print("Unix time = ");

			const unsigned long seventyYears = 2208988800UL;

			epoch = secsSince1900 - seventyYears;

			Serial.println(epoch);
		}


		client.setServer(mqtt_server, 1883);
		reconnect(data,CLIENT_ID,user_name,pass_word,topic);
		client.setCallback(callback);


	}
}


void senddata(char* data,const char* mqtt_server,const char* mqtt_port,const char* user_name,const char* pass_word,const char* CLIENT_ID,const char* topic) 
{

	if (!client.connected())
	{


		reconnect(data,CLIENT_ID,user_name,pass_word,topic);

	} 

	for(i=0;i<1000;i++)
	{
		client.loop();
	}

	delay(5000);

	client.disconnect();

}
