#include "9cc.h"

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
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

Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}

Node *unary()
{
    if (consume("+"))
    {
        return primary();
    }
    if (consume("-"))
    {
        return new_binary(ND_SUB, new_node_num(0), primary());
    }
    return primary();
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

Node *expr()
{
    return equality();
}