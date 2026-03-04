#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GPS.h>
#include "Adafruit_APDS9960.h"

#define SLEEP_SECONDS 5

// Sender and Receiver MAC addresses
const uint8_t MAC_SENDER_1[]   = { 0x34, 0xB7, 0xDA, 0xF6, 0x3C, 0x34 };
const uint8_t MAC_RECEIVER_1[] = { 0x34, 0xB7, 0xDA, 0xF6, 0x39, 0x74 };

// Packet Structure
typedef struct Packet {
    uint32_t id;
    short int alert;
    float latitude;
    float longitude;
    float altitude;
	// output from color sensor
	uint16_t color_r, color_g, color_b, color_c;
} Packet;

// UART DEFINITIONS
#define GPSSerial Serial1
#define GPS_RX 20
#define GPS_TX 21

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set to true to echo raw NMEA
#define GPSECHO false

Adafruit_APDS9960 apds;

unsigned long timer = 0;
uint32_t packetCounter = 0;
short int systemAlert = 0;
uint16_t color_r = 0, color_g = 0, color_b = 0, color_c = 0;

// ESP-NOW Callback Function
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
    Serial.printf("Packet #%d Send Status: ", packetCounter);
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Send Packet Function
void SendPacket() {	
    Packet packet;

    packet.id = packetCounter++;
    packet.alert = systemAlert; // No change currently
    packet.latitude = GPS.latitudeDegrees;
    packet.longitude = GPS.longitudeDegrees;
    packet.altitude = GPS.altitude;
	packet.color_r = color_r;
	packet.color_g = color_g;
	packet.color_b = color_b;
	packet.color_c = color_c;

    esp_err_t result = esp_now_send(MAC_RECEIVER_1, (uint8_t*)&packet, sizeof(Packet));

    // Sending packet via ESP-NOW
    if (result == ESP_OK) {
        Serial.println("Packet sent successfully");
    }
	else {
        Serial.println("Error sending packet");
    }
}

// Register Peer Function
void RegisterPeer() {
	// Create peer info structure
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, MAC_RECEIVER_1, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

	// Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
    }
	else {
        Serial.println("Registered peer");
    }
}

// Initialize ESP-NOW Function
void InitEspNow()
{
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
	// Register send callback function to get status of transmitted packet
	esp_now_register_send_cb(OnDataSent);
	// Register the receiver as a peer
	RegisterPeer();
}

// Water Health
void WaterHealth() {
	// Turn on Motor for 1 seconds
	Serial.println("Motor Active.");
	delay(1*1000);
	// Turn on Servo for 1 second
	Serial.println("Servo Rotated.");
	delay(1*1000);
	/* Use RGB Sensor*/
	//wait for color data to be ready
	while(!apds.colorDataReady()){
		delay(5);
	}
	//get the data and print the different channels
	apds.getColorData(&color_r, &color_g, &color_b, &color_c);
	Serial.println("Color Detected.");
	
	// Save to Onboard Memory
	Serial.println("Data saved.");
}

// WaterFilter
void WaterFilter() {
	// Turn on Motor for 1 second
	Serial.println("Motor Active.");
	delay(1*1000);
}

// Setup Function
void setup() {
    Serial.begin(115200);
    delay(2000);
    WiFi.mode(WIFI_STA);

    // ESP now initialization
    InitEspNow();

    // Initialize UART with explicit pins (ESP32 requirement)
    GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

	// Initialize GPS
    GPS.begin(9600);

	// Configure GPS output
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    GPS.sendCommand(PGCMD_ANTENNA);

    delay(1000);

	// Configure Color Sensor
	if(!apds.begin()){
		Serial.println("failed to initialize device! Please check your wiring.");
	}
	else {
		Serial.println("Device initialized!");
	}
	//enable color sensing mode
	apds.enableColor(true);

    timer = millis();
}

// Loop Function
void loop() {
	// Read from GPS module
    char c = GPS.read();

    if (GPSECHO && c) {
        Serial.print(c);
    }

    if (GPS.newNMEAreceived()) {
        if (!GPS.parse(GPS.lastNMEA())) {
            return; // failed parse, wait for next sentence
        }
    }

    // Attempt to send packet every SLEEP_SECONDS
    if (millis() - timer > SLEEP_SECONDS * 1000) {
        timer = millis();

		// Send packet when there is a valid GPS fix
        if (GPS.fix) {
            Serial.println("GPS Fix Valid. Sending Packet");
            SendPacket();
        }
		else {
            Serial.println("Pending GPS fix...");
        }

        // Water Health
		WaterHealth();

        // Water Filter
		WaterFilter();
    }
}