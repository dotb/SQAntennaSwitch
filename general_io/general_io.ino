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
 */

#include <Servo.h>

/* 
 *  Change this device ID if you're connecting
 *  multiple devices to the same serial line.
 */
#define THIS_DEVICE_ID 1

// Length of the command buffer.
#define LENGTH_COMMAND 4

#define SERVOS_MAX 6

// Commands
#define PIN_MODE 1
#define DIGITAL_WRITE 2
#define ANALOG_WRITE 3
#define DIGITAL_READ 4
#define ANALOG_READ 5
#define SERVO_ADD 6
#define SERVO_POSITION 7

int command[LENGTH_COMMAND];
int commandPos = 0;
int nextServoPos = 0;
Servo servos[SERVOS_MAX]; // Allow up to 6 servos (6 PWM channels).

void setup() {
  Serial.begin(57600);
  reset();
}

/*
 * Continuously monitor the serial line and
 * collect input into a command buffer.
 * Execute the command when the buffer 
 * is fulled (LENGTH_COMMAND is reached).
 */
void loop() {
  while (Serial.available() && commandPos < 6) {
    populateCommandBuffer();
  }

  if (commandPos >= LENGTH_COMMAND) {
    executeCommandAndReset();
  }
}

/*
 * Read one character (two bytes) into
 * the command buffer. Skip end of line
 * characters if the are received.
 */
void populateCommandBuffer() {
  char c = Serial.peek();
  if ('\n' == c || '\r' == c) {
    // Throw away end-of-line characters
    Serial.read();
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
  int input = Serial.parseInt();
  command[commandPos] = input;
  commandPos++;
  Serial.print(" |");
  Serial.print(input);
  Serial.print("|");
}

/*
 * Execute the command in the command 
 * buffer and then reset to prepare for the 
 * next command.
 */
void executeCommandAndReset() {
  Serial.println(" ->");
  Serial.println("RUN");
  String result = executeCommand();
  reset();
  Serial.println(result);
}

/*
 * Execute the command in the command buffer
 * if the nominated device ID belongs to this device.
 */
String executeCommand() {
  if (command[0] == THIS_DEVICE_ID) {
    switch (command[1]) {
      case PIN_MODE:
        return handlePinMode();
        break;
      case DIGITAL_WRITE:
        return handleDigitalWrite();
        break;
      case ANALOG_WRITE:
        return handleAnalogWrite();
        break;
      case DIGITAL_READ:
        return handleDigitalRead();
        break;
      case ANALOG_READ:
        return handleAnalogRead();
        break;
      case SERVO_ADD:
        return addServoToPin();
        break;
      case SERVO_POSITION:
        return setServoPosition();
        break;
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
