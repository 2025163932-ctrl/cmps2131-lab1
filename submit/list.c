#include "list.h"
#include <stdlib.h> 

TokenList list_create(void) {

    return (TokenList){.head = nullptr, .tail = nullptr, .length = 0};
}

void list_destroy(TokenList *list) {

    TokenNode *current = list->head;
    while (current != nullptr) {
        TokenNode *next = current->next; 
        token_free(&current->token);     
        free(current);                   
        current = next;
    }
    *list = list_create();
}

bool list_push_front(TokenList *list, Token token) {

    TokenNode *node = malloc(sizeof(TokenNode));
    if (node == nullptr) {
        token_free(&token);
        return false;
    }
    node->token = token;
    node->next  = list->head;
    list->head  = node;
    if (list->tail == nullptr) {
        list->tail = node;
    }
    list->length++;
    return true;
}

bool list_push_back(TokenList *list, Token token) {

    TokenNode *node = malloc(sizeof(TokenNode));
    if (node == nullptr) {
        token_free(&token);
        return false;
    }
    node->token = token;
    node->next  = nullptr;
    if (list->tail == nullptr) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail       = node;
    }
    list->length++;
    return true;
}

bool list_insert_after(TokenList *list, TokenNode *node, Token token) {

    if (node == nullptr) {
        token_free(&token);
        return false;
    }
    TokenNode *new_node = malloc(sizeof(TokenNode));
    if (new_node == nullptr) {
        token_free(&token);
        return false;
    }
    new_node->token = token;
    new_node->next  = node->next;
    node->next      = new_node;
    if (node == list->tail) {
        list->tail = new_node;
    }
    list->length++;
    return true;
}

TokenNode *list_find_first(const TokenList *list, TokenKind kind) {

    TokenNode *current = list->head;
    while (current != nullptr) {
        if (current->token.kind == kind) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

bool list_remove_first(TokenList *list, TokenKind kind) {

    TokenNode *prev    = nullptr;
    TokenNode *current = list->head;

    while (current != nullptr && current->token.kind != kind) {
        prev    = current;
        current = current->next;
    }

    if (current == nullptr) {
        return false; // not found
    }

    // Unlink `current` from the chain.
    if (prev == nullptr) {
        list->head = current->next; // removing the head
    } else {
        prev->next = current->next;
    }

    if (current == list->tail) {
        list->tail = prev; // removing the tail: move tail back
    }

    token_free(&current->token);
    free(current);
    list->length--;
    return true;
}

size_t list_length(const TokenList *list) {

    return list->length;
}

void list_print(const TokenList *list, FILE *out) {

    for (TokenNode *cur = list->head; cur != nullptr; cur = cur->next) {
        fprintf(out, "  %-9s '%s' (%d:%d)\n", token_kind_name(cur->token.kind), cur->token.lexeme,
                cur->token.line, cur->token.col);
    }
}


static void dot_print_escaped(FILE *out, const char *text) {
    for (const char *p = text; *p != '\0'; p++) {
        switch (*p) {
        case '{':
        case '}':
        case '|':
        case '<':
        case '>':
        case '\\':
        case '"':
            fputc('\\', out);
            fputc(*p, out);
            break;
        default: fputc(*p, out); break;
        }
    }
}

void list_to_dot(const TokenList *list, FILE *out) {

    fprintf(out, "digraph tokens {\n");
    fprintf(out, "  rankdir=LR;\n");
    fprintf(out, "  node [shape=record];\n");

    int i = 0;
    for (TokenNode *cur = list->head; cur != nullptr; cur = cur->next, i++) {
        fprintf(out, "  n%d [label=\"{%s|", i, token_kind_name(cur->token.kind));
        dot_print_escaped(out, cur->token.lexeme);
        fprintf(out, "}\"];\n");
        if (cur->next != nullptr) {
            fprintf(out, "  n%d -> n%d;\n", i, i + 1);
        } else {
            fprintf(out, "  n%d -> nnull;\n", i);
        }
    }
    fprintf(out, "  nnull [shape=point];\n");
    fprintf(out, "}\n");
}

bool list_check_invariant(const TokenList *list) {

    size_t count       = 0;
    TokenNode *current = list->head;
    TokenNode *last    = nullptr;

    while (current != nullptr) {
        last = current;
        count++;
        current = current->next;
    }

    if (count != list->length) {
        return false;
    }
    if (list->tail != last) {
        return false;
    }
    if (list->length == 0 && list->tail != nullptr) {
        return false;
    }
    return true;
}
