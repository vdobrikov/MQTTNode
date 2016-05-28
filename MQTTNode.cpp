#include "MQTTNode.h"

MQTTNode::MQTTNode(const char* name, const char* host){

	init(name, host, DEFAULT_MQTT_PORT);
}

MQTTNode::MQTTNode(const char* name, const char* host, uint16_t port){

	init(name, host, port);
}

MQTTNode::MQTTNode(const char* name, const char* host, uint16_t port, const char* mqttUsername, const char* mqttPassword){
	init(name, host, port);
	this->mqtt_username = mqttUsername;
	this->mqtt_password = mqttPassword;
}

void MQTTNode::init(const char* name, const char* host, uint16_t port) {
	this->name = name;
	client.setServer(host, port);
}

PubSubClient* MQTTNode::getClient() {
	return &client;
}

void MQTTNode::reconnect() {
	if (name == NULL) {
		name = DEFAULT_NODE_NAME;
	}

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

bool MQTTNode::isMqttConnected() {
	return client.connected();
}



ESP8266MQTTNode::ESP8266MQTTNode(const char* name, const char* host) :
	MQTTNode(name, host) {

	this->client.setClient(this->wifiClient);
}

ESP8266MQTTNode::ESP8266MQTTNode(const char* name, const char* host, uint16_t port) :
	MQTTNode(name, host, port) {

	this->client.setClient(this->wifiClient);
}

ESP8266MQTTNode::ESP8266MQTTNode(const char* name, const char* host, uint16_t port, const char* mqttUsername, const char* mqttPassword) :
	MQTTNode(name, host, port, mqttUsername, mqttPassword) {

	this->client.setClient(this->wifiClient);
}

ESP8266MQTTNode::ESP8266MQTTNode(const char* host, const char* mqttUsername, const char* mqttPassword, const char* wifiSsid, const char* wifiPassword, MQTTNodeOnConnectedCallback connectCallback, MQTT_CALLBACK_SIGNATURE) :
	MQTTNode(NULL, host, DEFAULT_MQTT_PORT, mqttUsername, mqttPassword) {

	this->wifi_ssid = wifiSsid;
	this->wifi_password = wifiPassword;
	this->connectedCallback = connectCallback;
	this->client.setCallback(callback);
	this->client.setClient(wifiClient);
}

const char* ESP8266MQTTNode::generateNodeName() {
  return String(ESP.getChipId()).c_str();
}

void ESP8266MQTTNode::setupWifi(const char* ssid, const char* wifiPassword) {

	delay(10);
	// We start by connecting to a WiFi network
	MQTTNODE_PRINTLN();
	MQTTNODE_PRINT("Connecting to ");
	MQTTNODE_PRINTLN(ssid);

	WiFi.begin(ssid, wifiPassword);

	uint32_t connectionTimeoutMs = 10000;
	uint32_t lastConnectionRetry = millis();

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		MQTTNODE_PRINT(".");

		if (millis() - lastConnectionRetry > connectionTimeoutMs) {
			break;
		}
	}

	MQTTNODE_PRINTLN("");
	MQTTNODE_PRINT("WiFi connected: ");
	MQTTNODE_PRINTLN(WiFi.status() == WL_CONNECTED);
	MQTTNODE_PRINTLN("IP address: ");
	MQTTNODE_PRINTLN(WiFi.localIP());
}

bool ESP8266MQTTNode::isWifiConnected() {
	return WiFi.status() == WL_CONNECTED;
}

void ESP8266MQTTNode::begin() {
	if (name == NULL) {
		name = generateNodeName();
	}
	MQTTNODE_PRINTLN("Connecting");
	setupWifi(wifi_ssid, wifi_password);

	// Ensure the client is connected
	reconnect();
}

void ESP8266MQTTNode::loop() {

	// Ensure we're still connected to WiFi
	if (!isWifiConnected()) {
		setupWifi(wifi_ssid, wifi_password);
	}

	// Ensure we're still connected to the MQTT server
	if (!isMqttConnected()) {
		reconnect();
	}

	// Check for any new broadcasts
	client.loop();
}
