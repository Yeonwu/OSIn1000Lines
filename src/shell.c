//
// Created by ohye on 25. 3. 4.
//
#include "user.h"

#define PROMPT_MAX_LEN 10
#define PROMPT_SUCCESS 0
#define PROMPT_TOO_LONG 1

int get_prompt(char* prompt, int max_length) {
    int i;
    for (i = 0; i < max_length; i++) {
        char ch = getchar();
        if (ch == '\r') {
            putchar('\n');
            prompt[i] = '\0';
            break;
        } else if (ch == 127) {
            i--;
            putchar('\b');
            prompt[i] = 0;
        } else {
            putchar(ch);
            prompt[i] = ch;
        }
    }

    if (i == max_length) {
        putchar('\n');
        return PROMPT_TOO_LONG;
    }

    return PROMPT_SUCCESS;
}

void main(void) {
    while(1) {
        putchar('>');
        char prompt[PROMPT_MAX_LEN];
        int result = get_prompt(prompt, PROMPT_MAX_LEN);
        if (result == PROMPT_SUCCESS) {
            if (strcmp(prompt, "say hello") == 0) {
                printf("Hello world from shell!\n");
            } else if (strcmp(prompt, "exit") == 0) {
                printf("Goodbye\n");
                exit();
            } else {
                printf("unknown prompt: %s\n", prompt);
            }
        } else {
            switch (result) {
                case PROMPT_TOO_LONG:
                    printf("prompt too long\n");
                    break;
                default:
                    printf("unknown error while reading prompt\n");
                    break;
            }
        }
    }
}