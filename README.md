# Future Engineers 2025 – Self-Driving Car  
**Team:** <PKW Engineer> (School/Club: <Phuketwittayalai>)  
**Country:** <Thailand>

---

## 0) Quick Links
- Photos – team: `/t-photos/` • car: `/v-photos/`
- Diagrams: `/schemes/` (power + wiring)
- Code: `/src/`
- Videos: `/video/video.md`  *(TODO: add YouTube links later)*
- Models (3D/CAD): `/models/` *(optional)*
- Extra docs / datasheets / logs: `/other/`

---

## 1) Challenge Summary & Goals
We build an **autonomous 4-wheel car** that:
- Completes **3 laps** on a track with inner walls placed randomly (Open Challenge).
- For the final mission (Obstacle Challenge), obeys **traffic posts**: **RED → keep right, GREEN → keep left**.
- Finishes with **parallel parking** inside the designated area.
- Complies with the WRO requirements: one power switch + one start button, fully autonomous driving (no remote control).

**Software priority rule:** When a color post is detected, **Color (TCS3200) overrides Ultrasonic** decisions until the color maneuver completes, then Ultrasonic avoidance resumes.

---

## 2) System Overview

### 2.1 Electronics
- **MCU:** Arduino UNO R4 WiFi
- **Motor Driver:** L298N (rear DC motor drive; PWM speed on ENA)
- **Steering:** Front-wheel steering via servo (linkage to front axle)
- **Sensors:**
  - **Ultrasonic HC-SR04** on a scanning servo (sweeps left–right–center to find a free direction)
  - **TCS3200 color sensor** for red/green post recognition
- **Power:**
  - **2S Li-ion 7.4 V** battery
  - **Buck Converter 5 V (≥3 A)** feeding: both servos + HC-SR04 + TCS3200
  - Arduino receives 7.4 V on **VIN**; L298N receives 7.4 V on **+12V (Vs)**
  - **Common Ground** shared among battery, Arduino, Buck (OUT−), L298N, and all sensors/servos

### 2.2 Mechanics
- Two-layer wooden chassis (balsa/plywood)
- Rear-wheel drive; front-wheel steering with tie-rod linkage
- Ultrasonic mounted on the upper servo platform
- Design targets: light weight, low steering friction, short wiring, low center of gravity

### 2.3 Start Procedure (per rules)
- One **main power switch**.
- One **start button** on the car to begin autonomous operation after setup.

---

## 3) Pin Map (Arduino UNO R4)

| Module / Signal        | Arduino Pin |
|------------------------|-------------|
| **L298N IN1**          | D2          |
| **L298N IN2**          | D3          |
| **L298N ENA (PWM)**    | D5          |
| **Steering Servo**     | D9          |
| **Scan Servo**         | D10         |
| **Ultrasonic Trig**    | D11         |
| **Ultrasonic Echo**    | D12         |
| **TCS3200 S0**         | D4          |
| **TCS3200 S1**         | D6          |
| **TCS3200 S2**         | D7          |
| **TCS3200 S3**         | D8          |
| **TCS3200 OUT**        | D13         |

> Keep motor/supply wiring **off** the breadboard; use screw terminals. Use the breadboard only for 5 V sensor/servo distribution and GND.

---

## 4) Power Distribution (very important)

**Battery 7.4 V (2S)** splits into three branches:

1) → **VIN** (Arduino UNO R4)  
2) → **L298N +12V / GND** (motor power)  
3) → **Buck Converter IN+ / IN−** → **OUT+ = 5 V**, **OUT− = GND** to:
   - Steering servo (D9, signal only from Arduino)
   - Scan servo (D10)
   - HC-SR04 (VCC)
   - TCS3200 (VCC)

**All GNDs must be common:** Battery negative, Arduino GND, Buck OUT−, L298N GND, servo/sensor GND → tied together on the breadboard GND rail.

**L298N 5V_EN jumper:**  
- Keep **installed** if you want L298N to self-power its logic from >7 V.  
- Or **remove** and feed L298N’s 5 V pin from Arduino/Buck. **Use one approach only.**

**Recommended decoupling:** 470–1000 µF electrolytic across 5 V–GND near the servo rail.

Add these two images in `/schemes/` and reference them here:
- `power-diagram.png` – battery → VIN/L298N/Buck → 5 V rail; common ground
- `wiring-diagram.png` – full pin map and connectors

---

## 5) Software Architecture

### 5.1 Modules (overview)
- **MotorControl:** direction (IN1/IN2), PWM speed (ENA)
- **SteeringControl:** servo ramping (smooth steps) with calibrated center/limits
- **UltrasonicScan:** sweep left → right → center; read distances; pick freer side
- **ColorDetect (TCS3200):** read R and G frequencies; compute dominance with a tunable delta
- **Priority Manager:** if color event (RED/GREEN) is active, **pause Ultrasonic** and execute color maneuver, then resume Ultrasonic
- **State Machine:** `IDLE → START → LAPS → PARKING → FINISH` with timeouts/failsafes

