/*
  Lane Change & Overtake Assistance System
  LoRa-based V2V Communication
  Vehicle Receiver / Warning Unit

  Platform: ESP32
  LoRa module: SX1278/RFM95-compatible module
  GPS: NEO-6M (TinyGPS++ library)
  Ultrasonic: HC-SR04
  Warning: LED + Buzzer
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

// ---------- Warning ----------
#define LED_PIN 2
#define BUZZER_PIN 25

// ---------- Vehicle ----------
const char LOCAL_VEHICLE_ID[] = "V2";

#define LORA_FREQUENCY 433E6

// Safety threshold from the project logic.
// Change this value to match your tested setup.
const float SAFE_DISTANCE_CM = 1000.0; // 10 m

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

float receivedSpeed = 0.0;
double receivedLatitude = 0.0;
double receivedLongitude = 0.0;
float receivedDirection = 0.0;
float receivedDistance = -1.0;

unsigned long lastPacketTime = 0;
const unsigned long PACKET_TIMEOUT = 5000;

float readLocalSpeed()
{
  if (gps.speed.isValid())
    return gps.speed.kmph();

  return 0.0;
}

float readLocalDistanceCM()
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

// Expected packet:
// ID,Speed,Latitude,Longitude,Direction,Distance

bool parsePacket(String packet)
{
  int index[5];
  int start = 0;

  for (int i = 0; i < 5; i++)
  {
    index[i] = packet.indexOf(',', start);

    if (index[i] == -1)
      return false;

    start = index[i] + 1;
  }

  String id = packet.substring(0, index[0]);
  String speedText = packet.substring(index[0] + 1, index[1]);
  String latText = packet.substring(index[1] + 1, index[2]);
  String lonText = packet.substring(index[2] + 1, index[3]);
  String directionText = packet.substring(index[3] + 1, index[4]);
  String distanceText = packet.substring(index[4] + 1);

  receivedSpeed = speedText.toFloat();
  receivedLatitude = latText.toDouble();
  receivedLongitude = lonText.toDouble();
  receivedDirection = directionText.toFloat();
  receivedDistance = distanceText.toFloat();

  Serial.println("----- Received Vehicle Data -----");
  Serial.print("Vehicle ID: ");
  Serial.println(id);
  Serial.print("Speed (km/h): ");
  Serial.println(receivedSpeed);
  Serial.print("Latitude: ");
  Serial.println(receivedLatitude, 6);
  Serial.print("Longitude: ");
  Serial.println(receivedLongitude, 6);
  Serial.print("Direction (deg): ");
  Serial.println(receivedDirection);
  Serial.print("Distance (cm): ");
  Serial.println(receivedDistance);

  return true;
}

void setWarning(bool warning)
{
  digitalWrite(LED_PIN, warning ? HIGH : LOW);

  if (warning)
  {
    tone(BUZZER_PIN, 2000);
  }
  else
  {
    noTone(BUZZER_PIN);
  }
}

void checkSafety()
{
  float localSpeed = readLocalSpeed();

  // Relative speed according to the report:
  // Vr = V2 - V1
  float relativeSpeed = receivedSpeed - localSpeed;

  // Use the received ultrasonic distance when valid.
  float distance = receivedDistance;

  // If no valid received distance is available, use local ultrasonic data.
  if (distance < 0)
    distance = readLocalDistanceCM();

  bool unsafe = false;

  if (distance >= 0 && distance < SAFE_DISTANCE_CM)
  {
    unsafe = true;
  }

  // If the other vehicle is approaching faster, also treat it as a
  // warning condition.
  if (relativeSpeed > 10.0 && distance >= 0 &&
      distance < (SAFE_DISTANCE_CM * 1.5))
  {
    unsafe = true;
  }

  if (unsafe)
  {
    Serial.println("WARNING: Unsafe condition detected!");
    setWarning(true);
  }
  else
  {
    Serial.println("Status: SAFE");
    setWarning(false);
  }

  Serial.print("Local Speed (km/h): ");
  Serial.println(localSpeed);
  Serial.print("Relative Speed (km/h): ");
  Serial.println(relativeSpeed);
}

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);

  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQUENCY))
  {
    Serial.println("LoRa initialization failed!");
    while (true)
      delay(1000);
  }

  Serial.println("Vehicle Receiver Started");
}

void loop()
{
  readGPS();

  int packetSize = LoRa.parsePacket();

  if (packetSize)
  {
    String receivedPacket = "";

    while (LoRa.available())
    {
      receivedPacket += (char)LoRa.read();
    }

    Serial.print("Raw packet: ");
    Serial.println(receivedPacket);

    if (parsePacket(receivedPacket))
    {
      lastPacketTime = millis();
      checkSafety();
    }
  }

  // If communication is lost, turn the warning off after timeout.
  if (millis() - lastPacketTime > PACKET_TIMEOUT)
  {
    setWarning(false);
  }
}
