# Operating-System-Project-2

A small, modular C program written in an "OS-style" layout (separate `screen`, `keyboard`, `memory`, `math`, `string` modules).  
It runs a simple typing test: it prints a randomly generated sentence, reads what you type, then reports accuracy and error count.

## Project Layout

- `src/main.c` : Program entry point (typing test flow and accuracy calculation)
- `src/screen.c`, `include/screen.h` : Output helpers (wrap `printf`)
- `src/keyboard.c`, `include/keyboard.h` : Input helper (`getchar`-based line read)
- `src/memory.c`, `include/memory.h` : Simple fixed memory-pool allocator (`my_alloc`)
- `src/math.c`, `include/math.h` : Integer multiply/divide using loops
- `src/string.c`, `include/mystring.h` : Tiny string utilities + sentence generator

## Build

From the repo root:

```sh
gcc src/*.c -Iinclude -o app
```

## Run

```sh
./app
```

You will see a prompt like:

```text
Type this:
<generated sentence>

Your input:
```

After you type and press Enter, it prints:

- `Accuracy`: percentage of correct characters (integer)
- `Errors`: number of mismatched character positions

## How It Works (High Level)

1. `main.c` seeds the RNG (`srand(1)`) and calls `generate_sentence(5)`.
2. `generate_sentence` selects words from a small dictionary, allocates memory using `my_alloc`, and builds one string.
3. The sentence is printed using the `screen_*` functions.
4. User input is read using `keyboard_readline`.
5. `main.c` compares the typed input against the sentence character-by-character and counts mismatches.
6. Accuracy is computed as `correct * 100 / len` using `my_multiply` and `my_divide`.

## Notes / Limitations

- Deterministic output: `srand(1)` means the "random" sentence is the same on every run unless you change the seed.
- Error counting: comparison is done only for `len` characters (length of the target sentence). Extra characters typed beyond `len` are not counted as errors.
- Memory allocator: `my_alloc` is a bump allocator over a fixed `65536` byte pool and does not free individual allocations.
- Division by zero: `my_divide(a, 0)` returns `0`.

