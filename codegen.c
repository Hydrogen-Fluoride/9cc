#include "9cc.h"

int labelnum = 0;
char *rg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *srg[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

bool isleft = false;
bool lptr = false;
bool rptr = false;
bool ptr = false;

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

    if (!node)
    {
        return;
    }
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        if (isleft)
        {
            lptr = false;
        }
        else
        {
            rptr = false;
        }
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        if (node->type->ty == PTR)
        {
            printf("    mov rax, [rax]\n");
            if (isleft)
            {
                lptr = true;
            }
            else
            {
                rptr = true;
            }
            ptr = (node->type->ptr_to->ty == PTR);
        }
        else
        {
            printf("    mov eax, DWORD PTR [rax]\n");
            if (isleft)
            {
                lptr = false;
            }
            else
            {
                rptr = false;
            }
        }
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rbx\n");
        printf("    pop rax\n");
        if (node->lhs->type->ty == PTR)
        {
            printf("    mov [rax], rbx\n");
        }
        else
        {
            printf("    mov DWORD PTR [rax], ebx\n");
        }
        printf("    push rbx\n");
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
            printf("    pop rax\n");
            printf("    mov %s, rax\n", rg[i]);
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
            else
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
        return;
    case ND_DEREF:
        gen(node->lhs);
        printf("    pop rax\n");
        if (node->lhs->type->ty == PTR)
        {
            printf("    mov rax, [rax]\n");
            ptr = true;
        }
        else
        {
            printf("    mov eax, DWORD PTR [rax]\n");
            ptr = false;
        }
        printf("    push rax\n");
        return;
    }

    bool t_lptr = lptr;
    bool t_rptr = rptr;
    bool t_ptr = ptr;

    bool t_isleft = isleft;
    isleft = true;
    gen(node->lhs);
    isleft = false;
    gen(node->rhs);
    isleft = t_isleft;

    printf("    pop rbx\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        if (lptr && rptr)
        {
            error("加算の両方がポインタ\n");
        }
        else if (lptr || rptr)
        {
            printf("    sal %s, %d\n", lptr ? "rbx" : "rax", ptr ? 3 : 2);
            if (isleft)
            {
                lptr = true;
                rptr = t_rptr;
            }
            else
            {
                lptr = t_lptr;
                rptr = true;
            }
        }
        else
        {
            if (isleft)
            {
                lptr = false;
                rptr = t_rptr;
            }
            else
            {
                lptr = t_lptr;
                rptr = false;
            }
        }
        printf("    add rax, rbx\n");
        break;
    case ND_SUB:
        if (lptr && rptr)
        {
            printf("    sub rax, rbx\n");
            printf("    sar rax, %d\n", ptr ? 3 : 2);
            if (isleft)
            {
                lptr = false;
                rptr = t_rptr;
            }
            else
            {
                lptr = t_lptr;
                rptr = false;
            }
        }
        else if (lptr)
        {
            printf("    sal rbx, %d\n", ptr ? 3 : 2);
            printf("    sub rax, rbx\n");
            if (isleft)
            {
                lptr = true;
                rptr = t_rptr;
            }
            else
            {
                lptr = t_lptr;
                rptr = true;
            }
        }
        else if (rptr)
        {
            error("減算の右側のみポインタ\n");
        }
        else
        {
            printf("    sub rax, rbx\n");
            if (isleft)
            {
                lptr = false;
                rptr = t_rptr;
            }
            else
            {
                lptr = t_lptr;
                rptr = false;
            }
        }
        ptr = t_ptr;
        break;
    case ND_MUL:
        printf("    imul rax, rbx\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rbx\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rbx\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rbx\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rbx\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rbx\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}