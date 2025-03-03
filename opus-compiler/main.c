// main.c
//
// Created by fanboyan on 2025/1/16.
//

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char *argv[]) {
    // Ensure the user provides a file as an argument to compile
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file.opus>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Safely open given Opus source code by using function openOpusSourceCode()
    FILE *sourceCode = openOpusSourceCode(argv[1]);
    printf("Compiling...\n");

    // Initialize the Parser and try to generate the AST for the provided sourceCode
    Parser *parser = initParser(); 
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to parse variable declaration
    ASTNode *root = parseProgram(parser, sourceCode);
    displayAST(root, 0);
    freeAST(root);

    // Close the provided sourceCode after parsing
    fclose(sourceCode);
    return EXIT_SUCCESS;
}
