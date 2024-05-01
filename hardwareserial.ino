HardwareSerial SerialPort(1);

void setup()  
{
  SerialPort.begin(9600, SERIAL_8N1, 47, 48); 
  Serial.begin(9600);
}

void loop()
{
  if(SerialPort.available()>0){        
    byte in = SerialPort.read();
    SerialPort.write(in);
    Serial.print(in);
  }
  }
