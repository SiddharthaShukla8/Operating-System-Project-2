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
│   ├── data/
│   │   └── words.json        # 650-word prompt dataset loaded once at startup
│   ├── include/            # Custom headers for OS-style modules
│   │   ├── input_engine.h
│   │   ├── json_loader.h
│   │   ├── keyboard.h
│   │   ├── math.h
│   │   ├── memory.h
│   │   ├── mystring.h
│   │   └── screen.h
│   ├── src/                # Modular C implementations
│   │   ├── input_engine.c
│   │   ├── json_loader.c
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
- **Role**: Custom first-fit allocator with block metadata, ownership tracking, reset support, and fragmentation awareness.
- **OS Simulation**: Simulates virtual RAM management with visible allocation/deallocation flow and session memory accounting.

### `string.c`
- **Role**: Length/compare/copy helpers plus JSON-driven sentence generation and response-string assembly.
- **OS Simulation**: Simulates low-level character buffer processing used in prompt building and report formatting.

### `math.c`
- **Role**: Custom multiplication and division logic.
- **OS Simulation**: Replaces standard math headers with optimized integer arithmetic for system metrics.

### `json_loader.c`
- **Role**: Loads and validates `backend/data/words.json` once at startup, then stores the dataset in persistent memory.
- **OS Simulation**: Simulates a safe resource loader with explicit parsing and ownership rules.

### `input_engine.c`
- **Role**: Circular-buffer input queue with polling APIs (`input_read`, `input_peek`, `input_flush`, `input_has_pending`).
- **OS Simulation**: Simulates a low-level keyboard/input controller without directly coupling typing logic to `getchar`.

---

## 6. Workflow & Integration

1. **Startup Cache**: `server.c` initializes the allocator, loads `words.json` through `json_loader.c`, and keeps the dataset in persistent memory.
2. **Prompt Request**: The frontend requests `/api/sentence` with count, difficulty, punctuation, and number flags.
3. **Session Reset**: The backend resets non-persistent memory, resets the input engine session buffer, and prepares a fresh sentence allocation.
4. **Sentence Generation**: `string.c` randomly selects words from the cached dataset, avoids nearby duplicates, and allocates the sentence safely inside the memory pool.
5. **Metadata Response**: The backend returns the sentence plus dataset, input buffer, and memory-usage stats in one JSON payload.
6. **Interactive Session**: The browser typing engine handles cursor flow, backspace, accuracy, WPM, raw speed, and consistency live.
7. **Final Analytics**: The result screen shows performance graph, character/word errors, sentence-generation details, and allocator usage.

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
