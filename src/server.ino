// Version 6.0

#include <esp_now.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Wifi
const char* ssid = "SSID_Name";
const char* password = "SSID_Password";
// Time Between HTTP Post Requests
#define SLEEP_MINUTES 30

// Domain Name
const char* serverName = "http://MicroMolaTemp";

// Sender and Receiver MAC addresses
const uint8_t MAC_SENDER_1[]   = { 0x34, 0xB7, 0xDA, 0xF6, 0x3C, 0x34 };
const uint8_t MAC_RECEIVER_1[] = { 0x34, 0xB7, 0xDA, 0xF6, 0x39, 0x74 };

// Encryption Keys
static const char* PMK_KEY_STR = "3mIcRoMoLa7xQ2pZ";
static const char* LMK_KEY_STR = "9kR4mIcRoMoLaX5w";

// Packet Structure
typedef struct Packet {
    uint32_t id;
    int16_t alert;
    float latitude;
    float longitude;
    float altitude;
	// output from color sensor
    uint16_t color_r, color_g, color_b, color_c;
} Packet;

unsigned long timer = 0;
// Json to send
char httpRequestData[1000] = "{}";

// Callback function for receiving ESP-NOW packets
void OnPacketReceived(const esp_now_recv_info_t* recv_info, const uint8_t* data, int len) {
	// Mac Address
    const uint8_t* mac = recv_info->src_addr;
    Serial.printf("Packet received from: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    // RSSI in dBm
	int8_t rssi = recv_info->rx_ctrl->rssi;
	Serial.printf("RSSI: %d dBm\n", rssi);
    Serial.printf("Bytes received: %d\n", len);

	// Make sure that the packet struct was received
    if (len != sizeof(Packet)) {
        Serial.println("Received data size does not match Packet struct!");
        return;
    }

    Packet* packet = (Packet*)data;

	// Normalize RGBC to RGB
	if (packet->color_c == 0) {
        packet->color_r = 0;
		packet->color_g = 0;
		packet->color_b = 0;
    }
	else {
		packet->color_r = (uint16_t)((packet->color_r * 255) / packet->color_c);
		packet->color_g = (uint16_t)((packet->color_g * 255) / packet->color_c);
		packet->color_b = (uint16_t)((packet->color_b * 255) / packet->color_c);
	}

	// Print received packet information
	Serial.println("");
    Serial.printf("Packet ID: %lu\n", packet->id);
    Serial.printf("Alert: %d\n", packet->alert);
    Serial.printf("Latitude: %.6f\n", packet->latitude);
    Serial.printf("Longitude: %.6f\n", packet->longitude);
    Serial.printf("Altitude: %.2f\n", packet->altitude);
	Serial.printf("Color R: %d\n", packet->color_r);
    Serial.printf("Color G: %d\n", packet->color_g);
    Serial.printf("Color B: %d\n", packet->color_b);
    Serial.printf("Color C: %d\n", packet->color_c);
	Serial.println("");

	// Build JSON string
	sprintf(httpRequestData,"{\"id\":%lu,\"alert\":%d,\"lat\":%f,\"long\":%f,\"alt\":%f,\"r\":%d,\"g\":%d,\"b\":%d}", packet->id, packet->alert, packet->latitude, packet->longitude, packet->altitude, packet->color_r, packet->color_g, packet->color_b);
}

// Initialize ESP-NOW
void InitEspNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
	// Set the PMK key
    esp_now_set_pmk((uint8_t *)PMK_KEY_STR);

    // Register receive callback
    esp_now_register_recv_cb(OnPacketReceived);

    // Create peer info structure
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, MAC_SENDER_1, 6);
    peerInfo.channel = 0;
    for (uint8_t i = 0; i < 16; i++) {
		peerInfo.lmk[i] = LMK_KEY_STR[i];
	}
	// Set encryption to true
    peerInfo.encrypt = true;

	// Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
    } else {
        Serial.println("Registered peer");
    }
}

void setup() {
    Serial.begin(115200);
	delay(1000);
	WiFi.mode(WIFI_STA);

	// Connect to Wifi
	delay(1000);
	WiFi.begin(ssid, password);
	Serial.println("Connecting to Wifi");
	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.printf("Successfully connected to %s", ssid);

	// Initialize ESP now
    delay(1000);
    InitEspNow();
    Serial.println("Receiver primed to receive packets...");

	timer = millis();
}

void loop() {
	// Send a HTTP POST request every SLEEP_MINUTES
	if (millis() - timer > SLEEP_MINUTES * 60 * 1000) {
		timer = millis();

		//Check WiFi connection status
		if(WiFi.status()== WL_CONNECTED){
			WiFiClient client;
			HTTPClient http;

			// Your Domain name with URL path or IP address with path
			http.begin(client, serverName);

			// If you need Node-RED/server authentication, insert user and password below
			//http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

			// Specify content-type header
			http.addHeader("Content-Type", "application/json");      
			// Send HTTP POST request
			int httpResponseCode = http.POST(httpRequestData);

			Serial.print("HTTP Response code: ");
			Serial.println(httpResponseCode);
				
			// Free resources
			http.end();
		}
		else {
			Serial.println("WiFi Disconnected");
		}
	}
}