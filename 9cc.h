#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_INT,
    TK_SIZEOF
} TokenKind;

typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_ASSIGN,
    ND_LVAR,
    ND_NUM,
    ND_RETURN,
    ND_IF,
    ND_IFELSE,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_FUNC,
    ND_FUNCDEF,
    ND_ADDR,
    ND_DEREF,
    ND_GVAR
} NodeKind;

typedef struct Token Token;
struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

typedef struct Type Type;
struct Type
{
    enum { INT, PTR, ARRAY } ty;
    Type *ptr_to;
    size_t array_size;
};


typedef struct LVar LVar;
struct LVar
{
    LVar *next;
    char *name;
    int len;
    int offset;
    Type *type;
};

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    Node *init;
    Node *cond;
    Node *update;
    Node *statement[100];
    int val;
    int offset;
    Type *type;
    char *funcname;
    int funclen;
    Node *arg[6];
};

extern Token *token;
extern char *user_input;
extern Node *code[100];

// container.c
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

// tokenize.c
Token *tokenize(char *p);

// codegen.c
void gen(Node *node);

// parse.c
void program();