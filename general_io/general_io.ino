
// Protocol is 8 bytes long (consisting of 4 integers
// <-- deviceID | CMD | PIN | VALUE 

#define THIS_DEVICE_ID 1
#define LENGTH_COMMAND 4

// Commands
#define PIN_MODE 1
#define DIGITAL_WRITE 2
#define ANALOG_WRITE 3
#define DIGITAL_READ 4
#define ANALOG_READ 5

int command[LENGTH_COMMAND];
int commandPos = 0;

void setup() {
  Serial.begin(57600);
  reset();
}

void loop() {
  while (Serial.available() && commandPos < 6) {
    char c = Serial.peek();
    if ('\n' == c || '\r' == c) {
      Serial.read(); // Throw away end-of-line characters
    } else {
      // Consume the value into our command
      int input = Serial.parseInt();
      command[commandPos] = input;
      commandPos++;
      Serial.print(" |");
      Serial.print(input);
      Serial.print("|");
    }
  }

  if (commandPos >= LENGTH_COMMAND) {
    Serial.println(" ->");
    Serial.println("RUN");
    String result = executeCommand();
    reset();
    Serial.println(result);
  }
  delay(100);
}

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
      default:
        return "?-CMD";
    } 
  } else {
    return "?-ID";
  }
}

void reset() {
  commandPos = 0;
  for (int i = 0; i < LENGTH_COMMAND; i++) {
    command[i] = 0;
  }
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
