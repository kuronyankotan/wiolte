#include <WioLTEforArduino.h>
#include <stdio.h>

#define SENSOR_PIN    (WIOLTE_D38)
#define INTERVAL  (60000)
WioLTE Wio;

bool State_chg = false;
float State_temp = 0;

void setup()
{
  TemperatureAndHumidityBegin(SENSOR_PIN);
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(5000);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
}

void loop()
{
  float temp;
  float humi;
  char data[100];
  int connectId;

  if (!TemperatureAndHumidityRead(&temp, &humi)) {
    SerialUSB.println("ERROR!");
    goto err;
  }

  SerialUSB.print("Current humidity = ");
  SerialUSB.print(humi);
  SerialUSB.print("%  ");
  SerialUSB.print("Def_temperature = ");
  SerialUSB.print(State_temp);
  SerialUSB.println("C");
  SerialUSB.print("temperature = ");
  SerialUSB.print(temp);
  SerialUSB.println("C");

 //温度センサーに変化があればフラグをON
  State_chg = false;
  if (State_temp != temp) {
    State_chg = true;
    State_temp = temp;
    SerialUSB.println("DIFF!!");
  }
// Soracom beamで送信
  if (State_chg) {
    SerialUSB.println("### Open.");
    connectId = Wio.SocketOpen("beam.soracom.io", 23080, WIOLTE_UDP);
    if (connectId < 0) {
      SerialUSB.println("### ERROR! ###");
      goto err;
    }

    SerialUSB.println("### Send.");
    sprintf(data, "{\"humi\":%f,\"temp\":%f}", humi, temp);
    SerialUSB.print("Send:");
    SerialUSB.print(data);
    SerialUSB.println("");
    if (!Wio.SocketSend(connectId, data)) {
      SerialUSB.println("### ERROR! ###");
      !Wio.SocketClose(connectId);
      goto err;
    }

    SerialUSB.println("### Receive.");
    int length;
    do {
      length = Wio.SocketReceive(connectId, data, sizeof (data));
      if (length < 0) {
        SerialUSB.println("### ERROR! ###");
        !Wio.SocketClose(connectId);
        goto err;
      }
    } while (length == 0);
    SerialUSB.print("Receive:");
    SerialUSB.print(data);
    SerialUSB.println("");

    SerialUSB.println("### Close.");
    if (!Wio.SocketClose(connectId)) {
      SerialUSB.println("### ERROR! ###");
      goto err;
    }
  }

err:
  delay(INTERVAL);
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

