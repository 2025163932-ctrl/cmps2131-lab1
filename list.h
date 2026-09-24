// ============================================================================
// list.h -- a singly linked list of Tokens.
//
// Picture it as boxes and arrows:
//
//   head --> [tok|next] --> [tok|next] --> [tok|next] --> nullptr
//                                           ^tail
//
// Each box is a heap-allocated NODE holding one Token and a pointer to the next
// box. `head` is the first box, `tail` is the last (so appending is cheap), and
// the final node's `next` is nullptr, which is how we know the chain has ended.
//
// "Singly" linked means each node points only FORWARD (to `next`), never back.
// ============================================================================

#ifndef LIST_H
#define LIST_H

#include "token.h"
#include <stdio.h> // FILE (used by the printing helpers)

// One box in the chain. `struct TokenNode` names itself so that `next` can
// point to the same type -- a self-referential struct, the shape behind every
// linked structure you will build this semester.
typedef struct TokenNode {
    Token token;
    struct TokenNode *next;
} TokenNode;

// The list "header": bookkeeping that lives wherever you declare it (usually on
// the stack). Only the NODES live on the heap. Tracking `tail` and `length`
// lets us append and count in O(1) instead of walking the whole chain.
typedef struct TokenList {
    TokenNode *head;
    TokenNode *tail;
    size_t length;
} TokenList;

// Create an empty list (every field zero). Returned by value -- cheap, and it
// needs no heap of its own.
[[nodiscard]] TokenList list_create(void);

// Free every node AND every token's owned lexeme, then reset the header to
// empty. Safe to call on an already-empty list.
void list_destroy(TokenList *list);

// Insert at the front (O(1)). The list TAKES OWNERSHIP of `token` (and of its
// lexeme). Returns false if a node could not be allocated; on failure the token
// is freed for you, so nothing leaks either way.
bool list_push_front(TokenList *list, Token token);

// Append at the back (O(1), thanks to `tail`). Same ownership rules as above.
bool list_push_back(TokenList *list, Token token);

// Insert `token` immediately after `node` (O(1)). `node` must belong to `list`.
// Same ownership rules as above.
bool list_insert_after(TokenList *list, TokenNode *node, Token token);

// Return the first node whose token has `kind`, or nullptr if there is none.
// O(n): it may walk the whole chain -- this is exactly the cost the perf and
// hyperfine targets let you measure.
[[nodiscard]] TokenNode *list_find_first(const TokenList *list, TokenKind kind);

// Remove and free the first node whose token has `kind`. Returns true if one
// was removed, false if no match was found. O(n) to locate it.
bool list_remove_first(TokenList *list, TokenKind kind);

// Number of nodes. O(1), because we keep the count up to date as we go.
[[nodiscard]] size_t list_length(const TokenList *list);

// Print the list as a readable token stream to `out` (e.g. stdout).
void list_print(const TokenList *list, FILE *out);

// Emit Graphviz DOT so you can SEE the boxes and arrows. `make dot` runs this
// and turns the result into tokens.png.
void list_to_dot(const TokenList *list, FILE *out);

// Check the list's REP INVARIANT -- the rules that must ALWAYS hold if the list
// is well formed:
//   * length equals the number of reachable nodes
//   * tail is the last node, and is nullptr exactly when the list is empty
// Returns true if healthy. Call it in debug builds after you mutate the list to
// catch bookkeeping bugs early. (You will meet this idea formally in a later
// course; here it is just a helpful self-check.)
[[nodiscard]] bool list_check_invariant(const TokenList *list);

#endif // LIST_H
