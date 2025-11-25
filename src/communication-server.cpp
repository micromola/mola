#include <communication.h>

// Global Variable for testing
Packet packet;

// Function Declaration
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len);

// Setup Function Definition
void setup() {
    Serial.begin(115200);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    Serial.println("ESP-NOW Server (Receiver) Started");

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register receive callback function for receiving messages
    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    // Nothing for now.
}

// Callback Function Definition
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
    // Check the packet size to the struct size
    if (len != sizeof(Packet)) {
        Serial.printf("Received packet size is not the same as the struct size.\n");
        return;
    }
    
    // Copy and print packet data
    memcpy(&packet, incomingData, sizeof(packet));
    packet.message[MAX_MESSAGE - 1] = '\0';
    Serial.printf("Packet %d has been received\n");
    Serial.printf("Latitude: %f\n", packet.latitude);
    Serial.printf("Longitude: %f\n", packet.longitude);
    Serial.printf("Msg: %s\n", packet.message);
}