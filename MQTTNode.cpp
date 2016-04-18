#include "MQTTNode.h"

MQTTNode::MQTTNode(const char* name, const char* host, uint16_t port){
	
	init(name, host, port);
}

MQTTNode::MQTTNode(const char* name, const char* host, uint16_t port, const char* username, const char* password){
	
	init(name, host, port);
	this->mqtt_username = username;
	this->mqtt_password = password;
}

void MQTTNode::init(const char* name, const char* host, uint16_t port) {

	this->name = name;
	this->mqtt_host = host;
	this->mqtt_port = port;
}

PubSubClient MQTTNode::getClient() {

	return client;
}

void MQTTNode::reconnect() {

	// Loop until we're reconnected
	while (!client.connected()) {
		MQTTNODE_PRINT("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect(name, mqtt_username, mqtt_password)) {
			MQTTNODE_PRINTLN("connected");
			if (this->connectedCallback) {
				this->connectedCallback(*this);
			}
		}
		else {
			MQTTNODE_PRINT("failed, rc=");
			MQTTNODE_PRINT(client.state());
			MQTTNODE_PRINTLN(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

ESP8266MQTTNode::ESP8266MQTTNode(const char* name, const char* host, uint16_t port) :
	MQTTNode(name, host, port) {

	this->client.setClient(this->wifiClient);
}

ESP8266MQTTNode::ESP8266MQTTNode(const char* name, const char* host, uint16_t port, const char* username, const char* password) :
	MQTTNode(name, host, port, username, password) {

	this->client.setClient(this->wifiClient);
}

void ESP8266MQTTNode::setupWifi(const char* ssid, const char* password) {

	delay(10);
	// We start by connecting to a WiFi network
	MQTTNODE_PRINTLN();
	MQTTNODE_PRINT("Connecting to ");
	MQTTNODE_PRINTLN(ssid);

	WiFi.begin(ssid, password);

	uint32_t connectionTimeoutMs = 10000;
	uint32_t connectionAttemptStart = millis();

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		MQTTNODE_PRINT(".");

		if (millis() - connectionAttemptStart > connectionTimeoutMs) {
			break;
		}
	}

	MQTTNODE_PRINTLN("");
	MQTTNODE_PRINTLN("WiFi connected");
	MQTTNODE_PRINTLN("IP address: ");
	MQTTNODE_PRINTLN(WiFi.localIP());
}

void ESP8266MQTTNode::connect(const char* ssid, const char* password, MQTTNodeOnConnectedCallback connectCallback, MQTT_CALLBACK_SIGNATURE) {

	MQTTNODE_PRINTLN("Connect");
	setupWifi(ssid, password);
	MQTTNODE_PRINTLN("Set server");
	client.setServer(mqtt_host, mqtt_port);
	MQTTNODE_PRINTLN("Set callback");
	client.setCallback(callback);
	this->connectedCallback = connectCallback;

	// Ensure the client is connected
	loop();
}

void ESP8266MQTTNode::loop() {

	// Ensure we're still connected to the MQTT server
	if (!client.connected()) {
		reconnect();
	}

	// Check for any new broadcasts
	client.loop();
}