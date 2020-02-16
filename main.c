#include "9cc.h"

char *user_input;
Token *token;
Node *code[100];

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    for (int i = 0; code[i]; i++)
    {
        printf("%.*s:\n", code[i]->func->len, code[i]->func->name);

        // プロローグ
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, %d\n", code[i]->offset);

        for (int j = 0; code[i]->arg[j]; j++)
        {
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n", 8 * (2 + j));
            printf("    push %s\n", rg[i]);
            printf("    pop rdi\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
        }
        
        for (int j = 0; code[i]->statement[j]; j++)
        {
            gen(code[i]->statement[j]);
            printf("    pop rax\n");
        }

        // エピローグ
        printf("    mov rsp, rbp\n");
        printf("	pop rbp\n");
        printf("	ret\n");
    }

    return 0;
}