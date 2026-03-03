#include <communication.h>

// MAC Address (See README for for information)
uint8_t serverMAC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Global Variable for testing
Packet packet;

// Function Declaration
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

// Setup Function Definition
void setup() {
    // Initialize the serial communication
    Serial.begin(115200);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    Serial.println("ESP-NOW Client Started");

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register send callback function to get status of transmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register the server as a peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, serverMAC, 6); // Copy 6 bytes into peer info structure.
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    Serial.println("Ready to send messages...");
}

// Loop Function Definition
void loop() {
    // Populate packet with sample data
    packet.id += 1;
    packet.latitude += 0.01;
    packet.longitude += 0.01;
    char *message = "Hello World";
    strncpy(packet.message, message, MAX_MESSAGE - 1);
    packet.message[MAX_MESSAGE-1] = '\0';

    // Send packet via ESP-NOW
    esp_err_t result = esp_now_send(serverMAC, (uint8_t *)&packet, sizeof(packet));

    if (result == ESP_OK) {
        Serial.println("Packet has been sent");
    } else {
        Serial.println("Error sending packet");
    }

    // Send every 10 seconds
    delay(10000);
}

// Callback Function Definition
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}