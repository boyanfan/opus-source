// parser.c 
//
// Created by Boyan Fan, 2025/03/02 
//

#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

ASTNode *parseProgram(Parser *parser, FILE *sourceCode) {
    ASTNode *root = initASTNode(AST_PROGRAM, NULL);
    ASTNode *currentNode = root;

    while (!matchTokenType(parser, TOKEN_EOF)) {
        // Skip orphan delimiters like (delimiter delimiter...)
        if (parser->currentToken->tokenType == TOKEN_DELIMITER) {
            parser->currentToken = advanceParser(parser, sourceCode);
            continue;
        }

        currentNode->left = parseStatement(parser, sourceCode);

        if (!matchTokenType(parser, TOKEN_EOF)) {
            currentNode->right = initASTNode(AST_PROGRAM, NULL);
            currentNode = currentNode->right;
        }

        // Must explicitly handle the case that there is no delimiter at the end of the file 
        // Since Opus uses newline character as a delimiter and in some cases, there might be no 
        // Newline character at the end of the file, but only a EOF, so in this case,
        // Treat the EOF as a delimiter to complete the AST structure (avoid NULL right node)
        else currentNode->right = initASTNode(AST_PROGRAM, NULL);
    }

    return root;
}

ASTNode *parseStatement(Parser *parser, FILE *sourceCode) {
    // Try to parse declaration statement
    if (matchTokenType(parser, TOKEN_KEYWORD_VAR) || matchTokenType(parser, TOKEN_KEYWORD_LET)) { 
        return parseVariableDeclaration(parser, sourceCode);
    }

    // If unable to parse the statement
    parser->parseError = PARSE_ERROR_UNRESOLVABLE;
    reportParseError(parser);

    exit(1);
}

ASTNode *parseVariableDeclaration(Parser *parser, FILE *sourceCode) {
    // Create a node for the variable declaration statement
    ASTNode *root = (parser->currentToken->tokenType == TOKEN_KEYWORD_VAR)
        ? initASTNode(AST_VARIABLE_DECLARATION, parser->currentToken)
        : initASTNode(AST_CONSTANT_DECLARATION, parser->currentToken);
    
    // Consume the current keyword token 'var' or 'let'
    parser->currentToken = advanceParser(parser, sourceCode);

    // Then try to parse the identifier
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_IDENTIFIER;
        reportParseError(parser);
        exit(1);
    }

    // Create a node for the identifier
    ASTNode *identifierNode = initASTNode(AST_IDENTIFIER, parser->currentToken);

    // Consume the current identifier token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match colon 
    if (!matchTokenType(parser, TOKEN_COLON)) {
        parser->parseError = PARSE_ERROR_MISSING_TYPE_ANNOTATION;
        reportParseError(parser);
        exit(1);
    }

    // Consume the current colon token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the type identifier
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_TYPE_NAME;
        reportParseError(parser);
        exit(1);
    }

    // Create a node for the type annotation
    ASTNode *typeAnnotationNode = initASTNode(AST_TYPE_ANNOTATION, parser->currentToken);

    // Create the AST for the variable declaration statement
    root->left = identifierNode;
    root->right = typeAnnotationNode;

    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the delimiter
    if (!matchTokenType(parser, TOKEN_DELIMITER)) {
        parser->parseError = PARSE_ERROR_MISSING_DELIMITER;
        reportParseError(parser);
        exit(1);
    }
    
    // Comsume the current delimiter token
    parser->currentToken = advanceParser(parser, sourceCode);
    return root;
}

int matchTokenType(Parser *parser, TokenType type) { return parser->currentToken->tokenType == type; }

Token *advanceParser(Parser *parser, FILE *sourceCode) { return getNextToken(parser->lexer, sourceCode); } 

Parser *initParser() {
    // Allocate memory for a Parser instance and return NULL if memory allocation failed 
    Parser *parser = (Parser*) malloc(sizeof(Parser));
    if (!parser) return NULL;

    // Allocate memory for a Lexer instance and return NULL if memory allocation failed 
    Lexer *lexer = initLexer();
    if (!lexer) return NULL;

    parser->parseError = PARSE_ERROR_NONE;
    parser->lexer = lexer;
    parser->currentToken = NULL;

    return parser;
}

ASTNode* initASTNode(ASTNodeType nodeType, Token *token) {
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

    // Display the node
    switch (node->nodeType) {
        case AST_PROGRAM:                printf("AST_PROGRAM\n"); break;
        case AST_VARIABLE_DECLARATION:   printf("AST_VARIABLE_DECLARATION (%s)\n", node->token->lexeme); break;
        case AST_CONSTANT_DECLARATION:   printf("AST_CONSTANT_DECLARATION (%s)\n", node->token->lexeme); break;
        case AST_IDENTIFIER:             printf("AST_IDENTIFIER (%s)\n", node->token->lexeme); break;
        case AST_TYPE_ANNOTATION:        printf("AST_TYPE_ANNOTATION (%s)\n", node->token->lexeme); break;
        case AST_ASSIGNMENT_STATEMENT:   printf("AST_ASSIGNMENT (%s)\n", node->token->lexeme); break;
        default:                         printf("UNKNOWN NODE\n"); break;
    }

    if (node->left) displayAST(node->left, level + 1);
    if (node->right) displayAST(node->right, level + 1);
}

void reportParseError(Parser *parser) {
    Token *currentToken = parser->currentToken;
    printf("Parsing Error at %d:%d\n", currentToken->location.line, currentToken->location.column);

    // Return if there is no error to display
    if (parser->parseError == PARSE_ERROR_NONE) return;

    switch (parser->parseError) {
        case PARSE_ERROR_MISSING_IDENTIFIER:
            printf("[ERROR] Expecting an identifier for the variable after %s\n", currentToken->lexeme); break;
        case PARSE_ERROR_MISSING_TYPE_ANNOTATION:
            printf("[ERROR] Expecting \":\" after %s\n", currentToken->lexeme); break;
        case PARSE_ERROR_MISSING_TYPE_NAME:
            printf("[ERROR] Expecting a type name after %s\n", currentToken->lexeme); break;
        case PARSE_ERROR_UNRESOLVABLE:
            printf("[ERROR] Unresolvable token after %s\n", currentToken->lexeme); break;
        default:
            printf("[ERROR] Unable to generate diagnostic information...\n");
    }
}

