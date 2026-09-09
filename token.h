// ============================================================================
// token.h -- the DOMAIN type our linked list will store.
//
//   A LEXEME is the exact run of characters taken from the source code, such as
//   "route", "->", "42", or "gateway".
//   A TOKEN wraps a lexeme with a KIND (which category it is) and its position
//   (line and column) in the source. A "lexer" turns raw source text into a
//   stream of tokens; this lab builds the list that holds that stream.
//
// You are NEW to C, so this file is commented heavily on purpose. Later labs
// will explain less -- the goal now is to make the workflow and the language
// feel familiar.
// ============================================================================

#ifndef TOKEN_H // include guard: stops this header being pasted in
#define TOKEN_H // twice if it is #included from several files.

// An `enum` is a named set of integer constants. Rather than sprinkling magic
// numbers (0, 1, 2, ...) through the code, we give each token category a
// readable name. In C23 we may pin the underlying type with `: int`, so the
// size is explicit and portable.
enum TokenKind : int {
    // --- Belize language keywords (reserved words) ---
    TOK_LET,
    TOK_CONST,
    TOK_FN,
    TOK_RETURN,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_ROUTE,
    TOK_FAULT,
    TOK_SCHEDULE,
    TOK_NODE,
    TOK_LINK,

    // --- literals and names ---
    TOK_IDENT,  // an identifier: a name the programmer chose, e.g. `gateway`
    TOK_NUMBER, // a numeric literal, e.g. `42`
    TOK_STRING, // a string literal, e.g. "hello"

    // --- operators and punctuation ---
    TOK_ARROW, // ->
    TOK_LPAREN,
    TOK_RPAREN, // ( )
    TOK_LBRACE,
    TOK_RBRACE,    // { }
    TOK_SEMICOLON, // ;

    // --- housekeeping ---
    TOK_EOF,     // end of input -- a lexer emits this last
    TOK_UNKNOWN, // a character we did not recognise
};
typedef enum TokenKind TokenKind; // so we can write `TokenKind` not
                                  // `enum TokenKind` everywhere.

// A Token is a small value. The `lexeme` points to a HEAP-allocated copy of the
// text that this token OWNS: whoever holds the token is responsible for freeing
// that string exactly once. This ownership rule is the heart of the lab -- get
// it wrong and Valgrind / AddressSanitizer will tell you.
typedef struct Token {
    TokenKind kind;
    char *lexeme; // owned heap string (released by token_free)
    int line;     // 1-based line in the source
    int col;      // 1-based column in the source
} Token;

// Build a token. `text` is COPIED onto the heap, so the caller keeps ownership
// of whatever it passed in. `[[nodiscard]]` (a C23 attribute) means "do not
// ignore the return value" -- the compiler warns if you drop the token on the
// floor and leak its lexeme.
[[nodiscard]] Token token_make(TokenKind kind, const char *text, int line, int col);

// Free the owned lexeme and blank the token, so a stale pointer cannot be
// reused by accident. Safe to call on a NULL pointer.
void token_free(Token *token);

// A short, human-readable name for a kind ("ROUTE", "ARROW", ...). Used when
// printing the list and when drawing it with Graphviz.
const char *token_kind_name(TokenKind kind);

#endif // TOKEN_H
