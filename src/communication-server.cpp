#include <esp_now.h>
#include <WiFi.h>

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
    // Store and print test message
    const int maxLength = 100;
    char message[maxLength];
    if (len >= maxLength){
        len = maxLength - 1;
    }
    memcpy(message, incomingData, len);
    message[len] = '\0'; // set string null character in case
    Serial.print("Received message: ");
    Serial.println(message);
}