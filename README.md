# 🐍 Open-Snake – Modular Snake Robot with DYNAMIXEL 2XC (or 2XL)

**Open-Snake** is an open-source snake robot project built using **DYNAMIXEL 2XC-series motors**, capable of two-axis motion per segment.  
It reproduces realistic **serpentine locomotion** through chained modular segments — inspired by the motion of real snakes.

🚀 **What makes it special?**  
It runs on **low-performance hardware**, thanks to its **simple, wave-based control logic**.  
There’s no need for heavy computation or complex planning — just basic math enables smooth and flexible motion.  
Perfect for microcontroller-level systems such as **OpenRB-150** or **Arduino-class boards**.

---

## 🧠 Ideal For:
- 🤖 Robotics enthusiasts exploring undulatory movement  
- 📚 Educational demonstrations on modular kinematics  
- 🧪 Experimental development of bio-inspired locomotion  
- 🛠️ Embedded or low-cost systems with minimal computing power

---

## 🎬 Watch It in Action

### 1️⃣ Assembly + Mat Crawling Demo  
*Starts with a quick assembly view, then shows the robot slithering smoothly on a curved mat surface.*  
[![Video 1](https://img.youtube.com/vi/h0bHZ_Ra9OY/0.jpg)](https://youtu.be/h0bHZ_Ra9OY)  
👉 [Watch on YouTube](https://youtu.be/h0bHZ_Ra9OY?si=mw3a8K3SPtbjuDOo)

---

### 2️⃣ Smooth Crawling on Flat Ground  
[![Video 2](https://img.youtube.com/vi/5Tr2ZO-IKu0/0.jpg)](https://youtu.be/5Tr2ZO-IKu0)  
👉 [Watch on YouTube](https://youtu.be/5Tr2ZO-IKu0?si=_ELYOdXuaDLJPf6L)

---

## 🎥 Additional Demonstration Videos

Explore more real-world motion tests and experiments:

---

### 🎮 Simple Remote-Control Demo  
*A brief test of manual control using a remote input to activate basic movement.*  
[![Video A](https://img.youtube.com/vi/ao1gBglVeOY/0.jpg)](https://youtu.be/ao1gBglVeOY)  
👉 [Watch on YouTube](https://youtu.be/ao1gBglVeOY)

---

### 💧 On-Water Test (Reverse Direction Due to Fluid Resistance)  
*An experimental test on water. Interestingly, motion occurs in the opposite direction due to the change in force distribution. Sadly, the robot was not waterproof...*  
[![Video B](https://img.youtube.com/vi/3CEjuPAVRLE/0.jpg)](https://youtu.be/3CEjuPAVRLE)  
👉 [Watch on YouTube](https://youtu.be/3CEjuPAVRLE)

---

### 🌄 Outdoor Terrain Test – Movement Without Wheel Support  
*A field experiment showing how the robot can move over natural terrain, independent of surface type.*  
[![Video C](https://img.youtube.com/vi/01R4jejPqz8/0.jpg)](https://youtu.be/01R4jejPqz8)  
👉 [Watch on YouTube](https://youtu.be/01R4jejPqz8)

---

### 🌊 Smooth Motion Without Wheels on Flat Surface  
*Demonstrating smooth crawling on a flat surface with no wheels or rolling parts.*  
[![Video D](https://img.youtube.com/vi/VN9_0te3Hug/0.jpg)](https://youtu.be/VN9_0te3Hug)  
👉 [Watch on YouTube](https://youtu.be/VN9_0te3Hug)

---

### 🐍 Compact Zig-Zag Motion (Wave Timing Test)  
*A test focused on optimizing wave phase and timing between segments for tight turns.*  
[![Video E](https://img.youtube.com/vi/9r-uT1uWE80/0.jpg)](https://youtu.be/9r-uT1uWE80)  
👉 [Watch on YouTube](https://youtu.be/9r-uT1uWE80)

---

## 🔧 Key Features

- Built using **DYNAMIXEL 2XC (or 2XL)** – dual-axis control per module  
- Modular architecture – simply connect more segments to scale  
- Lightweight control logic – runs even on microcontrollers  
- STL files provided for 3D printing  
- Fully open hardware – easy to build, modify, and extend

---

## 📁 Repository Structure

```plaintext
Open-Snake/
├── stl/             # 3D printable parts for each segment
├── docs/            # Wiring diagrams and assembly images
├── firmware/        # (Coming soon) control logic for locomotion
└── README.md
