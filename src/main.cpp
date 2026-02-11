/*
This main file is used to select the appropriate build for the ESP32.
This way there is no need to have separate files for the client and server.
If neither is defined, the default is to run the client.
*/
#include <Arduino.h>
#include "./client/client.h"
#include "./server/server.h"

// Comment & uncomment the appropriate line below to switch between client and server.
#define MICROMOLA_CLIENT
// #define MICROMOLA_SERVER


void setup() {
#ifdef MICROMOLA_SERVER
  // Server setup
  serverSetup();
#else
  // Client setup
  clientSetup();
#endif
}

void loop() {
#ifdef MICROMOLA_SERVER
  // Server main loop
  serverLoop();
#else
  // Client main loop
  clientLoop();
#endif
}

