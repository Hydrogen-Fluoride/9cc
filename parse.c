#include "9cc.h"

LVar *locals;

Node* array_to_ptr(Node *node)
{
    if (node->type->ty != ARRAY)
    {
        return node;
    }

    Type *type = calloc(1, sizeof(Type));
    type->ty = PTR;
    type->ptr_to = node->type->ptr_to;
    node->type = type;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    if (node->lhs->type->ty == node->rhs->type->ty)
    {
        node->type = calloc(1, sizeof(Type));
        node->type->ty = INT;
    }
    else if (node->lhs->type->ty == PTR || node->lhs->type->ty == ARRAY)
    {
        node->type = node->lhs->type;
    }
    else
    {
        node->type = node->rhs->type;
    }
    return array_to_ptr(node);
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    node->type = calloc(1, sizeof(Type));
    node->type->ty = INT;
    return node;
}

bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident()
{
    if (token->kind != TK_IDENT)
    {
        return NULL;
    }
    Token *ret = token;
    token = token->next;
    return ret;
}

bool consume_token(TokenKind tk)
{
    if (token->kind != tk)
    {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        error_at(token->str, "\"%s\"ではありません", op);
    }
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
    {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
        {
            return var;
        }
    }
    return NULL;
}

int calc_offset(Type* type)
{
    switch (type->ty)
    {
    case PTR:
        return 8;
    case INT:
        return 4;
    case ARRAY:
        return calc_offset(type->ptr_to) * type->array_size;
    }
}

Node *func();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void program()
{
    int i = 0;
    while (!at_eof())
    {
        code[i++] = func();
    }
    code[i] = NULL;
}

Node *func()
{
    locals = NULL;
    if (!consume_token(TK_INT))
    {
        error_at(token->str, "intではありません");
    }
    Token *tok = consume_ident();
    if (!tok)
    {
        error_at(token->str, "関数ではありません");
    }
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNCDEF;
    node->type = calloc(1, sizeof(Type));
    node->type->ty = INT;
    node->funcname = tok->str;
    node->funclen = tok->len;
    expect("(");
    int i = 0;
    while (true)
    {
        if (consume(")"))
        {
            break;
        }
        if (!consume_token(TK_INT))
        {
            error_at(token->str, "intではありません");
        }
        Type *type = calloc(1, sizeof(Type));
        type->ty = INT;
        while (consume("*"))
        {
            Type *newtype = calloc(1, sizeof(Type));
            newtype->ptr_to = type;
            newtype->ty = PTR;
            type = newtype;
        }
        node->arg[i] = calloc(1, sizeof(Node));
        node->arg[i]->kind = ND_LVAR;
        tok = consume_ident();
        if (!tok)
        {
            error_at(token->str, "引数ではありません");
        }
        LVar *lvar = find_lvar(tok);
        if (lvar)
        {
            error_at(token->str, "引数名が重複しています");
        }
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->type = type;
        int off = calc_offset(type);
        lvar->offset = locals ? (locals->offset + off) : off;
        node->arg[i]->type = lvar->type;
        node->arg[i]->offset = lvar->offset;
        locals = lvar;
        i++;
        if (consume(")"))
        {
            break;
        }
        expect(",");
    }
    node->arg[i] = NULL;
    expect("{");
    i = 0;
    while (!consume("}"))
    {
        node->statement[i++] = stmt();
    }
    node->statement[i] = NULL;
    node->offset = locals ? locals->offset : 0;
    return node;
}

