/*
  Lane Change & Overtake Assistance System
  LoRa-based V2V Communication
  Vehicle Transmitter

  Platform: ESP32
  LoRa module: SX1278/RFM95-compatible module
  GPS: NEO-6M (TinyGPS++ library)
  Ultrasonic: HC-SR04
  LoRa library: LoRa by Sandeep Mistry
  GPS library: TinyGPSPlus

  IMPORTANT:
  Pin numbers below are example ESP32 connections. Change them to match
  your actual hardware before uploading to a real board.
*/

#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

// ---------- LoRa SX1278 pins ----------
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

// ---------- ESP32 VSPI pins ----------
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23

// ---------- GPS ----------
#define GPS_RX 16
#define GPS_TX 17
#define GPS_BAUD 9600

// ---------- Ultrasonic ----------
#define TRIG_PIN 32
#define ECHO_PIN 33

// ---------- Vehicle ----------
const char VEHICLE_ID[] = "V1";

// Set the LoRa frequency according to the module/regulations used in
// your project. 433E6 is commonly used with SX1278 433 MHz modules.
#define LORA_FREQUENCY 433E6

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 1000; // send every 1 second

float readDistanceCM()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
    return -1.0;

  return (duration * 0.0343) / 2.0;
}

void readGPS()
{
  while (GPSSerial.available())
  {
    gps.encode(GPSSerial.read());
  }
}

float getSpeedKmph()
{
  if (gps.speed.isValid())
    return gps.speed.kmph();

  return 0.0;
}

double getLatitude()
{
  if (gps.location.isValid())
    return gps.location.lat();

  return 0.0;
}

double getLongitude()
{
  if (gps.location.isValid())
    return gps.location.lng();

  return 0.0;
}

float getDirection()
{
  if (gps.course.isValid())
    return gps.course.deg();

  return 0.0;
}

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQUENCY))
  {
    Serial.println("LoRa initialization failed!");
    while (true)
      delay(1000);
  }

  Serial.println("Vehicle Transmitter Started");
}

void loop()
{
  readGPS();

  if (millis() - lastSendTime >= SEND_INTERVAL)
  {
    lastSendTime = millis();

    float speed = getSpeedKmph();
    double latitude = getLatitude();
    double longitude = getLongitude();
    float direction = getDirection();
    float distance = readDistanceCM();

    // Report format:
    // <ID, Speed, Latitude, Longitude, Direction>
    //
    // Distance is also included so that the receiving vehicle can
    // directly use the measured nearby-vehicle distance.
    String packet =
      String(VEHICLE_ID) + "," +
      String(speed, 2) + "," +
      String(latitude, 6) + "," +
      String(longitude, 6) + "," +
      String(direction, 2) + "," +
      String(distance, 2);

    LoRa.beginPacket();
    LoRa.print(packet);
    LoRa.endPacket();

    Serial.print("Transmitted: ");
    Serial.println(packet);
  }
}
