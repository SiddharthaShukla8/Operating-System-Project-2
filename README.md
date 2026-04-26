# Typing Tutor

Typing Tutor is an OS-inspired typing tutor project built for an Operating Systems Laboratory capstone. The project combines a custom C backend with a browser-based interactive interface. The backend is responsible for low-level modules such as string handling, memory management, math helpers, screen helpers, keyboard helpers, and dynamic prompt generation, while the frontend provides a high-performance typing experience with live metrics.

## 1. Core Objective

This project demonstrates:
- **Custom Low-Level C Modules**: Core engine built without standard library dependencies for critical logic.
- **Interactive Full-Stack Application**: Real-time integration between a C server and a web frontend.
- **System Module Integration**: Coordination between memory, string, math, keyboard, and screen-style modules.
- **Performance Analytics**: Precise calculation of WPM, Accuracy, and character-level statistics.

## 2. Project Track

This project fits **Track A: Interactive Typing Tutor**.

The user starts a typing session, receives instant feedback, and views a comprehensive final performance report including:
- **WPM & Raw WPM**
- **Accuracy & Consistency**
- **Detailed Character Breakdown** (Correct, Incorrect, Missed, Extra)
- **Session History Graph** (Performance over time)

## 3. Project Team

Developed by:
1. **Siddhartha Shukla** (230104)
2. **Sugat Athawale** (230109)
3. **Piyush Kaushal** (230112)

## 4. Folder Structure

```text
TypingTutor/
├── backend/
│   ├── include/            # Custom headers for OS-style modules
│   │   ├── keyboard.h
│   │   ├── math.h
│   │   ├── memory.h
│   │   ├── mystring.h
│   │   └── screen.h
│   ├── src/                # Modular C implementations
│   │   ├── keyboard.c
│   │   ├── math.c
│   │   ├── memory.c
│   │   ├── screen.c
│   │   └── string.c
│   └── server.c            # API Server & Task Management
├── frontend/
│   ├── app.js              # Reactive UI Logic
│   ├── index.html          # Application Structure
│   └── style.css           # Premium Aesthetics
├── Makefile                # Build Automation
├── run.sh                  # Execution Script
└── README.md
```

## 5. Custom Libraries & OS Simulation

### `memory.c`
- **Role**: Fixed memory-pool allocator (`my_alloc`, `my_reset`).
- **OS Simulation**: Simulates static kernel memory allocation, ensuring zero fragmentation and deterministic performance.

### `string.c`
- **Role**: Length calculation, string comparison, and prompt synthesis.
- **OS Simulation**: Simulates low-level character buffer processing used in terminal drivers.

### `math.c`
- **Role**: Custom multiplication and division logic.
- **OS Simulation**: Replaces standard math headers with optimized integer arithmetic for system metrics.

---

## 6. Workflow & Integration

1. **Frontend Request**: The browser requests a typing prompt from the C backend.
2. **Backend Synthesis**: The server resets the memory pool (`my_reset()`) and generates a prompt using `string.c`.
3. **Prompt Delivery**: The generated text is served via the API to the frontend.
4. **Interactive Session**: The user types; the frontend calculates live metrics (WPM/Acc).
5. **Final Analytics**: Upon completion, a detailed report and performance graph are generated.

## 7. Features & Controls

### Modes
- **Time**: 15s, 30s, 60s challenges.
- **Words**: Fixed word count targets.
- **Zen**: Infinite typing for practice.
- **Custom**: User-defined parameters.

### Controls
- `Tab`: Instant Restart.
- `Backspace`: Character deletion.
- `Space`: Word submission.
- `Command Line`: Accessible via `ESC` or `Ctrl+Shift+P`.

---

## 8. Build and Run

### Prerequisites
- GCC Compiler
- Make

### Quick Start
```bash
chmod +x run.sh
./run.sh
```

Once running, access the application at:
`http://localhost:3000/`

---

## 9. Suggested Viva Summary

**Typing Tutor** is an OS-inspired application where a modular C backend provides low-level system services (memory, string, math) and prompt generation, while a modern frontend delivers a high-fidelity typing experience with real-time performance analytics.
