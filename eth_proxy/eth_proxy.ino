/*
 * This sketch implements an ethernet <-> serial proxy.
 * It opens a TCP socket and brokers String data between
 * the client socket and the serial line. This sketch was
 * written to be used with the general_io sketch, so that
 * an enternet enabled arduino Uno can drive a larger arduino
 * device that allows control of a large number of IO pins. However,
 * it can be used for other proxy applications.
 * 
 * This sketch was origionally written for the OLIMEX ESP32-POE device.
 * https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/open-source-hardware
 */

#include <ETH.h>

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

WiFiServer ethServer(80);
WiFiClient ethClient;
static bool ethIsConnected = false;

/*
 * Handle ethernet events, and manage the connected state.
 */
void handleEthEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      ETH.setHostname("roof-io");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      ethIsConnected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      ethIsConnected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      ethIsConnected = false;
      break;
    default:
      break;
  }
}

void setup() {
  WiFi.onEvent(handleEthEvent);
  ETH.begin();
  ethServer.begin();
  Serial.begin(57600);
}

/*
 * Handle data from the ethernet
 * socket and serial line.
 */
void loop () {
  if (ethIsConnected) {
    checkForAnEthernetClient();
    handleDataFromEthSocket();
    handleDataFromSerialLine();
    cleanupEthernetClient();
  }
}

/*
 * Handle data from the ethernet socket.
 * Data received on the ethernet socket is sent to
 * the serial line.
 */
void handleDataFromEthSocket() {
  if (ethIsConnected && ethClient && ethClient.connected()) {
    while (ethClient.available()) {
      char input = ethClient.read();
      Serial.write(input);
    }
  }
}

/*
 * Handle data from the serial line.
 * Data received on the serial line is sent to
 * the ethernet socket, if a client is connected.
 */
void handleDataFromSerialLine() {
  while (Serial.available()) {
    char c = Serial.read();
    // Send the character to the ethernet client or throw it away if no client is connected.
    if (ethIsConnected && ethClient && ethClient.connected()) {
      ethClient.print(c);
    }
  }
}

/*
 * Check for an ethernet client and set
 * a pointer if one is available.
 */
void checkForAnEthernetClient() {
  if (!ethClient || !ethClient.connected()) {
    ethClient = ethServer.available();
  }
}

/*
 * Clean up the ethernet client poiner
 * if the client has disconnected.
 */
void cleanupEthernetClient() {
  if (ethClient && !ethClient.connected()) {
    ethClient.stop();
  }
}
