// ============================================================================
// token.c -- implementation of the Token domain type.
// ============================================================================

#include "token.h"

#include <stdlib.h> // malloc, free
#include <string.h> // strlen, memcpy

// Make an owned heap copy of a C string (this is what the library's strdup
// does). We write it by hand so the malloc you must LATER free is right in
// front of you. Returns nullptr if the heap is exhausted.
static char *own_string(const char *s) {
    size_t len = strlen(s) + 1; // +1 leaves room for the terminating '\0'
    char *copy = malloc(len);
    if (copy == nullptr) { // ALWAYS check the result of malloc
        return nullptr;
    }
    memcpy(copy, s, len); // copy the bytes, including the '\0'
    return copy;
}

Token token_make(TokenKind kind, const char *text, int line, int col) {
    // If text is nullptr we store an empty owned string, so callers never have
    // to worry about a null lexeme.
    char *owned = own_string(text != nullptr ? text : "");

    // A designated initializer: each field is set by name, which reads clearly
    // and guarantees nothing is left uninitialised.
    Token token = {
        .kind   = kind,
        .lexeme = owned, // may be nullptr if own_string ran out of memory
        .line   = line,
        .col    = col,
    };
    return token; // returned BY VALUE; ownership of `owned` travels with it
}

void token_free(Token *token) {
    if (token == nullptr) {
        return;
    }
    free(token->lexeme);         // release the owned string...
    token->lexeme = nullptr;     // ...then null it, so a later use is an obvious
    token->kind   = TOK_UNKNOWN; // bug rather than a silent read of freed memory
}

const char *token_kind_name(TokenKind kind) {
    // A `switch` over every enum value. Because there is no `default:` case, the
    // compiler (with -Wall) warns HERE if we later add a new TokenKind and
    // forget to name it -- a safety net you get for free.
    switch (kind) {
    case TOK_LET: return "LET";
    case TOK_CONST: return "CONST";
    case TOK_FN: return "FN";
    case TOK_RETURN: return "RETURN";
    case TOK_IF: return "IF";
    case TOK_ELSE: return "ELSE";
    case TOK_WHILE: return "WHILE";
    case TOK_ROUTE: return "ROUTE";
    case TOK_FAULT: return "FAULT";
    case TOK_SCHEDULE: return "SCHEDULE";
    case TOK_NODE: return "NODE";
    case TOK_LINK: return "LINK";
    case TOK_IDENT: return "IDENT";
    case TOK_NUMBER: return "NUMBER";
    case TOK_STRING: return "STRING";
    case TOK_ARROW: return "ARROW";
    case TOK_LPAREN: return "LPAREN";
    case TOK_RPAREN: return "RPAREN";
    case TOK_LBRACE: return "LBRACE";
    case TOK_RBRACE: return "RBRACE";
    case TOK_SEMICOLON: return "SEMICOLON";
    case TOK_EOF: return "EOF";
    case TOK_UNKNOWN: return "UNKNOWN";
    }
    return "??"; // reached only if `kind` holds a value outside the enum
}
