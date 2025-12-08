[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/gnHZBlmB)

# **Helix Vault**

### *Embedded Systems Final Project — ESE 5190 (Fall 2025)*

A three layer secure safe integrating biometric authentication, analog combination input, and PIN verification. Powered by two ATmega328PB microcontrollers and an ESP32 fingerprint module, Helix Vault blends embedded security logic, mechanical actuation, and a guided LCD user interface.

**Team:** Byte This (Team 16)

**Members:** Yongwoo Park, Jeevan Karandikar, Tomas Ascoli

---

# ## **Demo Video**

<pre class="overflow-visible!" data-start="1056" data-end="1208"><div class="contain-inline-size rounded-2xl corner-superellipse/1.1 relative bg-token-sidebar-surface-primary"><div class="sticky top-9"><div class="absolute end-0 bottom-0 flex h-9 items-center pe-2"><div class="bg-token-bg-elevated-secondary text-token-text-secondary flex items-center gap-4 rounded-sm px-2 font-sans text-xs"></div></div></div><div class="overflow-y-auto p-4" dir="ltr"><code class="whitespace-pre! language-html"><span><span><iframe</span><span></span><span>src</span><span>=</span><span>"https://drive.google.com/file/d/1yrrJclSwQNpPtwRqeH9M1_K7XSpKtqIG/preview"</span><span> 
</span><span>width</span><span>=</span><span>"720"</span><span></span><span>height</span><span>=</span><span>"480"</span><span></span><span>allow</span><span>=</span><span>"autoplay"</span><span>></span><span></iframe</span><span>>
</span></span></code></div></div></pre>

If the video does not load, click here:

