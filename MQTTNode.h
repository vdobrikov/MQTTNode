#ifndef MQTTNODE_H_
#define MQTTNODE_H_

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTTNODE_DEBUG

#if defined(MQTTNODE_DEBUG)
#define MQTTNODE_PRINT(str) Serial.print(str)
#define MQTTNODE_PRINTLN(str) Serial.println(str)
#else
#define MQTTNODE_PRINT(str)
#define MQTTNODE_PRINTLN(str)
#endif

#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_NODE_NAME "MQTTNode"

// Forward definition
class MQTTNode;

typedef void(*MQTTNodeOnConnectedCallback)(MQTTNode&);

class MQTTNode {

	protected:
		const char* name = NULL;
		const char* mqtt_username = NULL;
		const char* mqtt_password = NULL;

		PubSubClient client;
		MQTTNodeOnConnectedCallback connectedCallback;

		void init(const char*, const char*, uint16_t);
		void reconnect();
        bool isMqttConnected();

	public:
        MQTTNode(const char*, const char*);
		MQTTNode(const char*, const char*, uint16_t);
		MQTTNode(const char*, const char*, uint16_t, const char*, const char*);

		virtual void loop() = 0;
		PubSubClient* getClient();
};

class ESP8266MQTTNode : public MQTTNode{

	private:
		WiFiClient wifiClient;

		const char* wifi_ssid = NULL;
		const char* wifi_password = NULL;

		void setupWifi(const char*, const char*);
        bool isWifiConnected();
        const char* generateNodeName();

	public:
        ESP8266MQTTNode(const char*, const char*);
		ESP8266MQTTNode(const char*, const char*, uint16_t);
		ESP8266MQTTNode(const char*, const char*, uint16_t, const char*, const char*);
        ESP8266MQTTNode(const char*, const char*, const char*, const char*, const char*, MQTTNodeOnConnectedCallback, MQTT_CALLBACK_SIGNATURE);

		void begin();
		virtual void loop();

};


#endif // MQTTNODE_H_
