// Version 5.0

#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GPS.h>
#include <Adafruit_APDS9960.h>

// Time Between Packet Send
#define SLEEP_SECONDS 10

// Sender and Receiver MAC addresses
const uint8_t MAC_SENDER_1[]   = { 0x34, 0xB7, 0xDA, 0xF6, 0x3C, 0x34 };
const uint8_t MAC_RECEIVER_1[] = { 0x34, 0xB7, 0xDA, 0xF6, 0x39, 0x74 };

// Encryption Keys
static const char* PMK_KEY_STR = "3mIcRoMoLa7xQ2pZ";
static const char* LMK_KEY_STR = "9kR4mIcRoMoLaX5w";

// Component Pin Assignments
#define PIN_WHITE_LED 3
#define PIN_SERVO 4
#define PIN_PUMP 5
#define PIN_PROPELLER_A 6
#define PIN_PROPELLER_B 7

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
int16_t systemAlert = 0;
uint16_t color_r = 0, color_g = 0, color_b = 0, color_c = 0;

// ESP-NOW Callback Function
void OnDataSent(const esp_now_send_info_t * tx_info, esp_now_send_status_t status) {
    Serial.printf("Packet #%"PRIu32"4 Send Status: ", packetCounter);
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
	//Set the receiver device LMK key
    for (uint8_t i = 0; i < 16; i++) {
		peerInfo.lmk[i] = LMK_KEY_STR[i];
	}
    // Set encryption to true
	peerInfo.encrypt = true;

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
	// Set PMK key
    esp_now_set_pmk((uint8_t *)PMK_KEY_STR);
	// Register send callback function to get status of transmitted packet
	esp_now_register_send_cb(OnDataSent);
	// Register the receiver as a peer
	RegisterPeer();
}

// Water Health
void WaterHealth() {
	// Turn on Motor for 1 seconds
	Serial.println("Motor Active.");
	digitalWrite(PIN_PUMP, HIGH);
	delay(1*1000);
	digitalWrite(PIN_PUMP, LOW);
	Serial.println("Motor Deactivated.");
	// Turn on Servo for 1 second
	digitalWrite(PIN_SERVO, HIGH);
	delay(1*1000);
	digitalWrite(PIN_SERVO, LOW);
	Serial.println("Servo Rotated.");
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
	// Turn on Propeller for 1 second
	Serial.println("Propeller Active.");
	digitalWrite(PIN_PROPELLER_A, HIGH);
	digitalWrite(PIN_PROPELLER_B, HIGH);
	delay(1*1000);
	// I believe one of the pins is to control direction but for now I will turn both on/off as a placeholder.
	digitalWrite(PIN_PROPELLER_A, LOW);
	digitalWrite(PIN_PROPELLER_B, LOW);
	Serial.println("Propeller Deactivated.");
}

// Setup Function
void setup() {
    Serial.begin(115200);
    delay(1000);
    WiFi.mode(WIFI_STA);

	// Initialize Component Pins
	pinMode(PIN_WHITE_LED, OUTPUT);
	pinMode(PIN_SERVO, OUTPUT);
	pinMode(PIN_PUMP, OUTPUT);
	pinMode(PIN_PROPELLER_A, OUTPUT);
	pinMode(PIN_PROPELLER_B, OUTPUT);

	// Turn White LED On
	digitalWrite(PIN_WHITE_LED, HIGH);

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