// ============================================================================
// main.c -- driver for Lab 1. This file is GIVEN to you; you do not edit it.
// It exercises the list you build in list.c.
//
// Modes:
//   (no args)     build a small, hand-written Belize token stream and show it
//   --dot         print that same stream as Graphviz DOT (see `make dot`)
//   --count N     build N tokens WITHOUT printing them -- used by the perf and
//                 hyperfine targets to measure how the list scales
//   --help        print usage
// ============================================================================

#include "list.h"

#include <stdio.h>
#include <stdlib.h> // strtol, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h> // strcmp

// Build the token stream for this one line of Belize source:
//
//     route primary -> node gateway ;
//
// In later labs a real lexer will produce these tokens from source text; today
// we hand-feed them so we can concentrate on the list and the tooling.
static bool build_demo_stream(TokenList *list) {
    // A small table so the code below reads like the source line itself. Each
    // entry is one token: kind, lexeme, line, column.
    struct {
        TokenKind kind;
        const char *text;
        int line;
        int col;
    } demo[] = {
        {TOK_ROUTE, "route", 1, 1}, {TOK_IDENT, "primary", 1, 7},  {TOK_ARROW, "->", 1, 15},
        {TOK_NODE, "node", 1, 18},  {TOK_IDENT, "gateway", 1, 23}, {TOK_SEMICOLON, ";", 1, 31},
        {TOK_EOF, "", 1, 32},
    };

    size_t n = sizeof demo / sizeof demo[0];
    for (size_t i = 0; i < n; i++) {
        Token t = token_make(demo[i].kind, demo[i].text, demo[i].line, demo[i].col);
        if (!list_push_back(list, t)) {
            return false; // out of memory: push_back already freed the token
        }
    }
    return true;
}

// Build `count` throwaway tokens so we can time list operations on a big input.
static bool build_bulk_stream(TokenList *list, long count) {
    for (long i = 0; i < count; i++) {
        Token t = token_make(TOK_IDENT, "id", 0, 0);
        if (!list_push_back(list, t)) {
            return false;
        }
    }
    return true;
}

static void print_usage(const char *prog) {
    fprintf(stderr,
            "usage: %s [--dot] [--count N] [--help]\n"
            "  (no args)  show a small Belize token stream\n"
            "  --dot      emit Graphviz DOT for the stream\n"
            "  --count N  build N tokens quietly (for perf/hyperfine)\n",
            prog);
}

int main(int argc, char **argv) {
    bool want_dot = false;
    long count    = -1; // -1 means "--count was not given"

    // A small, explicit argument loop. The library's getopt() exists, but
    // reading argv directly keeps what is happening visible while you are new
    // to C.
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "--dot") == 0) {
            want_dot = true;
        } else if (strcmp(argv[i], "--count") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --count needs a number\n");
                return EXIT_FAILURE;
            }
            count = strtol(argv[++i], nullptr, 10);
            if (count < 0) {
                fprintf(stderr, "error: --count must be >= 0\n");
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    TokenList list = list_create();

    // --- bulk mode: build many tokens, do one search, then stop (no printing) --
    if (count >= 0) {
        if (!build_bulk_stream(&list, count)) {
            fprintf(stderr, "error: out of memory building %ld tokens\n", count);
            list_destroy(&list);
            return EXIT_FAILURE;
        }
        // Touch the data so the optimiser cannot delete the list as dead code:
        // a worst-case O(n) search for a kind that is absent walks every node.
        volatile bool found = list_find_first(&list, TOK_FAULT) != nullptr;
        (void)found;
        list_destroy(&list);
        return EXIT_SUCCESS;
    }

    // --- normal mode: the small hand-written stream --------------------------
    if (!build_demo_stream(&list)) {
        fprintf(stderr, "error: out of memory building the demo stream\n");
        fprintf(stderr, "(if you have not implemented list.c yet, this is "
                        "expected -- that is the lab!)\n");
        list_destroy(&list);
        return EXIT_FAILURE;
    }

    // In debug builds, confirm the list's bookkeeping is consistent.
    if (!list_check_invariant(&list)) {
        fprintf(stderr, "BUG: the list invariant is violated\n");
        list_destroy(&list);
        return EXIT_FAILURE;
    }

    if (want_dot) {
        list_to_dot(&list, stdout);
    } else {
        printf("Belize token stream (%zu tokens):\n", list_length(&list));
        list_print(&list, stdout);

        // A quick demonstration of find + remove on the stream.
        TokenNode *arrow = list_find_first(&list, TOK_ARROW);
        if (arrow != nullptr) {
            printf("\nfound the '%s' operator at line %d, column %d\n", arrow->token.lexeme,
                   arrow->token.line, arrow->token.col);
        }
    }

    // ALWAYS clean up. Run `make valgrind`: you should see zero leaks and zero
    // errors. Forgetting this line is the single most common bug in this lab.
    list_destroy(&list);
    return EXIT_SUCCESS;
}