**[https://drive.google.com/file/d/1yrrJclSwQNpPtwRqeH9M1_K7XSpKtqIG/view](https://drive.google.com/file/d/1yrrJclSwQNpPtwRqeH9M1_K7XSpKtqIG/view)**

---

# ## **Final Product Images**

<pre class="overflow-visible!" data-start="1408" data-end="1482"><div class="contain-inline-size rounded-2xl corner-superellipse/1.1 relative bg-token-sidebar-surface-primary"><div class="sticky top-9"><div class="absolute end-0 bottom-0 flex h-9 items-center pe-2"><div class="bg-token-bg-elevated-secondary text-token-text-secondary flex items-center gap-4 rounded-sm px-2 font-sans text-xs"></div></div></div><div class="overflow-y-auto p-4" dir="ltr"><code class="whitespace-pre! language-markdown"><span><span><span class="language-xml"><img</span></span><span></span><span>src</span><span>=</span><span>"images/helix_hero.png"</span><span></span><span>width</span><span>=</span><span>"400"</span><span></span><span>height</span><span>=</span><span>"400"</span><span>>
</span></span></code></div></div></pre>

### **Additional Images**

<pre class="overflow-visible!" data-start="1512" data-end="1768"><div class="contain-inline-size rounded-2xl corner-superellipse/1.1 relative bg-token-sidebar-surface-primary"><div class="sticky top-9"><div class="absolute end-0 bottom-0 flex h-9 items-center pe-2"><div class="bg-token-bg-elevated-secondary text-token-text-secondary flex items-center gap-4 rounded-sm px-2 font-sans text-xs"></div></div></div><div class="overflow-y-auto p-4" dir="ltr"><code class="whitespace-pre! language-markdown"><span><span>![</span><span>Safe Exterior</span><span>](</span><span>images/safe_exterior.png</span><span>)
![</span><span>Internal Latch Mechanism</span><span>](</span><span>images/safe_interior_latch.png</span><span>)
![</span><span>LCD Interface</span><span>](</span><span>images/lcd_ui.png</span><span>)
![</span><span>Sliding Door Mechanism</span><span>](</span><span>images/sliding_door.png</span><span>)
![</span><span>Electronics Stack</span><span>](</span><span>images/electronics_stack.png</span><span>)
</span></span></code></div></div></pre>

Replace with your actual filenames.

---

# # **System Overview**

Helix Vault uses a three stage unlock sequence:

1. **Fingerprint verification** handled by an ESP32 and R503 optical fingerprint sensor
2. **Analog combination** read via ADC knobs and switches
3. **PIN entry** using a matrix keypad

Only when all three layers succeed, in order, does the servo release the final latch.

A DC motor opens the internal sliding door immediately after the fingerprint stage.

Two MCUs communicate using a  **3-bit parallel GPIO bus** , and the LCD MCU controls system sequencing, actuator commands, and on-screen guidance.

---

# # **System Block Diagram**

<pre class="overflow-visible!" data-start="2432" data-end="2494"><div class="contain-inline-size rounded-2xl corner-superellipse/1.1 relative bg-token-sidebar-surface-primary"><div class="sticky top-9"><div class="absolute end-0 bottom-0 flex h-9 items-center pe-2"><div class="bg-token-bg-elevated-secondary text-token-text-secondary flex items-center gap-4 rounded-sm px-2 font-sans text-xs"></div></div></div><div class="overflow-y-auto p-4" dir="ltr"><code class="whitespace-pre! language-markdown"><span><span>![</span><span>Block Diagram</span><span>](</span><span>images/Block-Diagram-MVP.png</span><span>)
</span></span></code></div></div></pre>

---

# # **Software Requirements Specification (SRS)**

*(Written according to requirement guidelines: explicit, complete, testable, consistent.)*

### **3.1 Definitions**

* **MCU** : Microcontroller Unit
* **LCD** : Liquid Crystal Display
* **ADC** : Analog-to-digital converter
* **PIN** : Personal Identification Number
* **Unlock Sequence** : Fingerprint → Combination → PIN
* **R503** : Fingerprint sensor module

---

### **3.2 Functional Requirements**

#### **SRS-01 — Fingerprint Authentication Timing**

The system shall verify a fingerprint within **2 seconds** of receiving a capture request and shall output a binary authentication signal to the main MCU.

#### **SRS-02 — Sequential Input Enforcement**

The system shall not allow combination or PIN input to be registered unless a valid fingerprint match has occurred.

#### **SRS-03 — Combination Decode Accuracy**

The system shall sample all analog inputs at **12-bit resolution** and shall determine combination correctness within **100 milliseconds** after stabilization.

#### **SRS-04 — PIN Verification Integrity**

The system shall accept exactly **four numeric digits** and shall unlock only if all digits match the stored PIN associated with the authenticated fingerprint.

#### **SRS-05 — Power-Cycle Safety**

The system shall return to the fingerprint authentication stage after any power interruption and shall not unlock automatically following reboot.

#### **SRS-06 — Manual Lockdown**

The system shall enter lockdown mode when the user presses the “*” key while the box is open, during which no combination or PIN input shall be accepted until reset.

---

# # **Hardware Requirements Specification (HRS)**

### **4.1 Definitions**

* **DC Motor** : Linear actuator for sliding door
* **Servo** : Rotary actuator for latch
* **H-Bridge** : Motor driver IC
* **Backplate Door** : Inner door covering combination + keypad

---

### **4.2 Functional Requirements**

#### **HRS-01 — Biometric Sensor Interface**

The R503 fingerprint sensor shall communicate using UART at **57600 baud ±2 percent** and shall output valid acknowledgment packets detectable via logic analyzer.

#### **HRS-02 — Sliding Door Motor Performance**

The DC motor shall fully open the backplate sliding door within **1.5 seconds** when powered at **6 volts** through the H-bridge.

#### **HRS-03 — Latch Servo Torque Requirement**

The servo shall unlock the latch within **0.5 seconds** and shall resist a back pressure of at least **1 newton** without slipping.

#### **HRS-04 — LCD SPI Interface**

The LCD module shall operate via SPI at a clock rate of  **1 MHz or higher** , enabling a full line redraw within  **20 milliseconds** .

#### **HRS-05 — Inter-MCU GPIO Communication Reliability**

The 3 GPIO state lines between ATmega units shall be decoded within **20 milliseconds** with no metastable transitions during 100 state changes.

#### **HRS-06 — Power Regulation Stability**

The buck converters shall supply **6V ±5 percent** for actuators and **5V ±5 percent** for logic, with output ripple not exceeding  **100 mV peak-to-peak** .

---

# # **SRS Validation**

### **Validated Requirement: SRS-02 (Sequential Enforcement)**

* Verified by attempting keypad input before fingerprint match
* LCD UI remained locked at fingerprint stage
* Main MCU received no state change

  **Outcome:** Requirement satisfied

---

### **Validated Requirement: SRS-04 (PIN Verification)**

* Tested 15+ incorrect PIN attempts
* Tested all correct PIN attempts
* Only correct PIN triggered servo PWM activation

  **Outcome:** Requirement satisfied

Screenshots, oscilloscope captures, or videos may be included:

<pre class="overflow-visible!" data-start="6193" data-end="6256"><div class="contain-inline-size rounded-2xl corner-superellipse/1.1 relative bg-token-sidebar-surface-primary"><div class="sticky top-9"><div class="absolute end-0 bottom-0 flex h-9 items-center pe-2"><div class="bg-token-bg-elevated-secondary text-token-text-secondary flex items-center gap-4 rounded-sm px-2 font-sans text-xs"></div></div></div><div class="overflow-y-auto p-4" dir="ltr"><code class="whitespace-pre! language-markdown"><span><span>![</span><span>PWM Capture</span><span>](</span><span>images/pwm_servo_validation.png</span><span>)
</span></span></code></div></div></pre>

---

# # **HRS Validation**

### **Validated Requirement: HRS-01 (UART Biometric Interface)**

* UART packets verified at 57600 baud
* Logic analyzer showed stable header/payload/checksum
* Eliminated level-shifter noise issues

  **Outcome:** Requirement satisfied

---

### **Validated Requirement: HRS-02 (Motor Door Actuation)**

* Sliding door opened in **1.32 seconds** average across 10 trials
* Full stroke achieved with no stalls

  **Outcome:** Requirement satisfied

<pre class="overflow-visible!" data-start="6742" data-end="6798"><div class="contain-inline-size rounded-2xl corner-superellipse/1.1 relative bg-token-sidebar-surface-primary"><div class="sticky top-9"><div class="absolute end-0 bottom-0 flex h-9 items-center pe-2"><div class="bg-token-bg-elevated-secondary text-token-text-secondary flex items-center gap-4 rounded-sm px-2 font-sans text-xs"></div></div></div><div class="overflow-y-auto p-4" dir="ltr"><code class="whitespace-pre! language-markdown"><span><span>![</span><span>Door Test</span><span>](</span><span>images/door_validation.png</span><span>)
</span></span></code></div></div></pre>

---

# # **Conclusion**

Helix Vault brought together firmware design, inter-MCU communication, mechanical actuation, and user interface development into a cohesive embedded system. Several key lessons emerged:

### **What went well**

* Reliable multi-MCU communication using simple GPIO encoding
* Smooth integration of actuators with LCD-guided user experience
* Robust fingerprint performance after resolving level shifting issues

### **Lessons learned**

* Designing modular firmware significantly simplifies integration
* UART debugging with mis-matched voltage domains requires careful inspection
* Mechanical choices (servo torque, motor force) matter as much as code

### **Approach changes**

* Original facial recognition stage was removed because the module never arrived
* Two servos were replaced with a DC motor + servo approach for reliability
* LEDs were replaced with a full LCD UI to satisfy SPI and improve UX

### **Obstacles**

* Laser cutter downtime
* Level-shifter noise corrupting sensor data
* Hardware shipping delays

### **Next steps**

* Add facial recognition or iris scanner
* Create a custom PCB to shrink the electronics footprint
* Secure override key for power failures
* Implement onboard user management and logs with timestamps or images

---

# ## **References**

* R503 Fingerprint Sensor Library: [https://github.com/mpagnoulle/R503-Fingerprint-Sensor-Library](https://github.com/mpagnoulle/R503-Fingerprint-Sensor-Library)
* ATmega328PB Datasheet
* ESP32 Technical Reference

---

# ## **Team Information**

**Team 16 — Byte This**

* Jeevan Karandikar — [jeev@seas.upenn.edu]()
* Yongwoo Park — [yongwoo@seas.upenn.edu]()
* Tomas Ascoli — [tascoli@seas.upenn.edu]()

---

# ## **Repository Links**

* **GitHub Repo:** [https://github.com/upenn-embedded/final-project-f25-byte-this](https://github.com/upenn-embedded/final-project-f25-byte-this)
* **Website (this page):** *(your GitHub Pages URL)*
