// parser.c 
//
// Created by Boyan Fan, 2025/03/02 
//

#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "ast.h"

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
        parser->previousToken = root->token;
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
        parser->previousToken = identifierNode->token;
        reportParseError(parser);
        exit(1);
    }

    // Consume the current colon token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the type identifier
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_TYPE_NAME;
        parser->previousToken = parser->currentToken;
        reportParseError(parser);
        exit(1);
    }

    // Create a node for the type annotation
    ASTNode *typeAnnotationNode = initASTNode(AST_TYPE_ANNOTATION, parser->currentToken);

    // Create the AST for the variable declaration statement
    root->left = identifierNode;
    root->right = typeAnnotationNode;

    // Comsume the current type annotation token
    parser->currentToken = advanceParser(parser, sourceCode);
    
    // Try to match a delimiter
    if (matchTokenType(parser, TOKEN_DELIMITER)) {
        // Consume the current delimiter token
        parser->currentToken = advanceParser(parser, sourceCode);
        return root;
    }

    // Try if match assignment operator if there is an assignment statement after the declaration
    if (matchTokenType(parser, TOKEN_ASSIGNMENT_OPERATOR)) { 
        // The root now should be an assignment statement and the declaration is its left value
        return parseAssignmentStatement(parser, sourceCode, root);
    }

    // Explicitly handle missing delimiter case
    if (!matchTokenType(parser, TOKEN_DELIMITER)) {
        parser->parseError = PARSE_ERROR_MISSING_DELIMITER;
        parser->previousToken = typeAnnotationNode->token;
        reportParseError(parser);
        exit(1);
    }
    
    // Report an error if the declaration statement is invalid
    parser->parseError = PARSE_ERROR_DECLARATION_SYNTAX;
    parser->previousToken = typeAnnotationNode->token;
    reportParseError(parser);
    exit(1);
}

ASTNode *parseAssignmentStatement(Parser *parser, FILE *sourceCode, ASTNode *leftValue) {
    ASTNode *root = initASTNode(AST_ASSIGNMENT_STATEMENT, parser->currentToken);

    // Consume the current operator token '='
    parser->currentToken = advanceParser(parser, sourceCode);
    
    // Create the AST for the assignment statement
    root->left = leftValue;
    root->right = parseExpression(parser, sourceCode);

    // Try to match the delimiter
    if (!matchTokenType(parser, TOKEN_DELIMITER)) {
        parser->parseError = PARSE_ERROR_MISSING_DELIMITER;
        parser->previousToken = root->right->token;
        reportParseError(parser);
        exit(1);
    }

    // Comsume the current delimiter token
    parser->currentToken = advanceParser(parser, sourceCode);

    return root;
}

ASTNode *parseExpression(Parser *parser, FILE *sourceCode) {
    // Entry point for expression parsing, we start at the lowest precedence level (logical or)
    return parseLogicalOr(parser, sourceCode);
}

ASTNode *parseLogicalOr(Parser *parser, FILE *sourceCode) {
// Where logical and has higher precedence than the logical or, so try to parse it first
    ASTNode *root = parseLogicalAnd(parser, sourceCode);

    // Try to match logical or 
    while (matchTokenType(parser, TOKEN_LOGICAL_OR_OPERATOR)) {
        ASTNode *binaryNode = initASTNode(AST_BINARY_EXPRESSION, parser->currentToken);

        // Comsume the current operator token ('or')
        parser->currentToken = advanceParser(parser, sourceCode);

        binaryNode->left = root;
        binaryNode->right = parseLogicalAnd(parser, sourceCode);
        root = binaryNode;
    }

    return root;
}

