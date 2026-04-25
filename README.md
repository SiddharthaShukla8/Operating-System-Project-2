# TermType: Modular Typing Tutor

TermType is a high-performance, modular typing tutor application built as part of the Operating Systems Laboratory. The project demonstrates low-level system programming concepts in C, integrated with a modern web-based frontend.

## 🚀 Overview

TermType evolved from a terminal-based CLI tool (Phase 1) into a full-stack interactive application (Phase 2). It features a custom-built C backend that simulates core operating system modules, providing a robust foundation for character processing and session management.

### Key Features
- **Modular C Backend**: Implements custom libraries for memory management, string manipulation, and terminal I/O.
- **Real-time Analytics**: Calculates Words Per Minute (WPM), Accuracy, and detailed character-level statistics.
- **Modern Web Interface**: A sleek, responsive frontend designed for a premium user experience.
- **OS-Style Architecture**: Separate modules for `screen`, `keyboard`, `memory`, `math`, and `string` to ensure clean separation of concerns.

---

## 🛠️ Project Architecture

The project is structured into two primary components:

### 1. Backend (C)
Located in the `backend/` directory, the server is built using modular C files:
- **`memory.c`**: Custom fixed memory-pool allocator.
- **`mystring.c`**: Low-level string processing and sentence generation.
- **`math.c`**: Integer arithmetic optimized for performance.
- **`server.c`**: Lightweight HTTP server handling API requests.

### 2. Frontend (Web)
Located in the `frontend/` directory, the interface provides:
- **Dynamic Typing Engine**: Responsive character validation and cursor tracking.
- **Performance Dashboard**: Post-test analytics and visualization.
- **Customizable Sessions**: Adjustable timers and test types.

---

## 🏃 Getting Started

### Prerequisites
- GCC (GNU Compiler Collection)
- Make
- A modern web browser

### Installation & Execution
1. Clone the repository:
   ```bash
   git clone https://github.com/SiddharthaShukla8/Operating-System-Project-2.git
   cd Operating-System-Project-2
   ```

2. Run the application using the provided script:
   ```bash
   chmod +x run.sh
   ./run.sh
   ```
   This will compile the backend using the `Makefile` and launch the server.

3. Access the application:
   Open your browser and navigate to `http://localhost:8080`.

---

## 👥 Project Team

This project was developed by:

1. **Siddhartha Shukla** (Roll No: 230104)
2. **Sugat Athawale** (Roll No: 230109)
3. **Piyush Kaushal** (Roll No: 230112)

---

## 📜 License
This project is developed for educational purposes as part of the Operating Systems curriculum.
