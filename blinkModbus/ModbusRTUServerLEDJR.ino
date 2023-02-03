/*
  Modbus RTU Server LED

  This sketch creates a Modbus RTU Server with a simulated coil.
  The value of the simulated coil is set on the LED

  Circuit:
   - MKR board
   - MKR 485 shield
     - ISO GND connected to GND of the Modbus RTU server
     - Y connected to A/Y of the Modbus RTU client
     - Z connected to B/Z of the Modbus RTU client
     - Jumper positions
       - FULL set to OFF
       - Z \/\/ Y set to OFF

  created 16 July 2018
  by Sandeep Mistry
*/

#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

const int ledPinRed = 10;
const int ledPinYellow = 9;
const int ledPinGreen = 8;

void setup() {
  Serial.begin(9600);

  Serial.println("Modbus RTU Server LED");
  

  // start the Modbus RTU server, with (slave) id 1
  if (!ModbusRTUServer.begin(1, 9600)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
  }

  // configure the LED
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  digitalWrite(ledPinRed, LOW);
  digitalWrite(ledPinYellow, LOW);
  digitalWrite(ledPinGreen, LOW);

  delay(4000);

  // configure a single coil at address 0x00
  ModbusRTUServer.configureCoils(0x00, 3);
}

void loop() {
  // poll for Modbus RTU requests
  ModbusRTUServer.poll();

  // read the current value of the coil
  int coil1 = ModbusRTUServer.coilRead(0);
  int coil2 = ModbusRTUServer.coilRead(1);
  int coil3 = ModbusRTUServer.coilRead(2);
  
  if (coil1) {
    // coil value set, turn LED on
    digitalWrite(ledPinGreen, HIGH);
  } else {
    // coil value clear, turn LED off
    digitalWrite(ledPinGreen, LOW);
  }
  if (coil2) {
    // coil value set, turn LED on
    digitalWrite(ledPinYellow, HIGH);
  } else {
    // coil value clear, turn LED off
    digitalWrite(ledPinYellow, LOW);
  }
  
  if (coil3) {
    // coil value set, turn LED on
    digitalWrite(ledPinRed, HIGH);
  } else {
    // coil value clear, turn LED off
    digitalWrite(ledPinRed, LOW);
  }
}
