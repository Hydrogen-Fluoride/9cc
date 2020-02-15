#include "9cc.h"

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

bool is_reserved1(char *p)
{
    return *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';' || *p == '{' || *p == '}' || *p == ',';
}

bool is_reserved2(char *p)
{
    return startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=");
}

bool is_alnum(char c)
{
    return (c >= 'a' && c <= 'z') || (c >='A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_');
}

bool is_reservedn(char *p, char *q)
{
    return strncmp(p, q, strlen(q)) == 0 && !is_alnum(p[strlen(q)]);
}

Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (is_reserved2(p))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (is_reserved1(p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        if (is_reservedn(p, "return"))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (is_reservedn(p, "if"))
        {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if (is_reservedn(p, "else"))
        {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }
        
        if (is_reservedn(p, "while"))
        {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if (is_reservedn(p, "for"))
        {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        if (*p >= 'a' && *p <= 'z')
        {
            char *tmp = p;
            int cnt = 0;
            while (!(isspace(*p) || is_reserved2(p) || is_reserved1(p)))
            {
                p++;
            }
            cur = new_token(TK_IDENT, cur, tmp, p - tmp);
            continue;
        }
        
        error_at(cur->str + 1, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}