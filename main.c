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
        gen(code[i]);
    }

    return 0;
}