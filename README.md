[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/gnHZBlmB)

# **Helix Vault**

### *Embedded Systems Final Project — ESE 5190 (Fall 2025)*

A three layer secure safe integrating biometric authentication, analog combination input, and PIN verification. Powered by two ATmega328PB microcontrollers and an ESP32 fingerprint module, Helix Vault blends embedded security logic, mechanical actuation, and a guided LCD user interface.

**Team:** Byte This (Team 16)

**Members:** Yongwoo Park, Jeevan Karandikar, Tomas Ascoli

---

# ## **Demo Video**

<iframe src="https://drive.google.com/file/d/1yrrJclSwQNpPtwRqeH9M1_K7XSpKtqIG/preview"
width="720" height="480" allow="autoplay"></iframe>

If the video does not load, click here:

**[https://youtu.be/f8Z03Lm1yS4](https://youtu.be/f8Z03Lm1yS4)**

---

# ## **Final Product Images**

![Fingerprint Stage Unlocked](images/after_fingerprint.jpg)
![Fully Unlocked Safe](images/unlocked.jpg)
![Circuit Diagram</span><span></code>](images/circuit_diagram.png)
![Electronics](images/circuit_wiring.png)

### **Prototype Images**

![Breadboarded Electronics](images/breadboard.png)
![LCD Breadboarding](images/LCD_password.jpg)
![Front Panel Sketch](images/FrontPanelSketch.png)
![Top Panel Sketch](images/TopPanelSketch.png)

---

# # **System Overview**

Helix Vault uses a three stage unlock sequence:

1. **Fingerprint verification** handled by an ESP32 and R503 optical fingerprint sensor
2. **Analog combination** read via ADC knobs and switches
3. **PIN entry** using a matrix keypad

Only when all three layers succeed, in order, does the servo release the final latch.

A DC motor opens the internal sliding door immediately after the fingerprint stage, revealing the analog combination panel and the keypad.

---

## **System Block Diagram**

![Block Diagram](images/block_diagram.png)

---

## **Software Requirements Specification (SRS)**

### **Definitions**

* **MCU** : Microcontroller Unit
* **LCD** : Liquid Crystal Display
* **ADC** : Analog-to-digital converter
* **PIN** : Personal Identification Number
* **Unlock Sequence** : Fingerprint → Combination → PIN
* **R503** : Fingerprint sensor module

---

#### **SRS-01 - Low Power Management - when the system is not being used, it should be in low power mode. If the battery dies, it can be recharged without unlocking or compromising the safe.**

Result: We did not incorporate a low power feature when the system is not being used. However, we did make sure that powering up the system does not compromise any of the safety features of the box. When the box is powered up it makes sure all the locks or closed. However, it is important that if the user is going to power it off (disconnect it from the wall), they make sure the latch is engaged and the box is closed. The box doesn't lock itself when disconnected, but it does have an easy lock feature - when the box is open and the user presses "*" on the keypad, the box locks and goes back to fingerprint sensing (first security layer). Additionally, if the box is powered off at any point during the unlocking process, it will reset and go back to the first security layer.

#### **SRS-02 - Correct Password Recognition - the safe only opens when the correct password is typed into the system.**

Result: Requirement fully met. The first security layer can only be bypassed by the fingerprint reading one of the saved fingerprints (we tested with different people and fingers and the fingerprint sensor + software never failed). Each saved fingerprint is linked to its own combination + PIN, which is all handled and successfully tested by our software. The system will not read anything inputed into the keypad if the second layer of security (analog combination) hasn't been bypassed. Only the PIN that corresponds to the user's fingerprint will open the box. Any PINS longer than 4 digits are ignored and shorter/incorrect PINS do not open the box under any circumstance.

#### **SRS-03 - Servo Control - the 1st servo only opens the door when both the facial recognition and fingerprint scans have been successful and closes it when the user closes the box and presses the close button.**

Result: We changed this requirement to servo + motor control, since the first door is opened using a DC motor and the latch is opened using a servo. The first door only opens when one of the saved fingerprints is read, any other fingerprint does not open the door - this was thoroughly tested with 10+ incorrect fingers and a 100% success rate (success = door did not open). The analog combination + PIN panel cannot be accessed if the first door doesn't open, so there is no way to actuate the servo without getting a successful fingerprint reading first. The servo only actuates when the correct PIN is typed in (10+ incorrect PINs tested with 100% success rate). The security levels have to be bypassed sequentially (software architecture), so there is no way to actuate the servo with the correct PIN without the correct fingerprint and combination (correct PIN also depends on the fingerprint reading).

#### **SRS-04 - System Lockdown - the system locks down for 5 minutes if the user types in the incorrect password 3 times in a row.**

Result: The system does not lock down after 3 incorrect tries, but does lock down if the user "tells" it to lock down.

#### **SRS-05 - Storage Management - there is an interface for creating new passwords and deleting old ones. The system will reject an attempt to add a new password when storage is full.**

Result: We modified this requirement, instead of having a built in interface that allows the user to change passwords and add fingerprints, this can be done using a computer and connecting to the ATMega's when the box is open and the lid protecting the electronics is taken off.

---

# # **Hardware Requirements Specification (HRS)**

### **4.1 Definitions**

* **DC Motor** : Linear actuator for sliding door
* **Servo** : Rotary actuator for latch
* **H-Bridge** : Motor driver IC
* **Sliding Door** : Inner door covering combination + keypad

---

### **4.2 Functional Requirements**

#### **HRS-01 — HRS-01 - Microcontroller - the overall system control will be provided by the ATmega328PB.**

Result: No changes

#### **HRS-02 - Biometric Lock 1 - our first biometric lock will be a fingerprint scanner. The fingerprint scanner should be able to scan and recognize our (Yongwoo, Jeevan, and Tomas) fingerprints, and differentiate between them.**

Result: success

#### **HRS-03 - Biometric Lock 2 - our second biometric scanner will be a facial recognition camera. This camera should be able to scan and recognize our faces, and differentiate between them.**

Result: Unfortunately due to shipping issues we never received the facial recognition module.

#### **HRS-04 - Keypad - our last lock will be a keypad, where the user has to type an X-digit password to open the box. It will have two LEDs: a red LED that turns on when the keypad is on but the box is closed, a green LED that turns on when the correct password is typed in.**

Results: 4 digit keypad works. did away with the LEDs, use LCD instead for progress

#### **HRS-05 - Servo - we will use a servo to open the door that allows the user to use the knobs and keypad. This door will only open if the facial recognition and fingerprint recognize the user.**

Result: changed this to a DC motor driving a belt for linear actuation

#### **HRS-06 - Relay - if we decide to use a relay to turn on the keypad only when the correct combination of switches and knobs is inputted.**

Result: keypad ended up not needing power, just GPIO scanning

---

## **Conclusion**

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

### **References**

* R503 Fingerprint Sensor Library: [https://github.com/mpagnoulle/R503-Fingerprint-Sensor-Library](https://github.com/mpagnoulle/R503-Fingerprint-Sensor-Library)
  * Used on ESP32

---

### **Team Information**

**Team 16 — Byte This**

* Jeevan Karandikar — [jeev@seas.upenn.edu]()
* Yongwoo Park — [yongwoo@seas.upenn.edu]()
* Tomas Ascoli — [tascoli@seas.upenn.edu]()

---

### **Repository Links**

* **GitHub Repo:** [https://github.com/upenn-embedded/final-project-website-submission-f25-t16-f25-byte-this](https://github.com/upenn-embedded/final-project-website-submission-f25-t16-f25-byte-this)