ASTNode *parseLogicalAnd(Parser *parser, FILE *sourceCode) {
    ASTNode *root = parseAddition(parser, sourceCode);

    // Try to match logical and 
    while (matchTokenType(parser, TOKEN_LOGICAL_AND_OPERATOR)) {
        ASTNode *binaryNode = initASTNode(AST_BINARY_EXPRESSION, parser->currentToken);

        // Comsume the current operator token ('and')
        parser->currentToken = advanceParser(parser, sourceCode);

        binaryNode->left = root;
        binaryNode->right = parseAddition(parser, sourceCode);
        root = binaryNode;
    }

    return root;
}

ASTNode *parseAddition(Parser *parser, FILE *sourceCode) {
    // Where multiplication has higher precedence than the addition, so try to parse it first
    ASTNode *root = parseMultiplication(parser, sourceCode);

    // Try to match addition and subtraction
    while (matchTokenType(parser, TOKEN_ARITHMETIC_ADDITION) ||
           matchTokenType(parser, TOKEN_ARITHMETIC_SUBTRACTION)) {
        ASTNode *binaryNode = initASTNode(AST_BINARY_EXPRESSION, parser->currentToken);

        // Comsume the current operator token ('+' or '-')
        parser->currentToken = advanceParser(parser, sourceCode);

        binaryNode->left = root;
        binaryNode->right = parseMultiplication(parser, sourceCode);
        root = binaryNode;
    }

    return root;
}

ASTNode *parseMultiplication(Parser *parser, FILE *sourceCode) {
    // Where unary expression has higher precedence than the multiplication, so try to parse it first
    ASTNode *root = parsePrefix(parser, sourceCode);

    // Try to match addition and subtraction
    while (matchTokenType(parser, TOKEN_ARITHMETIC_MULTIPLICATION) ||
           matchTokenType(parser, TOKEN_ARITHMETIC_DIVISION) ||
           matchTokenType(parser, TOKEN_ARITHMETIC_MODULO)) {
        ASTNode *binaryNode = initASTNode(AST_BINARY_EXPRESSION, parser->currentToken);

        // Comsume the current operator token ('*', '/' or '%')
        parser->currentToken = advanceParser(parser, sourceCode);

        binaryNode->left = root;
        binaryNode->right = parsePrefix(parser, sourceCode);
        root = binaryNode;
    }

    return root;
}

ASTNode *parsePrefix(Parser *parser, FILE *sourceCode) {
    if (matchTokenType(parser, TOKEN_LOGICAL_NEGATION) || 
        matchTokenType(parser, TOKEN_ARITHMETIC_SUBTRACTION)) {
        ASTNode *root = initASTNode(AST_UNARY_EXPRESSION, parser->currentToken);
        
        // Comsume current operator token
        parser->currentToken = advanceParser(parser, sourceCode);
        root->left = parsePrefix(parser, sourceCode);

        return root;
    }

    return parsePostfix(parser, sourceCode);
}

ASTNode *parsePostfix(Parser *parser, FILE *sourceCode) {
    ASTNode *root = parsePrimary(parser, sourceCode);

    // Try to match all if there are multiple postfix operators
    while (1) {
        // Try to parse the function call
        if (matchTokenType(parser, TOKEN_OPENING_BRACKET)) {
            root = parseFunctionCall(parser, sourceCode, root);
        }

        // Try to match factorial
        else if (matchTokenType(parser, TOKEN_ARITHMETIC_FACTORIAL)) {
            ASTNode *postfixNode = initASTNode(AST_POSTFIX_EXPRESSION, parser->currentToken);
            postfixNode->left = root;
            root = postfixNode;

            parser->currentToken = advanceParser(parser, sourceCode);
        }

        else break;
    }

    return root;
}

