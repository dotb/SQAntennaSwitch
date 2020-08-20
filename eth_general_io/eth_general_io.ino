/*
 * This sketch parses serial commands (instructons)
 * and executes IO operations based on each issued
 * command and associated parameters. Similar to the Firmata
 * protocol, this sketch allows an arduino microcontroller to be
 * operated by a host arduino or computer, using generic instructions
 * that don't require the sketch to be updated.
 * 
 * The protocol is 8 bytes long, consisting of 4 integers.
 * <-- deviceID | CMD | PIN/ID | VALUE <--
 * Int 1 - deviceID = the device addressed. This identifier allows multiple devices to share a serial line and be addressed individualy.
 * Int 2 - CMD = The command or instruction
 * Int 3 - PIN/ID = the IO pin or device ID on which to execute the command
 * Int 4 - VALUE = the value to send to the pin. Not all instructions require this parameter and anything can be sent if it's not required.
 * 
 * Example:
 * 1 1 13 1 = on device 1, set the pin mode of pin 13 to an output
 * 1 2 13 1 = on device 1, set pin 13 to HIGH
 * 1 1 13 0 = on device 1, set the pin mode of pin 13 to an input
 * 1 4 13 0 = on device 1, read the analogue value of pin 13. The last parameter can be sent as any Int value. The result is written back to the serial line as a String.
 * 1 1 1 0  = on device 1, turn debug on
 */

#include <UIPEthernet.h>
#include <Servo.h>

/* 
 *  Change this device ID if you're connecting
 *  multiple devices to the same serial line.
 */
#define THIS_DEVICE_ID 1

// Length of the command buffer.
#define LENGTH_COMMAND 4

#define SERVOS_MAX 6

// Ethernet
#define LISTENPORT 1000
#define MACADDRESS 0x00,0x01,0x02,0x03,0x04,0x05

// Commands
#define DEBUG_TOGGLE 1
#define PIN_MODE 2
#define DIGITAL_WRITE 3
#define ANALOG_WRITE 4
#define DIGITAL_READ 5
#define ANALOG_READ 6
#define SERVO_ADD 7
#define SERVO_POSITION 8

EthernetServer ethServer = EthernetServer(LISTENPORT);
EthernetClient ethClient;
Servo servos[SERVOS_MAX]; // Allow up to 6 servos (6 PWM channels).
bool debug = false;
int command[LENGTH_COMMAND];
int commandPos = 0;
int nextServoPos = 0;


void setup() {
  uint8_t mac[6] = {MACADDRESS};
  Serial.begin(57600);
  Ethernet.begin(mac);
  ethServer.begin();
  reset();
}

/*
 * Continuously monitor the serial line and
 * collect input into a command buffer.
 * Execute the command when the buffer 
 * is fulled (LENGTH_COMMAND is reached).
 */
void loop() {  
  checkForAnEthernetClient();
  handleDataFromEthSocket();
  cleanupEthernetClient();
}

/*
 * Read one character (two bytes) into
 * the command buffer. Skip end of line
 * characters if the are received.
 */
void populateCommandBuffer() {
  char c = ethClient.peek();
  if ('\n' == c || '\r' == c) {
    // Throw away end-of-line characters
    ethClient.read();
  } else {
    // Consume the value into the command buffer
    addInputToBuffer();
  }
}

/*
 * Add the integer value of the input,
 * two bytes, to the command buffer.
 */
void addInputToBuffer() {
  int input = ethClient.parseInt();
  command[commandPos] = input;
  commandPos++;
  debugLog(" |");
  debugLog(String(input));
  debugLog("|");
}

/*
 * Execute the command in the command 
 * buffer and then reset to prepare for the 
 * next command.
 */
void executeCommandAndReset() {
  debugLog(" ->");
  debugLog("RUN");
  String result = executeCommand();
  reset();
  ethClient.println(result);
}

void debugLog(String str) {
  if (debug) {
    ethClient.print(str);
  }
}

void debugLogLn(String str) {
  if (debug) {
    ethClient.println(str);
  }
}

/*
 * Execute the command in the command buffer
 * if the nominated device ID belongs to this device.
 */
String executeCommand() {
  if (command[0] == THIS_DEVICE_ID) {
    switch (command[1]) {
      case DEBUG_TOGGLE:
        return toggleDebug();
      case PIN_MODE:
        return handlePinMode();
      case DIGITAL_WRITE:
        return handleDigitalWrite();
      case ANALOG_WRITE:
        return handleAnalogWrite();
      case DIGITAL_READ:
        return handleDigitalRead();
      case ANALOG_READ:
        return handleAnalogRead();
      case SERVO_ADD:
        return addServoToPin();
      case SERVO_POSITION:
        return setServoPosition();
      default:
        return "?-CMD"; // Unknown command
    } 
  } else {
    return "?-ID"; // Instruction is not for this device
  }
}

/*
 * Reset the system. This is called after a command is
 * executed so that the command buffer and pointers
 * can be cleared and reset.
 */
void reset() {
  commandPos = 0;
  for (int i = 0; i < LENGTH_COMMAND; i++) {
    command[i] = 0;
  }
}

/*
 * Command handlers. Each method handles one instruction
 * using the input parameters found in the command buffer.
 */
String toggleDebug() {
  int enabled = command[2];
  debug = enabled;
  return String(enabled);  
}

String handlePinMode() {
  int pin = command[2];
  int mode = command[3];
  pinMode(pin, mode);
  return "OK";
}

String handleDigitalWrite() {
  int pin = command[2];
  int value = command[3];
  digitalWrite(pin, value);
  return "OK";
}

String handleAnalogWrite() {
  int pin = command[2];
  int value = command[3];
  analogWrite(pin, value);
  return "OK";
}

String handleDigitalRead() {
  int pin = command[2];
  int value = digitalRead(pin);
  return String(value);
}

String handleAnalogRead() {
  int pin = command[2];
  int value = analogRead(pin);
  return String(value);
}

String addServoToPin() {
  int pin = command[2];
  int servoID = nextServoPos;
  servos[nextServoPos].attach(pin);
  nextServoPos++;
  if (nextServoPos >= SERVOS_MAX) {
    nextServoPos = 0;
  }
  return String(servoID);
}

String setServoPosition() {
  int servoID = command[2];
  int position = command[3];
  servos[servoID].write(position);
  return String(position);
}

/*
 * Methods that handle the enternet connection
 */
 /*
 * Handle data from the ethernet socket.
 * Data received on the ethernet socket is sent to
 * the serial line.
 */
void handleDataFromEthSocket() {
  if (ethClient && ethClient.connected()) {
    while (ethClient.available() && commandPos < 6) {
      populateCommandBuffer();
    }

    if (commandPos >= LENGTH_COMMAND) {
      executeCommandAndReset();
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
