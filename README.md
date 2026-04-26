# TermType

TermType is a typing tutor project built for an Operating Systems style capstone. The project combines a custom C backend with a browser-based typing interface. The C side is responsible for low-level modules such as string handling, memory management, math helpers, screen helpers, keyboard helpers, and prompt generation. The frontend is used to present the typing session, live metrics, and final report in a clean way.

## 1. Core Objective

This project is built to demonstrate:

- custom low-level C modules instead of depending on standard library logic for the main engine
- a working interactive typing application
- integration between memory, string, math, keyboard, and screen style modules
- a full typing workflow with prompt generation, live input handling, WPM calculation, accuracy calculation, and a final report

## 2. Project Track

This project fits **Track A: Interactive Typing Tutor**.

The user starts a typing session, types in real time, gets instant feedback, and sees a final performance report with:

- WPM
- raw WPM
- accuracy
- consistency
- correct and wrong words
- correct, incorrect, and missed characters
- total typed characters
- session duration
- a performance graph

## 3. Folder Structure

```text
TermType-main/
├── backend/
│   ├── include/
│   │   ├── keyboard.h
│   │   ├── math.h
│   │   ├── memory.h
│   │   ├── mystring.h
│   │   └── screen.h
│   ├── src/
│   │   ├── keyboard.c
│   │   ├── math.c
│   │   ├── memory.c
│   │   ├── screen.c
│   │   └── string.c
│   └── server.c
├── frontend/
│   ├── app.js
│   ├── index.html
│   └── style.css
├── Makefile
├── run.sh
└── README.md
```

## 4. Custom Libraries

### `string.c`

Current role:

- string length
- string compare
- typing prompt generation

Used for:

- building practice text from a word pool
- returning prompt text to the typing interface

### `memory.c`

Current role:

- virtual fixed memory pool
- custom allocation through `my_alloc`
- pool reset through `my_reset`

Used for:

- allocating the generated typing prompt inside the custom memory region

### `math.c`

Current role:

- custom multiplication
- custom division

Used for:

- arithmetic support for the backend module set

### `screen.c`

Current role:

- terminal print helpers

Used for:

- basic output routines for the OS-style library set

### `keyboard.c`

Current role:

- line-based keyboard input helper

Used for:

- basic input support for the OS-style library set

## 5. Integration Flow

This is the easiest way to explain the workflow in viva or presentation:

1. The browser opens the typing interface.
2. The frontend requests a prompt from the C backend.
3. The backend resets its custom memory pool using `my_reset()`.
4. `string.c` generates the practice text.
5. `memory.c` provides storage for that prompt.
6. The prompt is returned to the frontend.
7. The frontend renders the prompt immediately when the session loads.
8. As the user types, the frontend tracks correct characters, incorrect characters, missed characters, raw speed, and accuracy.
9. At the end of the session, the frontend builds the final report and performance graph.

Short explanation sentence:

`frontend input -> backend prompt generation -> custom memory + string modules -> live typing metrics -> final report`

## 6. Frontend Workflow

The frontend now follows a cleaner and more structured session flow:

1. Choose a mode: `time`, `words`, `quote`, `zen`, or `custom`
2. Choose length where applicable
3. Enable optional toggles:
   punctuation
   numbers
   sound
4. Prompt loads immediately on screen
5. User types in the main typing area
6. Extra characters beyond the word length are blocked
7. Pressing space without typing the word does not skip ahead
8. Backspace can move into the previous submitted word
9. Final report appears automatically when the session ends

## 7. Controls

- `Tab` restarts the session
- `Backspace` deletes the previous character
- `Space` submits the current word only after at least one character is typed
- `Shift + Enter` finishes zen mode
- `Restart` button starts a fresh run
- `Sound` toggle enables or disables typing feedback tones

## 8. Build and Run

### Option 1: Direct build

```bash
make
./typing_server
```

Then open:

```text
http://localhost:3000/
```

### Option 2: Run script

```bash
chmod +x run.sh
./run.sh
```

## 9. What Works

- prompt is visible from the beginning of the session
- minimal and more standard dark UI
- time, words, quote, zen, and custom modes
- punctuation and numbers toggles
- sound on/off toggle
- live WPM, raw WPM, and accuracy
- blocked empty-word space skips
- blocked overflow typing past the current word
- final session report with detailed statistics
- performance graph at the end of the test

## 10. Known Issues

- the main user demo is browser-based, not a pure terminal-rendered final loop
- `screen.c` and `keyboard.c` exist as project modules, but the current polished demo uses the web interface as the primary interaction layer
- the prompt generator still uses a fixed word pool, so content variety can be improved further
- the sound feature depends on browser audio permission and may only start after user interaction

## 11. How To Explain The Project

You can explain the project in four short blocks:

### A. Problem

We needed a real interactive application built around custom C modules instead of relying on standard high-level helpers.

### B. Engine

The backend is split into custom libraries for memory, string handling, math, keyboard, and screen behavior. These act like small OS services.

### C. Application

The final application is a typing tutor. The backend generates prompts and manages low-level support, while the frontend handles real-time typing feedback and reporting.

### D. Outcome

The project now has a stable typing flow, correct WPM and accuracy tracking, a structured final report, and a cleaner presentation that is easier to demo and explain.

## 12. Suggested Viva Summary

TermType is an OS-inspired typing tutor where a custom C backend provides prompt generation and low-level support modules, and the frontend presents a real-time typing session with accurate performance analysis.
