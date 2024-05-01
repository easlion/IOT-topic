#include <SoftwareSerial.h>

byte commands[9] = {0xff, 0x01, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte response[9];

// SoftwareSerial
// Arduino Uno has built-in support for serial communication (pin0 and 1)
// And usually happens by hardware (built into the chip) called UART
// It allows hardware do the serial communication while working on other task.

// SoftwareSerial library allows any digital pin to do the UART communication, as long as device supports UART procotol as well. 
// rx: receive
// tx: transmit
SoftwareSerial co2Serial(18, 17); // RX, TX

struct Co2Result
{
  byte startByte;
  byte command;
  byte high;
  byte low;
  unsigned short ppm;
};

// in MH-Z14A, there are multiple commands defined as follows:
// you can find command code in datasheet.
enum COMMANDS
{
  GET_GAS = 0x86,
  CALIBRATE_ZERO = 0x87,
  AUTO_CALIBRATE = 0x79
};

void writeCo2Result(Co2Result *result)
{
  Serial.write(result->high);
  Serial.write(result->low);
}

void sendCommand(COMMANDS command)
{
  memset(response, 0, sizeof(response));
  commands[2] = command;
  commands[8] = getCheckSum();
  co2Serial.write(commands, 9); // send command
  co2Serial.readBytes(response, 9);
}

void getGas()
{
  commands[3] = 0x0;
  sendCommand(GET_GAS);
}

void autoCalibrate(bool start)
{
  commands[3] = start ? 0xA0 : 0x00;
  sendCommand(GET_GAS);
}

void calibrateZeroPoint()
{
  commands[3] = 0x0;
  sendCommand(CALIBRATE_ZERO);
}

char getCheckSum()
{
  char checksum;
  for (auto i = 0; i < 8; i++)
  {
    checksum += commands[i];
  }
  checksum = 0xff - checksum;
  checksum += 1;
  return checksum;
}

void setup()
{
  co2Serial.begin(9600);
  Serial.begin(9600);
  
  // When doing zero point calibration
  // put MH-Z14A sensor in outdoor that CO2 level is 400ppm.
  
  // Comment me out when configured
  //------------------------
  // wait 20 minutes then reset
  //  for (auto i = 0; i < 1200; i++)
  //  {
  //    Serial.print(i + 1);
  //    Serial.print("/");
  //    Serial.print(1200);
  //    Serial.println("");
  //    delay(1000);
  //  }
  //  Serial.println("Sending calibrate zero point");
  //  calibrateZeroPoint();
  //  Serial.println("Done");
  //------------------------
  autoCalibrate(true);
}

void loop()
{
  sendCommand(GET_GAS);
  Co2Result result;
  result.startByte = response[0];
  result.command = response[1];
  result.high = response[2];
  result.low = response[3];
  result.ppm = (result.high << 8) + result.low;
  
  writeCo2Result(&result);
  delay(3000);
}
