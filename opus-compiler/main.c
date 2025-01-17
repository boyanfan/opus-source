// main.c
//
// Created by fanboyan on 2025/1/16.
//

#include <stdio.h>
#include "lexer.h"

int main() {
    FILE *sourceCode = openOpusSourceCode("../tests/arithmetic.opus");
    if (sourceCode) printf("Hello Opus!\n");

    Lexer *lexer = initLexer(sourceCode);
    printf("%d, %d:%d", lexer->lexerError, lexer->location.line, lexer->location.column);

    return 0;
}