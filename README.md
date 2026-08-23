🚗 Lane Change & Overtake Assistance System — LoRa-Based V2V Communication
An intelligent driver-assistance system that combines LoRa-based Vehicle-to-Vehicle (V2V) communication with image-based lane detection to warn drivers of unsafe lane-change and overtaking conditions in real time.

Platform Language Comm Status

📖 Abstract
Unsafe lane changes and blind-spot collisions remain one of the leading causes of road accidents, largely because drivers have no reliable way of sensing vehicles approaching from outside their direct line of sight. This project proposes a Lane Change and Overtake Assistance System that tackles the problem from two directions at once: long-range Vehicle-to-Vehicle (V2V) communication and real-time lane detection.

Each vehicle unit uses a GPS module to track its own position and speed and an ultrasonic sensor to gauge the distance to nearby vehicles. This data is packaged and broadcast over a LoRa radio link, chosen specifically for its ability to communicate over long distances while consuming very little power — a combination that classic options like DSRC, Wi-Fi, and cellular struggle to match. On the receiving end, a second vehicle unit decodes the incoming packet, computes the relative speed and distance to the transmitting vehicle, and compares the result against a safety threshold. When conditions turn unsafe, an LED and buzzer immediately alert the driver.

By pairing this communication layer with lane-detection concepts and basic signal-processing techniques, the system aims to close a gap left by conventional ADAS setups: sensor-only systems are blind beyond their field of view, while communication-only systems don't understand lane context. The result is a low-cost, low-power prototype that demonstrates how long-range wireless links can extend a vehicle's situational awareness well beyond what on-board sensors alone can offer — a small but practical step toward safer, smarter transportation.

✨ Key Features
📡 Long-range V2V communication via LoRa (SX1278/RFM95), enabling awareness of vehicles far beyond visual or ultrasonic range
🛰️ Real-time GPS tracking of vehicle position, speed, and direction
📏 Ultrasonic distance sensing for close-range accuracy
⚠️ Automatic safety evaluation — relative speed and distance are computed on every received packet
🔔 Instant driver alerts through LED and buzzer warnings
🔋 Low-power design, well suited to continuous in-vehicle operation
🧠 How It Works
 ┌────────────────────┐              LoRa               ┌────────────────────┐
 │   Vehicle A (Tx)    │ ───────────────────────────────▶️│   Vehicle B (Rx)    │
 │                     │      ID, Speed, Lat, Lon,        │                     │
 │  GPS + Ultrasonic   │      Direction, Distance         │  Safety Evaluation  │
 │                     │                                  │   LED + Buzzer      │
 └────────────────────┘                                  └────────────────────┘
GPS collects the vehicle's location and speed.
The ultrasonic sensor measures distance to the nearest vehicle ahead.
The transmitter packages this data and sends it over LoRa.
The receiver decodes the incoming packet.
Relative speed between vehicles is calculated.
Distance is compared against a configurable safety threshold.
If conditions are unsafe, the LED and buzzer activate to warn the driver.
This cycle repeats continuously, giving drivers a rolling picture of nearby vehicles even when they're outside the driver's direct line of sight.

📦 Communication Packet Format
As specified in the project report:

<ID, Speed, Latitude, Longitude, Direction>
As implemented in code (extended with the measured distance so the receiver can run its safety logic directly):

ID,Speed,Latitude,Longitude,Direction,Distance
🛠️ Hardware Used
Component	Purpose
ESP32 Development Board	Main microcontroller
SX1278 / RFM95 LoRa Module	Long-range V2V communication
NEO-6M GPS Module	Position and speed tracking
HC-SR04 Ultrasonic Sensor	Short-range vehicle distance detection
LED	Visual warning indicator
Buzzer	Audible warning indicator
⚠️ Note: The report does not specify exact GPIO pin mappings or the LoRa frequency used in the physical prototype. The pin assignments and 433E6 frequency in this repository's code are placeholder values — verify and update them to match your actual hardware before uploading.

💻 Software & Libraries
Arduino IDE
Embedded C
Arduino Libraries:
LoRa by Sandeep Mistry
TinyGPSPlus
📁 Repository Structure
├── Vehicle_Transmitter.ino   # Collects GPS + ultrasonic data, transmits over LoRa
├── Vehicle_Receiver.ino      # Receives LoRa data, evaluates safety, drives LED/buzzer
└── README.md
🚀 Getting Started
Clone this repository.
Open Vehicle_Transmitter.ino and Vehicle_Receiver.ino in the Arduino IDE.
Install the required libraries listed above via Sketch → Include Library → Manage Libraries.
Update the GPIO pin assignments and LoRa frequency to match your hardware.
Flash Vehicle_Transmitter.ino to the transmitting unit and Vehicle_Receiver.ino to the receiving unit.
Power on both units and monitor serial output for debugging.
Do not upload these sketches to hardware without first checking your wiring and RF configuration.

📊 Sample Results
Vehicle ID	Speed (km/h)	Distance (m)	Status
V1	60	15	✅ Safe
V2	70	8	⚠️ Warning
✅ Advantages
Long communication range with minimal power draw
Cost-effective, off-the-shelf hardware
Real-time warning generation
Stable performance across varied road conditions
⚠️ Current Limitations
LoRa's limited data rate constrains payload size and update frequency
GPS accuracy degrades in dense urban environments
Occasional communication delay under heavy channel usage
🔭 Future Scope
Integration with 5G for higher-throughput, ultra-low-latency communication
Cloud-based analytics for fleet-level insights
A dedicated in-vehicle UI for driver alerts
End-to-end encryption for secure data transmission
Coordination with autonomous-driving stacks
👥 Team
Name	Role
T. Rakesh	Contributor
M. Shravya Yadav	Contributor
D. Pranitha	Contributor
Supervisor: Ms. A D Rani Sirisha, Associate Professor Department: Electronics and Communication Engineering, Vardhaman College of Engineering

📄 License
This project was developed as part of an academic mini-project. Add a license of your choice (e.g., MIT) if you intend to open-source it.
