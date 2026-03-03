#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <esp_now.h>
#include <WiFi.h>
#define MAX_MESSAGE 100

typedef struct {
    int id;
    float latitude;
    float longitude;
    char message[MAX_MESSAGE];
    // Other data needed to be sent
} Packet;

#endif