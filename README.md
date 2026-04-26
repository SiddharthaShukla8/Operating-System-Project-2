# ⌨️ Typing Tutor: High-Performance OS-Style Application

[![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Frontend](https://img.shields.io/badge/Frontend-Vanilla%20JS-F7DF1E?style=for-the-badge&logo=javascript&logoColor=black)](https://developer.mozilla.org/en-US/docs/Web/JavaScript)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](https://opensource.org/licenses/MIT)

A sophisticated, full-stack typing tutor engineered with an **Operating Systems perspective**. Typing Tutor bridges the gap between low-level system programming and modern web interaction, featuring a custom C backend that operates without standard library dependencies for core logic.

---

## 🌟 Highlights

- **OS-Style Modularity**: Core logic implemented in independent, low-level modules simulating kernel-space services.
- **Zero-Dependency Core**: Custom implementations of `malloc`, `string.h`, and `math.h`.
- **Hybrid Architecture**: High-efficiency C backend paired with a reactive, glassmorphic web frontend.
- **Precision Metrics**: Frame-perfect WPM and Accuracy tracking with sub-millisecond precision.

---

## 🏗️ System Architecture

### 🛡️ The Backend (Kernel Simulation)
The backend is designed to demonstrate how a basic operating system handles resource management and I/O.

| Module | Responsibility | Technical Implementation |
| :--- | :--- | :--- |
| **Memory Manager** | Dynamic Allocation | Fixed-pool bump allocator with zero fragmentation overhead. |
| **String Engine** | Text Processing | Custom character-level buffer management and sentence synthesis. |
| **Math Library** | Statistics Calculation | High-performance integer arithmetic for WPM/Accuracy logic. |
| **Terminal I/O** | Input/Output | Low-level keyboard and screen buffer management. |
| **Network Layer** | HTTP API Server | Minimalist TCP server handling JSON-based communication. |

### 🎨 The Frontend (Modern UX)
The web interface focuses on speed and aesthetic excellence:
- **Glassmorphic Design**: A modern, translucent UI that feels premium and distraction-free.
- **Reactive Typing Engine**: Real-time feedback for correct/incorrect keystrokes.
- **Session Intelligence**: Dynamic sentence fetching and customizable time intervals (15s, 30s, 60s).

---

## 🚀 Getting Started

### 📦 Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/SiddharthaShukla8/Operating-System-Project-2.git
   cd Operating-System-Project-2
   ```

2. **Grant execution permissions**
   ```bash
   chmod +x run.sh
   ```

3. **Launch the environment**
   ```bash
   ./run.sh
   ```

### 🎮 Usage
- Navigate to `http://localhost:8080` in your preferred browser.
- Select your preferred time duration.
- Start typing! The timer begins automatically on the first keystroke.
- Press `Tab + Enter` to instantly restart a session.

---

## 👥 The Development Team

<div align="center">

| Name | Roll Number | Role |
| :--- | :--- | :--- |
| **Siddhartha Shukla** | 230104 | System Architecture & Backend Lead |
| **Sugat Athawale** | 230109 | Frontend Engineering & UX Design |
| **Piyush Kaushal** | 230112 | Algorithm Optimization & Testing |

</div>

---

## 🔮 Future Roadmap
- [ ] **Global Leaderboards**: Competitive typing with real-time score persistence.
- [ ] **Multi-language Support**: Typing tests in various programming languages and natural languages.
- [ ] **Custom Themes**: User-defined CSS injection for personalized aesthetics.
- [ ] **Code Mode**: Specialized mode for improving programming syntax speed.

---

<div align="center">
Developed with ❤️ for the Operating Systems Laboratory.
</div>
