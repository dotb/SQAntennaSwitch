// Useful references
// The Firmata protocol
// https://github.com/firmata/protocol/blob/master/protocol.md


#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

#include <ETH.h>

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
      ledOn();
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      ethIsConnected = false;
      ledOff();
      break;
    case SYSTEM_EVENT_ETH_STOP:
      ethIsConnected = false;
      ledOff();
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
  
  // pinMode(13, OUTPUT)
  Serial.write(0xF4);
  Serial.write(13);
  Serial.write(1);
}

void loop () {
  handleSocketData();
  delay(1000);
}

void handleSocketData() {
  char c = ' ';
  String currentLine = "";
  
  if (ethIsConnected) {
    if (!ethClient || !ethClient.connected()) {
      ethClient = ethServer.available();
    }
    
    if (ethClient && ethClient.connected() && ethClient.available()) {
      while (ethClient.available()) {
        char c = ethClient.read();
        if (c == '1') {
          ledOn();
          ethClient.println("LED on");            
        } else if (c != '\n' && c != '\r') {
          ledOff();
          ethClient.println("LED off");
        }
        ethClient.println("Thanks, done.");
      } // While available
    } else if (ethClient && !ethClient.connected()) {
      ethClient.stop();
    }
  }
}

void ledOn() {
  // digitalWrite(13, HIGH)
  Serial.write(0xF5);
  Serial.write(13);
  Serial.write(1);
}

void ledOff() {
  // digitalWrite(13, LOW)
  Serial.write(0xF5);
  Serial.write(13);
  Serial.write(0);
}
