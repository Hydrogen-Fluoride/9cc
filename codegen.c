#include "9cc.h"

int labelnum = 0;
char *rg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *srg[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

void gen_lval(Node *node)
{
    if (node->kind == ND_DEREF)
    {
        gen(node->lhs);
        return;
    }

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
    int rgcount = -1;

    if (!node)
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
        if (node->type->ty == PTR)
        {
            printf("    mov rax, [rax]\n");
        }
        else if (node->type->ty == INT)
        {
            printf("    mov eax, DWORD PTR [rax]\n");
        }
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        if (node->lhs->type->ty != node->rhs->type->ty)
        {
            error("型不一致");
        }
        if (node->lhs->type->ty == PTR)
        {
            printf("    mov [rax], rdi\n");
        }
        else if (node->lhs->type->ty == INT)
        {
            printf("    mov DWORD PTR [rax], edi\n");
        }
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
        for (int i = 0; i < 6 && node->arg[i]; i++)
        {
            gen(node->arg[i]);
            rgcount = i;
        }
        for (int i = rgcount; i >= 0; i--)
        {
            printf("    pop %s\n", rg[i]);
        }
        // rspを16の倍数にする
        printf("    call %.*s\n", node->funclen, node->funcname);
        printf("    push rax\n");
        return;
    case ND_FUNCDEF:
        printf("%.*s:\n", node->funclen, node->funcname);
        // プロローグ
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, %d\n", node->offset);
        for (int i = 0; node->arg[i]; i++)
        {
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n", node->arg[i]->offset);
            if (node->arg[i]->type->ty == PTR)
            {
                printf("    mov [rax], %s\n", rg[i]);
            }
            else if (node->arg[i]->type->ty == INT)
            {
                printf("    mov DWORD PTR [rax], %s\n", srg[i]);
            }
        }
        for (int i = 0; node->statement[i]; i++)
        {
            gen(node->statement[i]);
            printf("    pop rax\n");
        }
        // エピローグ
        printf("    mov rsp, rbp\n");
        printf("	pop rbp\n");
        printf("	ret\n");
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
        Type *type = calloc(1, sizeof(Type));
        type->ptr_to = node->type;
        type->ty = PTR;
        return;
    case ND_DEREF:
        gen(node->lhs);
        if (node->lhs->type->ty != PTR)
        {
            error("*をポインタ型以外に適用している\n");
        }
        printf("    pop rax\n");
        if (node->lhs->type->ptr_to->ty == PTR)
        {
            printf("    mov rax, [rax]\n");
        }
        else if (node->lhs->type->ptr_to->ty == INT)
        {
            printf("    mov eax, DWORD PTR [rax]\n");
        }
        printf("    push rax\n");
        return;
    }
    
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        if (node->lhs->type->ty == PTR && node->rhs->type->ty == PTR)
        {
            error("加算の両方がポインタ\n");
        }
        else if (node->lhs->type->ty == PTR)
        {
            printf("    sal rdi, %d\n", (node->lhs->type->ptr_to->ty == PTR) ? 3 : 2);
        }
        else if (node->rhs->type->ty == PTR)
        {
            printf("    sal rax, %d\n", (node->rhs->type->ptr_to->ty == PTR) ? 3 : 2);
        }
        printf("    add rax, rdi\n");
        break; 
    case ND_SUB:
        if (node->lhs->type->ty == PTR && node->rhs->type->ty == PTR)
        {
            printf("    sub rax, rdi\n");
            printf("    sar rax, %d\n", (node->lhs->type->ptr_to->ty == PTR) ? 3 : 2);
        }
        else if (node->lhs->type->ty == PTR)
        {
            printf("    sal rdi, %d\n", (node->lhs->type->ptr_to->ty == PTR) ? 3 : 2);
            printf("    sub rax, rdi\n");
        }
        else if (node->rhs->type->ty == PTR)
        {
            error("減算の右側のみポインタ\n");
        }
        else
        {
            printf("    sub rax, rdi\n");
        }
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