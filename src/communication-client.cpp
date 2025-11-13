#include <esp_now.h>
#include <WiFi.h>

// MAC Address (See README for for information)
uint8_t receiverMAC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
    memcpy(peerInfo.peer_addr, receiverMAC, 6); // Copy 6 bytes into peer info structure.
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
    // Test message to send
    char *message = "Hello";

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)message, strlen(message) + 1);

    if (result == ESP_OK) {
        Serial.println("Message sent: Hello");
    } else {
        Serial.println("Error sending message");
    }

    // Send every 10 seconds
    delay(10000);
}

// Callback Function Definition
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}