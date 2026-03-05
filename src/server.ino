//  Version 3.0

#include <esp_now.h>
#include <WiFi.h>

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
}

// Initialize ESP-NOW
void InitEspNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register receive callback
    esp_now_register_recv_cb(OnPacketReceived);

    // Create peer info structure
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, MAC_SENDER_1, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

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

    InitEspNow();

    Serial.println("Receiver primed to receive packets...");
}

void loop() {
    // Nothing for now
}