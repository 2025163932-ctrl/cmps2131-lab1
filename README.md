# CMPS2131 — Lab 1: A Token Linked List in C23

Welcome to your first lab. This one is deliberately **over-explained**: its real
purpose is not the linked list (you already met those in Programming 2) but the
**workflow** you will use for every lab this semester — building, debugging,
checking for memory bugs, formatting, and measuring. Get comfortable here and
the rest of the course gets easier. Later labs will explain less.

---

## 1. What you will build

A **singly linked list** whose nodes hold **tokens** from the Belize language.

- A **lexeme** is the exact text taken from source code — `route`, `->`, `42`.
- A **token** is a lexeme plus its **kind** (category) and its position.
- A **lexer** turns source text into a stream of tokens. Your list is what holds
  that stream — so this lab is the seed of the lexer → parser project you build
  across the semester.

Pictured:

```
head --> [ROUTE|route] --> [IDENT|primary] --> [ARROW|->] --> ... --> nullptr
                                                                        ^tail
```

## 2. Before you start

Your Fedora 44 machine already has everything from the course setup: `gcc`,
`clang`, `make`, `gdb`, `valgrind`, `clang-format`, `clang-tidy`, `graphviz`,
`perf`, and `hyperfine`. Nothing new to install.

Unzip the lab, open a terminal in its folder, and run:

```sh
make run
```

The first run will **fail** with a message telling you the demo stream could not
be built. That is expected — turning that failure into a printed token stream is
the lab. 

## 3. The files

| File              | Yours to edit?     | What it is                                   |
|-------------------|--------------------|----------------------------------------------|
| `token.h/.c`      | No (given)         | The `Token` type and its constructor/free    |
| `list.h`          | No (the contract)  | The functions you must implement, documented |
| `list.c`          | **Yes — your work**| Fill in every `TODO`                         |
| `main.c`          | No (given)         | A driver that exercises your list            |
| `Makefile`        | No                 | The workflow (read its comments)             |
| `.clang-format`   | No                 | The house code style                         |
| `.clang-tidy`     | No                 | The static-analysis rules                    |

## 4. Your task

Implement every function in **`list.c`**. `list.h` documents exactly what each
one must do, including its **ownership rule** and its **Big-O**. A sensible order:

1. `list_length` and `list_push_back` — get one node into the list.
2. `list_print` — now you can see your work.
3. `list_destroy` — make `make valgrind` pass before you go further.
4. `list_push_front`, `list_insert_after`.
5. `list_find_first`, `list_remove_first`.
6. `list_check_invariant`.
7. `list_to_dot` — last; then run `make dot`.

**The one rule that matters most:** every token owns a heap-allocated `lexeme`.
Whoever holds the token must free it **exactly once**. When you remove or destroy
a node, call `token_free(&node->token)` *before* `free(node)`. When a `push_*`
call fails to allocate, `token_free` the token you were handed so it does not
leak. The tools in step 5 exist to prove you got this right.

## 5. The workflow — what each command is for

Run these from the lab folder.

**Build and run**
- `make` — compile the debug program `./tokens` (flags: `-std=c23 -Wall -Wextra
  -Wconversion -Werror`; warnings are errors, so fix them all).
- `make run` — build and run it.
- `make CC=clang` — build with Clang instead of GCC. Your code must compile
  cleanly on **both**; each catches things the other misses.

**Memory & undefined behaviour — the gate**
- `make valgrind` — runs your program on a simulated CPU and reports every leak
  or invalid access. **This is the hard gate: a leak fails the lab.** A pass ends
  with `ERROR SUMMARY: 0 errors` and `in use at exit: 0 bytes`.
- `make sanitize` — builds a separate binary with AddressSanitizer +
  UndefinedBehaviorSanitizer and runs it. It is faster than Valgrind and catches
  different bugs (stack overflows, use-after-return, signed-overflow UB). Use
  both. (They must not run on the *same* binary — that is why there are two.)

**Style & static analysis**
- `make format` — auto-format your code to the house style.
- `make format-check` — fails if anything is unformatted (what grading runs).
- `make tidy` — clang-tidy reads your code like a picky senior engineer. Its
  ruleset is trimmed for Lab 1 and grows stricter in later labs.

**See it**
- `make dot` — turns your list into a picture, `tokens.png`. If your boxes and
  arrows look wrong, your links are wrong.

**Measure it** (do this once the list works)
- `make bench` — `hyperfine` times `--count 100000` vs `--count 1000000`. The
  search is O(n): watch the time grow roughly with N.
- `make perf` — `perf stat` shows instructions and cache misses for one big run.
  Use it to connect the Big-O you derive on paper to what the CPU actually does.

- `make clean` — delete build products.

## 6. Reading Valgrind

A **clean** run looks like this:

```
==NNNN== All heap blocks were freed -- no leaks are possible
==NNNN== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

A **leak** looks like this (you freed the node but not its lexeme, say):

```
==NNNN== 40 bytes in 5 blocks are definitely lost in loss record 1 of 1
==NNNN==    at 0x...: malloc
==NNNN==    by 0x...: own_string (token.c:16)   <-- where the memory came from
```

The `by` lines are a stack trace: read top-down to find the allocation you never
freed. `-g` in the build is what gives you those file:line numbers.

## 7. What to hand in

1. Your completed `list.c`.
2. `tokens.png` from `make dot`.
3. A short write-up (about half a page) answering, in your own words:

   > **Who owns each lexeme string, and where is it freed?** Trace the ownership
   > of one token from `token_make`, into the list, to the moment it is released.
   > What would happen if `push_back` failed to allocate and you *didn't* free the
   > token it was given?

   Use a *They Say / I Say* move: state the ownership rule the code assumes ("the
   list takes ownership on push"), then explain how your implementation upholds
   it. Grading rewards the reasoning, not just a correct answer.

## 8. Definition of done

- [X] `make` and `make CC=clang` both compile with **zero** warnings.
- [X] `make run` prints the seven-token Belize stream.
- [X] `make valgrind` ends with `0 errors` and `0 bytes` in use at exit.
- [X] `make sanitize` runs with no ASan/UBSan reports.
- [X] `make format-check` passes.
- [X] `make dot` produces a correct `tokens.png`.
- [X] Write-up complete.

## 9. References

- Robert Seacord, *Effective C*, 2nd ed. (No Starch, C23) — the course C book.
- **SEI CERT C Coding Standard** — the security rules `clang-tidy`'s `cert-*`
  checks come from: <https://wiki.sei.cmu.edu/confluence/display/c>