### 5.2 Key Parameters (defaults – tune on your field)
```cpp
// Steering (tune to avoid mechanical end-stops)
const int STEER_CENTER = 90, STEER_LEFT = 65, STEER_RIGHT = 115;
// Scan angles
const int SCAN_LEFT = 150, SCAN_RIGHT = 30, SCAN_CENTER = 90;
// Color threshold (difference between red/green frequency)
const int COLOR_DELTA = 20; // increase if lighting is noisy
// Motor speed for maneuvers
const uint8_t SPEED_CRUISE = 150;
```

### 5.3 Build & Flash (UNO R4)
- **IDE:** Arduino IDE 2.x  
- **Board:** Arduino UNO R4 WiFi  
- **Libraries:** `Servo.h` (built-in)  
- **Steps:** Open `/src/main.ino` → Select board/port → Upload  
- **If upload fails (COM issues):** try another USB port/cable, select the right COM, press **RESET** while IDE shows *Uploading…*

### 5.4 Calibration
1. Center the steering: set `STEER_CENTER` so wheels are straight.  
2. Limit `STEER_LEFT/RIGHT` to avoid mechanical end-stops (leave 5–10° margin).  
3. Tune `COLOR_DELTA` under venue lighting (record typical red/green frequency).  
4. Verify ultrasonic timeout, max distance, and scanning angles on your track.

## 6) Testing Plan
- **Bench:** motor forward/backward; steering sweep; color readouts; ultrasonic vs ruler.  
- **Track – Open Challenge:** 3 laps with randomized inner walls; measure lap time & success rate.  
- **Track – Obstacle Challenge:** obey red/green posts; color overrides ultrasonic; end with parallel parking.  
- **Robustness:** repeated start/stop; brown-out checks; recover after bumps.

## 7) Troubleshooting & Lessons Learned
- Power drops → fixed with **Buck 5 V (≥3 A)** and common ground; add 470–1000 µF near servo rail.  
- L298N weak/no output → ENA must be PWM (D5) or jumper ENA→5V for full-speed test; check `5V_EN`.  
- Servo jitter/stall → ramp angles gradually; keep linkage short; avoid end-stop binding.  
- COM “No device found” → choose correct port, try other USB/cable, press RESET during upload.  
- Direction reversed → swap motor leads (OUT1/OUT2) or invert IN1/IN2 logic.

## 8) Bill of Materials (BOM)
| Item | Qty | Notes |
|------|-----|------|
| Arduino UNO R4 WiFi | 1 | Main controller |
| L298N motor driver | 1 | Rear DC motor |
| DC motor (rear) | 1 | Drive |
| SG90/MG90S servos | 2 | Steering + scan |
| HC-SR04 ultrasonic | 1 | Range sensing |
| TCS3200 color sensor | 1 | Red/Green detection |
| Buck Converter 5 V ≥3 A | 1 | Power for servos & sensors |
| 2S Li-ion 7.4 V | 1 | Main battery |
| Breadboard, jumpers, capacitors | – | 5 V distribution & decoupling |
| Chassis/wheels/linkage hardware | – | Mechanical |

## 9) Repository Structure
/src → Arduino code (UNO R4)
/schemes → power-diagram.png, wiring-diagram.png
/v-photos → 6 car photos (front/back/left/right/top/bottom)
/t-photos → 2 team photos (official + funny)
/video → video.md with YouTube links (TODO)
/models → STL/DXF/STEP (optional)
/other → datasheets, logs, test notes, BOM
/docs → (optional) flowcharts, extra documentation
README.md

Copy code

**Notes**
- Keep motor power wiring out of the breadboard (use screw terminals).
- Add short captions in `/schemes/README.md` describing each diagram.
- Commit messages should be meaningful (e.g., `feat: add motor control`, `docs: wiring diagram`).

## 10) Videos (TODO – add later)
A short driving demonstration for each mission (≥ 30 s):
- **Open Challenge:** <YouTube URL>
- **Obstacle Challenge:** <YouTube URL>

Create a file `/video/video.md` with:
~~~md
# Videos
- Open Challenge (≥ 30 s): <YouTube URL>
- Obstacle Challenge (≥ 30 s): <YouTube URL>
~~~

> Tip: record in good lighting; show start procedure, 3 laps, color post reaction, and parallel parking.

## 11) References & License

### References
- Arduino documentation: https://www.arduino.cc/reference/en/  
- TCS3200 color sensor datasheet  
- HC-SR04 ultrasonic module datasheet  
- L298N motor driver documentation  

### License
All source code and documentation in this repository are released under the **MIT License**, unless otherwise specified.
