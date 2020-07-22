#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

#include <ETH.h>

#define SERVO_PIN 13

static bool ethIsConnected = false;
WiFiServer ethServer(80);
WiFiClient ethClient;

void handleEthEvent(WiFiEvent_t event)
{
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

void loop () {
  if (ethIsConnected) {
    handleCMDFromSocket();
    handleResponseToSocket();
  }
}

void handleCMDFromSocket() {
  char c = ' ';
  String currentLine = "";
  
  if (!ethClient || !ethClient.connected()) {
    ethClient = ethServer.available();
  }
  
  if (ethClient && ethClient.connected() && ethClient.available()) {
    while (ethClient.available()) {
  
      char c = ethClient.peek();
      if ('\n' == c || '\r' == c) {
        ethClient.read(); // Throw away end-of-line characters
      } else {
        // Consume the value and proxy it to the serial stream
        int input = ethClient.parseInt();
        Serial.println(input);
      }

    } // While available
  } else if (ethClient && !ethClient.connected()) {
    ethClient.stop();
  }
  
}

void handleResponseToSocket() {
  if (ethClient && ethClient.connected() && Serial.available()) {
    while (Serial.available()) {
      char c = Serial.read();
      ethClient.print(c);
    } // While available
  }
}