ASTNode *parsePrimary(Parser *parser, FILE *sourceCode) {
    // Try to match literals
    if (matchTokenType(parser, TOKEN_NUMERIC) || matchTokenType(parser, TOKEN_STRING_LITERAL)) {
        ASTNode *root = initASTNode(AST_LITERAL, parser->currentToken);
        parser->currentToken = advanceParser(parser, sourceCode);
        return root;
    }

    // Try to match identifiers
    else if (matchTokenType(parser, TOKEN_IDENTIFIER)) {
        ASTNode *root = initASTNode(AST_IDENTIFIER, parser->currentToken);
        parser->currentToken = advanceParser(parser, sourceCode);
        return root;
    } 

    // Handle parenthesized expression 
    else if (matchTokenType(parser, TOKEN_OPENING_BRACKET)) {
        // Comsume opening bracket
        parser->currentToken = advanceParser(parser, sourceCode);
        ASTNode *root = parseExpression(parser, sourceCode);

        // Opus Lexer guaranteed that the opening and closing brackets match
        // Therefore we do not need to explicitly check if we could match the closing bracket
        // Once the expression be parsed, the current token is guaranteed to be a closing bracket
        // We comsume it without checking
        parser->currentToken = advanceParser(parser, sourceCode);
        return root;
    }

    // Try to match boolean literals
    else if (matchTokenType(parser, TOKEN_KEYWORD_TRUE) || matchTokenType(parser, TOKEN_KEYWORD_FALSE)) {
        ASTNode *root = initASTNode(AST_BOOLEAN_LITERAL, parser->currentToken);
        parser->currentToken = advanceParser(parser, sourceCode);
        return root;
    }

    // If we are unable to match anything
    parser->parseError = PARSE_ERROR_UNRESOLVABLE;
    parser->previousToken = parser->currentToken;
    reportParseError(parser);
    exit(1);
}

ASTNode* parseFunctionCall(Parser *parser, FILE *sourceCode, ASTNode* callee) {
    ASTNode *root = initASTNode(AST_FUNCTION_CALL, callee->token);
    root->left = callee;

    // Comsume opening bracket
    parser->currentToken = advanceParser(parser, sourceCode);

    // Parse the argument list
    ASTNode *argumentListNode = NULL;

    // Try to parse arguments if any
    if (!matchTokenType(parser, TOKEN_CLOSING_BRACKET)) {
        argumentListNode = parseArgumentList(parser, sourceCode);
    }

    root->right = argumentListNode;

    // Opus Lexer guaranteed that the opening and closing brackets match
    // Therefore we do not need to explicitly check if we could match the closing bracket
    // Once the expression be parsed, the current token is guaranteed to be a closing bracket
    // We comsume it without checking
    parser->currentToken = advanceParser(parser, sourceCode);

    return root;
}

ASTNode* parseArgumentList(Parser *parser, FILE *sourceCode) {
    // Each argument must be labeled, so try to match the first labeled argument 
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_ARGUMENT_LABEL;
        parser->previousToken = parser->currentToken;
        reportParseError(parser);
        exit(1);
    }

    ASTNode *argumentListNode = initASTNode(AST_ARGUMENT_LIST, NULL);
    ASTNode *argumentNode = initASTNode(AST_ARGUMENT, NULL);
    ASTNode *argumentLabelNode = initASTNode(AST_ARGUMENT_LABEL, parser->currentToken);

    // Comsume the current token for the argument label
    parser->currentToken = advanceParser(parser, sourceCode);

    if (!matchTokenType(parser, TOKEN_COLON)) {
        parser->parseError = PARSE_ERROR_MISSING_COLON_AFTER_LABEL;
        parser->previousToken = argumentLabelNode->token;
        reportParseError(parser);
        exit(1);
    }

    // Consume the colon token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to parse the expression 
    argumentNode->right = parseExpression(parser, sourceCode);
    argumentNode->left = argumentLabelNode;
    argumentListNode->left = argumentNode;

    // Check for additional arguments separated by commas
    if (matchTokenType(parser, TOKEN_COMMA)) {
        parser->currentToken = advanceParser(parser, sourceCode);
        argumentListNode->right = parseArgumentList(parser, sourceCode);
    }

    else argumentListNode->right = initASTNode(AST_ARGUMENT_LIST, NULL);
    return argumentListNode;
}

int matchTokenType(Parser *parser, TokenType type) { 
    // Compare the current parsing token type with the provided expected token type
    return parser->currentToken->tokenType == type; 
}

