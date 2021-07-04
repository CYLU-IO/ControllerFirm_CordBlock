void i2cInit() {
  Wire.begin();
}

void eeprom_write (byte val, int address, int ADDR) 
{
  Wire.beginTransmission(ADDR);
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xff)); // LSB

  Wire.write(val);
  Wire.endTransmission();
}

int eeprom_read(int address, int ADDR) 
{
  byte rData = 0xff;
 
  Wire.beginTransmission(ADDR);
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xff)); // LSB
  Wire.endTransmission();  

  Wire.requestFrom(ADDR, 1);  

  rData =  Wire.read();

  return rData;
}