Node *stmt()
{
    Node *node;
    if (consume_token(TK_INT))
    {
        Type *type = calloc(1, sizeof(Type));
        type->ty = INT;
        while (consume("*"))
        {
            Type *newtype = calloc(1, sizeof(Type));
            newtype->ptr_to = type;
            newtype->ty = PTR;
            type = newtype;
        }
        Token *tok = consume_ident();
        if (!tok)
        {
            error_at(token->str, "変数ではありません");
        }
        while (consume("["))
        {
            Type *newtype = calloc(1, sizeof(Type));
            newtype->ty = ARRAY;
            newtype->ptr_to = type;
            newtype->array_size = expect_number();
            type = newtype;
            expect("]");
        }
        LVar *lvar = find_lvar(tok);
        if (lvar)
        {
            error_at(token->str, "既に定義されている変数です");
        }
        else
        {
            node = calloc(1, sizeof(Node));
            node->kind = ND_LVAR;
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->type = type;
            int off = calc_offset(type);
            lvar->offset = locals ? (locals->offset + off) : off;
            node->type = type;
            node->offset = lvar->offset;
            locals = lvar;
            consume(";");
        }
    }
    else if (consume_token(TK_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
    }
    else if (consume_token(TK_IF))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->cond = expr();
        expect(")");
        node->lhs = stmt();
        if (consume_token(TK_ELSE))
        {
            node->kind = ND_IFELSE;
            node->rhs = stmt();
        }
    }
    else if (consume_token(TK_WHILE))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->cond = expr();
        expect(")");
        node->lhs = stmt();
    }
    else if (consume_token(TK_FOR))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        if (!consume(";"))
        {
            node->init = expr();
            expect(";");
        }
        if (!consume(";"))
        {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")"))
        {
            node->update = expr();
            expect(")");
        }
        node->lhs = stmt();
    }
    else if (consume("{"))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        int i = 0;
        while (!consume("}"))
        {
            node->statement[i++] = stmt();
        }
        node->statement[i] = NULL;
    }
    else
    {
        node = expr();
        expect(";");
    }
    return node;
}

Node *expr()
{
    return assign();
}

Node *assign()
{
    Node *node = equality();
    if (consume("="))
    {
        return new_binary(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
        {
            node = new_binary(ND_EQ, node, relational());
        }
        else if (consume("!="))
        {
            node = new_binary(ND_NE, node, relational());
        }
        else
        {
            return node;
        }
    }
}

Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume("<"))
        {
            node = new_binary(ND_LT, node, add());
        }
        else if (consume("<="))
        {
            node = new_binary(ND_LE, node, add());
        }
        else if (consume(">"))
        {
            node = new_binary(ND_LT, add(), node);
        }
        else if (consume(">="))
        {
            node = new_binary(ND_LE, add(), node);
        }
        else
        {
            return node;
        }
    }
}

Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
        {
            node = new_binary(ND_ADD, node, mul());
        }
        else if (consume("-"))
        {
            node = new_binary(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}

Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
        {
            node = new_binary(ND_MUL, node, unary());
        }
        else if (consume("/"))
        {
            node = new_binary(ND_DIV, node, unary());
        }
        else
        {
            return node;
        }
    }
}

Node *unary()
{
    if (consume_token(TK_SIZEOF))
    {
        return new_node_num(calc_offset(unary()->type));
    }
    if (consume("+"))
    {
        return primary();
    }
    if (consume("-"))
    {
        return new_binary(ND_SUB, new_node_num(0), primary());
    }
    if (consume("&"))
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_ADDR;
        node->lhs = unary();
        node->type = calloc(1, sizeof(Type));
        node->type->ty = PTR;
        node->type->ptr_to = node->lhs->type;
        return node;
    }
    if (consume("*"))
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->lhs = unary();
        node->type = node->lhs->type->ptr_to;
        return array_to_ptr(node);
    }
    return primary();
}

Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    
    Token *tok = consume_ident();
    if (tok)
    {
        Node *node = calloc(1, sizeof(Node));
        if (consume("("))
        {
            node->kind = ND_FUNC;
            node->funcname = tok->str;
            node->funclen = tok->len;
            int i = 0;
            while (true)
            {
                if (consume(")"))
                {
                    break;
                }
                node->arg[i] = expr();
                i++;
                if (consume(")"))
                {
                    break;
                }
                expect(",");
            }
            node->arg[i] = NULL;

            node->type = calloc(1, sizeof(Type));
            node->type->ty = INT;
        }
        else
        {
            node->kind = ND_LVAR;
            LVar *lvar = find_lvar(tok);
            if (lvar)
            {
                node->offset = lvar->offset;
                node->type = lvar->type;
            }
            else
            {
                error_at(token->str, "宣言されていない変数です");
            }
        }
        return node;
    }
    
    return new_node_num(expect_number());
}