Token *advanceParser(Parser *parser, FILE *sourceCode) { 
    // Comsume the current token and move to the next token (and unable to move backward)
    return getNextToken(parser->lexer, sourceCode); 
} 

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
    parser->previousToken = NULL;

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
    for (int i = 0; i < level - 1; i++) printf("│   ");
    if (level > 0) printf("├── ");

    // Display the node
    switch (node->nodeType) {
        case AST_PROGRAM:                printf("AST_PROGRAM\n"); break;
        case AST_VARIABLE_DECLARATION:   printf("AST_VARIABLE_DECLARATION (%s)\n", node->token->lexeme); break;
        case AST_CONSTANT_DECLARATION:   printf("AST_CONSTANT_DECLARATION (%s)\n", node->token->lexeme); break;
        case AST_IDENTIFIER:             printf("AST_IDENTIFIER (%s)\n", node->token->lexeme); break;
        case AST_TYPE_ANNOTATION:        printf("AST_TYPE_ANNOTATION (%s)\n", node->token->lexeme); break;
        case AST_ASSIGNMENT_STATEMENT:   printf("AST_ASSIGNMENT (%s)\n", node->token->lexeme); break;
        case AST_LITERAL:                printf("AST_LITERAL (%s)\n", node->token->lexeme); break;
        case AST_BOOLEAN_LITERAL:        printf("AST_BOOLEAN_LITERAL (%s)\n", node->token->lexeme); break;
        case AST_BINARY_EXPRESSION:      printf("AST_BINARY_EXPRESSION (%s)\n", node->token->lexeme); break;
        case AST_UNARY_EXPRESSION:       printf("AST_UNARY_EXPRESSION (%s)\n", node->token->lexeme); break;
        case AST_POSTFIX_EXPRESSION:     printf("AST_POSTFIX_EXPRESSION (%s)\n", node->token->lexeme); break;
        case AST_FUNCTION_CALL:          printf("AST_FUNCTION_CALL\n"); break;
        case AST_ARGUMENT:               printf("AST_ARGUMENT\n"); break;
        case AST_ARGUMENT_LABEL:         printf("AST_ARGUMENT_LABEL (%s)\n", node->token->lexeme); break;
        case AST_ARGUMENT_LIST:          printf("AST_ARGUMENT_LIST\n"); break;
        default:                         printf("UNKNOWN NODE\n"); break;
    }

    if (node->left) displayAST(node->left, level + 1);
    if (node->right) displayAST(node->right, level + 1);
}

void reportParseError(Parser *parser) {
    Token *token = parser->previousToken;
    printf("Parsing Error at %d:%d\n", token->location.line, token->location.column);

    // Return if there is no error to display
    if (parser->parseError == PARSE_ERROR_NONE) return;

    switch (parser->parseError) {
        case PARSE_ERROR_MISSING_IDENTIFIER:
            printf("[ERROR] Expecting an name for the variable after '%s'\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_TYPE_ANNOTATION:
            printf("[ERROR] Expecting ':' for the type annotation after '%s'\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_TYPE_NAME:
            printf("[ERROR] Expecting a type name after ':'\n"); break;
        case PARSE_ERROR_DECLARATION_SYNTAX:
            printf("[ERROR] Expecting '=' or a newline after '%s'\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_RIGHT_VALUE:
            printf("[ERROR] Expecting something to be assigned to '%s' after '='\n", token->lexeme); break;
        case PARSE_ERROR_UNRESOLVABLE:
            printf("[ERROR] Unresolvable token after '%s'\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_ARGUMENT_LABEL:
            printf("[ERROR] Expecting label for argument %s in the function call.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_COLON_AFTER_LABEL:
            printf("[ERROR] Expecting ':' after the label '%s'", token->lexeme); break;
        default:
            printf("[ERROR] Unable to generate diagnostic information...\n");
    }
}

