# ============================================================================
# Makefile -- CMPS2131 Lab 1: Token Linked List (C23)
#
# `make` reads this file and runs the recipe for the target you ask for, e.g.
# `make valgrind`. Each target below is a step in the workflow you will use all
# semester. Read the comments -- they explain WHAT each tool is for, so you can
# work without waiting for help.
#
# Quick tour:
#   make            build the debug program        (./tokens)
#   make run        build and run it
#   make valgrind   run it under Valgrind          (memory-leak gate)
#   make sanitize   run it under ASan + UBSan      (catches bugs Valgrind can't)
#   make dot        draw the list                  (tokens.png)
#   make format     auto-format the code
#   make tidy       static analysis (clang-tidy)
#   make bench      benchmark scaling (hyperfine)
#   make perf       hardware counters (perf)
#   make clean      delete build products
# ============================================================================

# `?=` sets a default you can override on the command line, so both compilers
# your course requires are one word away:  make            (gcc)
#                                          make CC=clang    (clang)
CC ?= gcc

STD  = -std=c23
# The warning wall. Treat warnings as errors so nothing sloppy slips through.
#   -Wconversion catches silent narrowing (e.g. size_t -> int) that bites later.
WARN = -Wall -Wextra -Wconversion -Werror
DBG  = -g -Og          # debug info + light optimisation (kind to the debugger)
REL  = -O2 -DNDEBUG    # optimised release build
# Sanitizers are just compiler flags -- nothing to install. They instrument the
# program to catch memory and undefined-behaviour bugs AT RUNTIME.
SAN  = -fsanitize=address,undefined -fno-omit-frame-pointer

SRC = token.c list.c main.c
HDR = token.h list.h
BIN     = tokens
BIN_SAN = tokens-asan

# The default target (first one in the file) is a plain debug build. It has NO
# sanitizers on purpose, because Valgrind and AddressSanitizer must not run on
# the same binary -- they fight over the memory allocator.
all: $(BIN)

$(BIN): $(SRC) $(HDR)
	$(CC) $(STD) $(WARN) $(DBG) $(SRC) -o $@

# Optimised build, for the perf / hyperfine measurements.
release: $(SRC) $(HDR)
	$(CC) $(STD) $(WARN) $(REL) $(SRC) -o $(BIN)

# The sanitized binary is SEPARATE (tokens-asan) for the reason noted above.
$(BIN_SAN): $(SRC) $(HDR)
	$(CC) $(STD) $(WARN) $(DBG) $(SAN) $(SRC) -o $@

run: $(BIN)
	./$(BIN)

# --- memory + undefined-behaviour tools -------------------------------------
# Valgrind runs your program on a synthetic CPU and watches every memory access.
# It is the HARD GATE for this course: a leak or an invalid read fails the lab.
#   --leak-check=full        report each leaked block
#   --show-leak-kinds=all    include "still reachable" blocks, not just lost ones
#   --track-origins=yes      say WHERE an uninitialised value came from
#   --error-exitcode=1       make `make valgrind` fail (non-zero) on any error
valgrind: $(BIN)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
	         --error-exitcode=1 ./$(BIN)

# AddressSanitizer + UndefinedBehaviorSanitizer. Faster than Valgrind and it
# catches things Valgrind misses: stack-buffer overflows, use-after-return,
# signed-integer-overflow UB. Use BOTH tools -- they overlap only partly.
sanitize: $(BIN_SAN)
	./$(BIN_SAN)

# --- style + static analysis ------------------------------------------------
# clang-format rewrites your code to one consistent style (see .clang-format),
# so the whole class -- and your future self -- reads the same shape of code.
format:
	clang-format -i $(SRC) $(HDR)

# The check-only version: it changes nothing and FAILS if any file is not
# already formatted. This is what a grading script or CI would run.
format-check:
	clang-format --dry-run --Werror $(SRC) $(HDR)

# clang-tidy reads your code like a very picky senior engineer and flags likely
# bugs and risky patterns (see .clang-tidy). Advisory here, not a hard gate.
tidy:
	clang-tidy $(SRC) -- $(STD)

# --- visualise --------------------------------------------------------------
# Graphviz turns the DOT text your program prints into a real picture, so the
# "boxes and arrows" stop being abstract. Opens as tokens.png.
dot: $(BIN)
	./$(BIN) --dot > tokens.dot
	dot -Tpng tokens.dot -o tokens.png
	@echo "wrote tokens.png -- open it to see your list"

# --- measure ----------------------------------------------------------------
# hyperfine runs a command many times and reports a clean average with error
# bars, so you can compare two input sizes fairly. Watch the time grow with N:
# that growth IS the O(n) of the search, seen empirically.
bench: release
	hyperfine './$(BIN) --count 100000' './$(BIN) --count 1000000'

# perf reads the CPU's hardware counters (instructions, cache misses, ...) for
# one run. Use it to connect the Big-O you derived on paper to real behaviour.
perf: release
	perf stat ./$(BIN) --count 1000000

clean:
	rm -f $(BIN) $(BIN_SAN) tokens.dot tokens.png

# `.PHONY` tells make these targets are NOT filenames, so they always run even
# if a file of the same name happens to exist.
.PHONY: all release run valgrind sanitize format format-check tidy dot bench perf clean
