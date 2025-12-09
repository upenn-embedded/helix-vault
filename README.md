# **Helix Vault**

### *Embedded Systems Final Project — ESE 5190 (Fall 2025)*

A three layer secure safe integrating biometric authentication, analog combination input, and PIN verification. Powered by two ATmega328PB microcontrollers and an ESP32 fingerprint module, Helix Vault blends embedded security logic, mechanical actuation, and a guided LCD user interface.

**Team:** Byte This (Team 16)

**Members:** Yongwoo Park, Jeevan Karandikar, Tomas Ascoli

---

# ## **Demo Video**

**[https://youtu.be/f8Z03Lm1yS4](https://youtu.be/f8Z03Lm1yS4)**

---

# ## **Final Product Images**

![Fingerprint Stage Unlocked](images/after_fingerprint.jpg)
![Fully Unlocked Safe](images/unlocked.jpg)
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

![System Block Diagram](images/block_diagram.png)
![System Circuit Diagram](images/circuit_diagram.png)

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

Result: We did not add a low-power mode, but powering up the system never compromises safety—on startup, it always verifies that all locks are closed. If the box is powered off, it does not lock itself, so users should ensure it is closed and latched before disconnecting power. The box can also be easily locked by pressing “*”, and any power loss during unlocking resets it to the first security layer.

#### **SRS-02 - Correct Password Recognition - the safe only opens when the correct password is typed into the system.**

Result: Requirement fully met. The fingerprint sensor reliably accepts only saved fingerprints, each linked to a unique combination and PIN. The keypad is disabled until the analog combination is correctly entered, and only the exact 4-digit PIN associated with the authenticated fingerprint will open the box.

#### **SRS-03 - Servo Control - the 1st servo only opens the door when both the facial recognition and fingerprint scans have been successful and closes it when the user closes the box and presses the close button.**

Result: We updated this requirement to servo + DC motor control. The DC motor opens the first door only after a valid fingerprint match, and extensive testing with all of our unregistered fingers and other classmates confirmed that incorrect fingerprints never trigger it. The combination and PIN panel is physically inaccessible until this door opens, and the servo latch actuates only after the correct fingerprint, correct combination, and correct PIN are entered in sequence. All incorrect fingerprints and PINs tested showed a 100% success rate in blocking access.

#### **SRS-04 - System Lockdown - the system locks down for 5 minutes if the user types in the incorrect password 3 times in a row.**

Result: The system does not lock down after 3 incorrect tries, but does lock down if the user "tells" it to lock down.

#### **SRS-05 - Storage Management - there is an interface for creating new passwords and deleting old ones. The system will reject an attempt to add a new password when storage is full.**

Result: We modified this requirement, instead of having a built in interface that allows the user to change passwords and add fingerprints, this can be done using a computer and connecting to the ATMega's when the box is open and the lid protecting the electronics is taken off.

---

# # **Hardware Requirements Specification (HRS)**

### **Definitions**

* **DC Motor** : Linear actuator for sliding door
* **Servo** : Rotary actuator for latch
* **H-Bridge** : Motor driver IC
* **Sliding Door** : Inner door covering combination + keypad

---

#### **HRS-01 — HRS-01 - Microcontroller - the overall system control will be provided by the ATmega328PB.**

Result: No changes. Requirement fulfilled.

#### **HRS-02 - Biometric Lock 1 - our first biometric lock will be a fingerprint scanner. The fingerprint scanner should be able to scan and recognize our (Yongwoo, Jeevan, and Tomas) fingerprints, and differentiate between them.**

Result: No changes. Requirement fulfilled.

#### **HRS-03 - Biometric Lock 2 - our second biometric scanner will be a facial recognition camera. This camera should be able to scan and recognize our faces, and differentiate between them.**

Result: Unfortunately due to shipping issues we never received the facial recognition module.

#### **HRS-04 - Keypad - our last lock will be a keypad, where the user has to type an X-digit password to open the box. It will have two LEDs: a red LED that turns on when the keypad is on but the box is closed, a green LED that turns on when the correct password is typed in.**

Results: The keypad is used to enter a 4 digit keypad works. Instead of LEDs, an LCD screen was used instead for prompting the user and showing progress.

#### **HRS-05 - Servo - we will use a servo to open the door that allows the user to use the knobs and keypad. This door will only open if the facial recognition and fingerprint recognize the user.**

Result: The door revealing the knobs and keypad is actuated by a DC motor actuating a linear slider instead of a servo. This door only opens once the fingerprint is verified

#### **HRS-06 - Relay - Turn on the keypad only when the correct combination of switches and knobs is inputted.**

Result: The keypad does not need a separate power source - instead GPIO pins are used to scan for keypad inputs. They GPIOs do not scan the keypad unless the correct switch and knob combination is detected.

---

## **Conclusion**

Helix Vault brought together firmware design, inter-MCU communication, mechanical actuation, and user interface development into a cohesive embedded system.

### **What went well**

* Reliable multi-MCU communication using simple GPIO encoding
* Smooth integration of actuators with LCD-guided user experience
* Robust fingerprint performance
* Effective power management with simple 12V power supply to power all circuitry
* Mechanical design and hardware installation was seamless
* Efficiently split up tasks to create hardware and software parts that could come together for the final product

### **Lessons learned**

* Designing modular firmware significantly simplifies integration
* UART debugging with mis-matched voltage domains requires careful inspection
* Mechanical choices (servo torque, motor force) matter as much as code

### **Proudest Accomplishments**

* Homemade Multi MCU communication was seamless and effective
* No broken/fried electronic components
* Extremely smooth operation and unlock process
* Final result looks very close to a final prototype

### **Approach changes**

* Original facial recognition stage was removed because the module never arrived
* Two servos were replaced with a DC motor + servo approach for reliability
* LEDs were replaced with a full LCD UI to satisfy SPI and improve UX

### **Obstacles**

* Laser cutter downtime
* Hardware shipping delays
* Limited budget ($150 total)
* Low torque DC motor actuating sliding door
* Couldn't get a bare-metal C UART driver working with the

### **Next steps**

* Add facial recognition
* Add increased security layers
  * Biometric, for example iris scanner
  * More unique combination lock inputs
* Create a custom PCB to shrink the electronics footprint
* Secure override key for power failures
* Implement onboard user management
* Unlock attempt history
  * With facial recognition camera pictures can be taken of the user/intruder

---

### **References**

* R503 Fingerprint Sensor Library: [https://github.com/mpagnoulle/R503-Fingerprint-Sensor-Library](https://github.com/mpagnoulle/R503-Fingerprint-Sensor-Library)
  * Used on ESP32

---

### **Repository Links**

* **GitHub Repo:** [https://github.com/upenn-embedded/helix-vault](https://github.com/upenn-embedded/helix-vault)

---

### **Team Information**

**Team 16 — Byte This**

* Jeevan Karandikar — jeev@seas.upenn.edu
* Yongwoo Park — yongwoo@seas.upenn.edu
* Tomas Ascoli — tascoli@seas.upenn.edu

![Team Picture](images/team_pic.png)