// main.c
//
// Created by fanboyan on 2025/1/16.
//

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "token.h"

int main(int argc, char *argv[]) {
    /* // Ensure the user provides a file as an argument to compile */
    /* if (argc != 2) { */
    /*     fprintf(stderr, "Usage: %s <source_file.opus>\n", argv[0]); */
    /*     return EXIT_FAILURE; */
    /* } */
    /**/
    /* // Safely open given Opus source code by using function openOpusSourceCode() */
    /* FILE *sourceCode = openOpusSourceCode(argv[1]); */
    /* printf("Compiling %s...\n", argv[1]); */
    /**/
    /* Lexer *lexer = initLexer(); */
    /* Token *token = getNextToken(lexer, sourceCode); */
    /**/
    /* while (token->tokenType != TOKEN_EOF) { */
    /*     displayToken(*token); */
    /*     token = getNextToken(lexer, sourceCode); */
    /* } */
    /**/
    /* // Close file after compiling the source code */
    /* fclose(sourceCode); */

    // Create tokens for each AST node
    Token programToken = { .tokenType = TOKEN_EOF, .lexeme = "program" };
    Token varDeclToken = { .tokenType = TOKEN_KEYWORD_VAR, .lexeme = "x" };
    Token ifToken = { .tokenType = TOKEN_KEYWORD_IF, .lexeme = "if" };
    Token boolExprToken = { .tokenType = TOKEN_GREATER_THAN_OPERATOR, .lexeme = "x > 5" };
    Token assignIfToken = { .tokenType = TOKEN_ASSIGNMENT_OPERATOR, .lexeme = "x = x + 1" };
    Token assignElseToken = { .tokenType = TOKEN_ASSIGNMENT_OPERATOR, .lexeme = "x = 0" };

    // Create AST nodes
    ASTNode* root = initASTNode(AST_PROGRAM, programToken);

    // Variable Declaration: "var x: Int"
    ASTNode* varDecl = initASTNode(AST_VARIABLE_DECLARATION, varDeclToken);

    // Conditional Statement: "if (x > 5)"
    ASTNode* ifStmt = initASTNode(AST_CONDITIONAL, ifToken);
    
    // Boolean Expression: "x > 5"
    ASTNode* boolExpr = initASTNode(AST_BOOLEAN_EXPRESSION, boolExprToken);

    // Assignment in If Block: "x = x + 1"
    ASTNode* assignIf = initASTNode(AST_ASSIGNMENT, assignIfToken);

    // Assignment in Else Block: "x = 0"
    ASTNode* assignElse = initASTNode(AST_ASSIGNMENT, assignElseToken);

    // Build AST hierarchy
    root->left = varDecl;       // First statement is var decl
    varDecl->right = ifStmt;    // Followed by if statement

    ifStmt->left = boolExpr;    // If condition
    boolExpr->right = assignIf; // If block assignment
    assignIf->right = assignElse; // Else block assignment

    // Display the AST structure
    displayAST(root, 0);

    // Free memory after debugging
    freeAST(root);
    return EXIT_SUCCESS;
}
