// parser.c 
//
// Created by Boyan Fan, 2025/03/02 
//

#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

ASTNode* initASTNode(ASTNodeType nodeType, Token token) {
    // Try to allocate memory for an AST node, if memory allocation fails, return an empty node
    ASTNode *node = (ASTNode*) malloc(sizeof(ASTNode));
    if (!node) return node;
    
    node->nodeType = nodeType;
    node->token = token;
    node->left = NULL;
    node->right = NULL;

    return node;
}

void freeAST(ASTNode* node) {
    // Returun if there is no more node needed to be free
    if (!node) return;

    // Free the current node and its child nodes
    freeAST(node->left);
    freeAST(node->right);
    free(node);
}

void displayAST(ASTNode* node, int level) {
    // Return if there is no more node needed to be displayed
    if (!node) return;

    // Print indentation with box-drawing characters for a better format
    for (int i = 0; i < level - 1; i++) printf("    ");
    if (level > 0) printf("├── ");

    // Dsiplay the node
    switch (node->nodeType) {
        case AST_PROGRAM: printf("AST_PROGRAM\n"); break;
        case AST_VARIABLE_DECLARATION: printf("AST_VARIABLE_DECLARATION (%s)\n", node->token.lexeme); break;
        case AST_ASSIGNMENT: printf("AST_ASSIGNMENT (%s)\n", node->token.lexeme); break;
        case AST_ARITHMETIC_EXPRESSION: printf("AST_ARITHMETIC_EXPRESSION (%s)\n", node->token.lexeme); break;
        case AST_BOOLEAN_EXPRESSION: printf("AST_BOOLEAN_EXPRESSION (%s)\n", node->token.lexeme); break;
        case AST_CONDITIONAL: printf("AST_CONDITIONAL (if)\n"); break;
        case AST_FUNCTION_CALL: printf("AST_FUNCTION_CALL (%s)\n", node->token.lexeme); break;
        case AST_FORIN_LOOP: printf("AST_FORIN_LOOP (for %s)\n", node->token.lexeme); break;
        case AST_IO: printf("AST_IO (%s)\n", node->token.lexeme); break;
        case AST_REPEAT_UNTIL: printf("AST_REPEAT_UNTIL\n"); break;
        default: printf("UNKNOWN NODE\n"); break;
    }

    if (node->left) displayAST(node->left, level + 1);
    if (node->right) displayAST(node->right, level);
}
