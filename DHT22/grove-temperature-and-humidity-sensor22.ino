#include <WioLTEforArduino.h>

#define SENSOR_PIN    (WIOLTE_D38)

void setup()
{
  TemperatureAndHumidityBegin(SENSOR_PIN);
}

void loop()
{
  float temp;
  float humi;

  if (!TemperatureAndHumidityRead(&temp, &humi)) {
    SerialUSB.println("ERROR!");
    goto err;
  }

  SerialUSB.print("Current humidity = ");
  SerialUSB.print(humi);
  SerialUSB.print("%  ");
  SerialUSB.print("temperature = ");
  SerialUSB.print(temp);
  SerialUSB.println("C");

err:
  delay(2000);
}

////////////////////////////////////////////////////////////////////////////////////////
//

int TemperatureAndHumidityPin;

void TemperatureAndHumidityBegin(int pin)
{
  TemperatureAndHumidityPin = pin;
  DHT22Init(TemperatureAndHumidityPin);
}

bool TemperatureAndHumidityRead(float* temperature, float* humidity)
{
  byte data[5];
  float f = NAN;

  DHT22Start(TemperatureAndHumidityPin);
  for (int i = 0; i < 5; i++) data[i] = DHT22ReadByte(TemperatureAndHumidityPin);

  DHT22Finish(TemperatureAndHumidityPin);

  if (!DHT22Check(data, sizeof (data))) return false;
  f = data[0];
  f *= 256;
  f += data[1];
  f *= 0.1;
  *humidity = f;

  f = data[2] & 0x7F;
  f *= 256;
  f += data[3];
  f *= 0.1;
  if (data[2] & 0x80) {
    f *= -1;
  }
  *temperature = f;

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////
//

void DHT22Init(int pin)
{
  digitalWrite(pin, HIGH);
  pinMode(pin, OUTPUT);
}

void DHT22Start(int pin)
{
  // Host the start of signal
  digitalWrite(pin, LOW);
  delay(18);

  // Pulled up to wait for
  pinMode(pin, INPUT);
  while (!digitalRead(pin)) ;

  // Response signal
  while (digitalRead(pin)) ;

  // Pulled ready to output
  while (!digitalRead(pin)) ;
}

byte DHT22ReadByte(int pin)
{
  byte data = 0;

  for (int i = 0; i < 8; i++) {
    while (digitalRead(pin)) ;

    while (!digitalRead(pin)) ;
    unsigned long start = micros();

    while (digitalRead(pin)) ;
    unsigned long finish = micros();

    if ((unsigned long)(finish - start) > 50) data |= 1 << (7 - i);
  }

  return data;
}

void DHT22Finish(int pin)
{
  // Releases the bus
  while (!digitalRead(pin)) ;
  digitalWrite(pin, HIGH);
  pinMode(pin, OUTPUT);
}

bool DHT22Check(const byte* data, int dataSize)
{
  if (dataSize != 5) return false;

  byte sum = 0;
  for (int i = 0; i < dataSize - 1; i++) {
    sum += data[i];
  }

  return data[dataSize - 1] == sum;
}

////////////////////////////////////////////////////////////////////////////////////////

