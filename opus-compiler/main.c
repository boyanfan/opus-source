// main.c
//
// Created by fanboyan on 2025/1/16.
//

#include <stdio.h>
#include "lexer.h"

int main() {
    FILE *sourceCode = openOpusSourceCode("../tests/arithmetic.opus");
    if (sourceCode) printf("Hello Opus!");
    return 0;
}