#include "9cc.h"

int labelnum = 0;

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error("代入の左辺値が変数ではありません");
    }
    
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node *node)
{
    int lelse, lend, lbegin;
    if (node == NULL)
    {
        return;
    }
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", lend = labelnum++);
        gen(node->lhs);
        printf(".Lend%d:\n", lend);
        return;
    case ND_IFELSE:
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%d\n", lelse = labelnum++);
        gen(node->lhs);
        printf("    jmp .Lend%d\n", lend = labelnum++);
        printf(".Lelse%d:\n", lelse);
        gen(node->rhs);
        printf(".Lend%d:\n", lend);
        return;
    case ND_WHILE:
        printf(".Lbegin%d:\n", lbegin = labelnum++);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", lend = labelnum++);
        gen(node->lhs);
        printf("    jmp .Lbegin%d\n", lbegin);
        printf(".Lend%d:\n", lend);
        return;
    case ND_FOR:
        gen(node->init);
        printf(".Lbegin%d:\n", lbegin = labelnum++);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", lend = labelnum++);
        gen(node->lhs);
        gen(node->update);
        printf("    jmp .Lbegin%d\n", lbegin);
        printf(".Lend%d:\n", lend);
        return;
    case ND_BLOCK:
        for (int i = 0; node->statement[i] != NULL; i++)
        {
            gen(node->statement[i]);
            printf("    pop rax\n");
        }
        return;
    case ND_FUNC:
        printf("    call %.*s\n", node->func->len, node->func->name);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}