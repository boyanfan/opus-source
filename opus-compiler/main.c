// main.c
//
// Created by fanboyan on 2025/1/16.
//

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
    // Ensure the user provides a file as an argument to compile
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file.opus>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Safely open given Opus source code by using function openOpusSourceCode()
    FILE *sourceCode = openOpusSourceCode(argv[1]);
    printf("Compiling %s...\n", argv[1]);

    Lexer *lexer = initLexer();
    Token *token = getNextToken(lexer, sourceCode);

    while (token->tokenType != TOKEN_EOF) {
        displayToken(*token);
        token = getNextToken(lexer, sourceCode);
    }

    // Close file after compiling the source code
    fclose(sourceCode);
    return EXIT_SUCCESS;
}
