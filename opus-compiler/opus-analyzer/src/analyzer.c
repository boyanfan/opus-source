// analyzer.c 
//
// Created by Boyan Fan, 2025/03/23
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "analyzer.h"
#include "ast.h"
#include "symbol.h"

int analyzeProgram(Analyzer *analyzer, ASTNode *node) {
    // Return successful indication (True) if there is no node to analyze
    int result = 1;
    if (!node) return result;

    // Recursively analyze each node's left and right nodes
    if (node->nodeType == AST_PROGRAM) { 
        if (node->left) result = analyzeStatement(analyzer, node->left) && result;
        if (node->right) result = analyzeProgram(analyzer, node->right) && result;
    }

    // Return the result after recursively analyzed all AST nodes
    return result;
}

int analyzeStatement(Analyzer *analyzer, ASTNode *node) {
    // Try to analyze variable and constant declaration statements
    if (node->nodeType == AST_VARIABLE_DECLARATION || node->nodeType == AST_CONSTANT_DECLARATION) { 
        return analyzeDeclarationStatement(analyzer, node);
    } 

    // Try to analyze assignment statements
    else if (node->nodeType == AST_ASSIGNMENT_STATEMENT) return analyzeAssignmentStatement(analyzer, node);

    // Try to analyze a conditional statement
    else if (node->nodeType == AST_CONDITIONAL_STATEMENT) return analyzeConditionalStatement(analyzer, node);

    // Return successful indication (True) if there is no node to analyze
    return 1;
}

int analyzeDeclarationStatement(Analyzer *analyzer, ASTNode *node) {
    // Get the variable or constant identifier and its type for symbol table lookup 
    const char *identifier = node->left->token->lexeme;
    const char *type = node->right->token->lexeme;

    // Check if the declaration already exists, report error
    if (lookupSymbolFromCurrentNamespace(analyzer->symbolTable, identifier)) {
        analyzer->analyzerError = ANALYZER_ERROR_REDECLARED_VARIABLE;
        reportAnalyzerError(analyzer, node->left);
        return 0;
    }

    // Add this declaration to the table
    addSymbol(analyzer->symbolTable, identifier, type, node->token->location);

    // Check if it is mutable and update the symbol table
    if (node->nodeType == AST_VARIABLE_DECLARATION) analyzer->symbolTable->headSymbol->isMutable = 1;

    return 1;
}

int analyzeAssignmentStatement(Analyzer *analyzer, ASTNode *node) {
    // Initialize successful indication (True) for multiple statements analyzing
    int result = 1;
    const char *identifier = node->left->token->lexeme;

    // If the declaration statement comes together with the assignment statement
    if (node->left->nodeType == AST_VARIABLE_DECLARATION || node->left->nodeType == AST_CONSTANT_DECLARATION) { 
        result = analyzeDeclarationStatement(analyzer, node->left);

        // Return if the declaration statement is invalid
        if (!result) return result;

        // Otherwise, get the declared identifier
        identifier = analyzer->symbolTable->headSymbol->identifier;
    }  

    // Then check if the identifier exist
    Symbol* symbol = lookupSymbolFromCurrentNamespace(analyzer->symbolTable, identifier);

    // Check if trying to assign to an undeclared variable or constant 
    if (!symbol) {
        analyzer->analyzerError = ANALYZER_ERROR_UNDECLARED_VARIABLE;
        reportAnalyzerError(analyzer, node->left);
        return 0;
    }

    // Check if trying to modify a constant
    if (!symbol->isMutable && symbol->hasInitialized) {
        analyzer->analyzerError = ANALYZER_ERROR_IMMUTABLE_MODIFICATION;
        reportAnalyzerError(analyzer, node->left);
        return 0;
    }

    // Analyze the rhs expression
    result = analyzeExpression(analyzer, node->right) && result;

    // Perform type checkinig for the assignment statement (type-check lhs and rhs)
    if (strcmp(symbol->type, node->right->inferredType)) {
        analyzer->analyzerError = ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH;
        reportAnalyzerError(analyzer, node);
        return 0;
    }

    // If the right-hand side is foldable, propagate its value to the symbol 
    if (node->right->isFoldable) {
        if (strcmp(node->right->inferredType, "Int") == 0) {
            int value = node->right->nodeValue.integerValue;
            symbol->symbolValue.integerValue = value;
            printf("[Analyzer] Symbol '%s' may be assigned with integer '%d'.\n", symbol->identifier, value);
        }

        else if (strcmp(node->right->inferredType, "Float") == 0) {
            float value = node->right->nodeValue.floatingValue;
            symbol->symbolValue.floatingValue = value;
            printf("[Analyzer] Symbol '%s' may be assigned with float '%f'.\n", symbol->identifier, value);
        }

        else if (strcmp(node->right->inferredType, "Bool") == 0) {
            int value = node->right->nodeValue.booleanValue;
            symbol->symbolValue.booleanValue = value;
            printf("[Analyzer] Symbol '%s' may be assigned with boolean '%s'.\n", symbol->identifier, 
                   value == 0 ? "false" : "true");
        }

        else if (strcmp(node->right->inferredType, "String") == 0) {
            const char* value = node->right->nodeValue.stringLiteral;
            strcpy(symbol->symbolValue.stringLiteral, value);
            printf("[Analyzer] Symbol '%s' may be assigned with string '%s'.\n", symbol->identifier, value);
        }
    }

    // Initialize symbol by assigning a value to it
    symbol->hasInitialized = 1;
    return result;
}

int analyzeExpression(Analyzer *analyzer, ASTNode *node) {
    // Return successful indication (True) if there is no node to analyze
    if (!node) return 1;

    // Perform ASTNode evaluation based on the node type  
    switch (node->nodeType) {
        // Determine if the boolean literal is 'true' or 'false'
        case AST_BOOLEAN_LITERAL: {
            strcpy(node->inferredType, "Bool");
            node->isFoldable = 1;
            node->nodeValue.booleanValue = (strcmp(node->token->lexeme, "true") == 0);
            return 1;
        }

        // Determine if the literal is a Float, Int, or StringLiteral
        case AST_LITERAL: {
            // Handle string literal 
            if (node->token->tokenType == TOKEN_STRING_LITERAL) {
                strcpy(node->inferredType, "String");
                node->isFoldable = 1;
                strcpy(node->nodeValue.stringLiteral, node->token->lexeme);
            }

            // Handle numeric literal
            else if (node->token->tokenType == TOKEN_NUMERIC) {
                // Handle floating point literal
                if (strchr(node->token->lexeme, '.') != NULL) {
                    strcpy(node->inferredType, "Float");
                    node->isFoldable = 1;
                    node->nodeValue.floatingValue = atof(node->token->lexeme);
                }

                // Otherwise it is an integer
                else {
                    strcpy(node->inferredType, "Int");
                    node->isFoldable = 1;
                    node->nodeValue.integerValue = atoi(node->token->lexeme);
                }
            }
            return 1;
        }

        // Determine if a symbol is referenced
        case AST_IDENTIFIER: {
            Symbol* symbol = lookupSymbolFromCurrentNamespace(analyzer->symbolTable, node->token->lexeme);

            // If an undeclared symbol is referenced
            if (!symbol) {
                analyzer->analyzerError = ANALYZER_ERROR_UNDECLARED_VARIABLE;
                reportAnalyzerError(analyzer, node);
                return 0;
            }

            strcpy(node->inferredType, symbol->type);

            // If it has been initialized, we can perform constant fold
            if (symbol->hasInitialized) {
                // Handle string literal 
                if (strcmp(symbol->type, "String")) {
                    strcpy(node->nodeValue.stringLiteral, symbol->symbolValue.stringLiteral);
                }

                // Handle float 
                else if (strcmp(symbol->type, "Float")) {
                    node->nodeValue.floatingValue = symbol->symbolValue.floatingValue;
                }

                // Handle integer
                else if (strcmp(symbol->type, "Int")) {
                    node->nodeValue.integerValue = symbol->symbolValue.integerValue;
                }

                // Handle boolean
                else if (strcmp(symbol->type, "Bool")) {
                    node->nodeValue.booleanValue = symbol->symbolValue.booleanValue;
                }

                // If unable to reference value from the identifier
                else node->isFoldable = 0;
            }

            else node->isFoldable = 0;
            return 1;
        }

        // Determine if it is a binary expression 
        case AST_BINARY_EXPRESSION: {
            // Recursively analyze left and right operands
            if (!analyzeExpression(analyzer, node->left)) return 0;
            if (!analyzeExpression(analyzer, node->right)) return 0;

            TokenType operator = node->token->tokenType;
            ASTNode* lhs = node->left;
            ASTNode* rhs = node->right;

            // For arithmetic operators, both operands must be numeric 
            if (operator == TOKEN_ARITHMETIC_ADDITION || operator == TOKEN_ARITHMETIC_SUBTRACTION ||
                operator == TOKEN_ARITHMETIC_MULTIPLICATION || operator == TOKEN_ARITHMETIC_DIVISION ||
                operator == TOKEN_ARITHMETIC_MODULO) {

                // Handle missmatched type 
                if (!isNumeric(lhs->inferredType) || !isNumeric(rhs->inferredType)) {
                    analyzer->analyzerError = ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH;
                    reportAnalyzerError(analyzer, node);
                    return 0;
                }

                // Infer the result type as Float if either operand is Float
                if (strcmp(lhs->inferredType, "Float") == 0 || strcmp(rhs->inferredType, "Float") == 0) {
                    strcpy(node->inferredType, "Float");
                }

                // Otherwise the result is an Int
                else strcpy(node->inferredType, "Int");    
            }

            // For logical operators 'and' and 'or', both operands must be boolean
            else if (operator == TOKEN_LOGICAL_AND_OPERATOR || operator == TOKEN_LOGICAL_OR_OPERATOR) {
                if (strcmp(lhs->inferredType, "Bool") != 0 || strcmp(rhs->inferredType, "Bool") != 0) {
                    analyzer->analyzerError = ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH;
                    reportAnalyzerError(analyzer, node);
                    return 0;
                }
            }

            // For logical operators '==' and '!=', both operands must be the same type 
            else if (operator == TOKEN_LOGICAL_EQUIVALENCE || operator == TOKEN_NOT_EQUAL_TO_OPERATOR) {
                if (strcmp(lhs->inferredType, rhs->inferredType) != 0) {
                    analyzer->analyzerError = ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH;
                    reportAnalyzerError(analyzer, node);
                    return 0;
                }
            }

            // For relational operators '>', '<', '>=' and '<=', both operands must be numeric
            else if (operator == TOKEN_GREATER_THAN_OPERATOR || operator == TOKEN_LESS_THAN_OPERATOR ||
                     operator == TOKEN_GREATER_OR_EQUAL_TO_OPERATOR || operator == TOKEN_LESS_OR_EQUAL_TO_OPERATOR) {
                if (!(isNumeric(lhs->inferredType) && isNumeric(rhs->inferredType))) {
                    analyzer->analyzerError = ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH;
                    reportAnalyzerError(analyzer, node);
                    return 0;
                }
            }

            // Perform constant fold if both lhs and rhs are foldable 
            if (node->left->isFoldable && node->right->isFoldable) foldBinaryExpression(node);

            // TODO:  Support relational and logical operators
            return 1;
        }

        // Determine if it is a unary expression
        case AST_UNARY_EXPRESSION: {
            // Recursively analyze left operands
            if (!analyzeExpression(analyzer, node->left)) return 0;

            TokenType operator = node->token->tokenType;
            ASTNode* operand = node->left;

            // Unary minus only applies on numeric value
            if (operator == TOKEN_ARITHMETIC_SUBTRACTION) {
                if (!isNumeric(operand->inferredType)) {
                    analyzer->analyzerError = ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH;
                    reportAnalyzerError(analyzer, node);
                    return 0;
                }
                strcpy(node->inferredType, operand->inferredType);
            }

            // Unary negation only applies on boolean value 
            else if (operator == TOKEN_LOGICAL_NEGATION) {
                if (strcmp(operand->inferredType, "Bool") != 0) {
                    analyzer->analyzerError = ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH;
                    reportAnalyzerError(analyzer, node);
                    return 0;
                }
                strcpy(node->inferredType, "Bool");
            }

            // Unary factorial only applies on positive integers
            else if (operator == TOKEN_ARITHMETIC_FACTORIAL) {
                if (strcmp(operand->inferredType, "Int") != 0) {
                    analyzer->analyzerError = ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH;
                    reportAnalyzerError(analyzer, node);
                    return 0;
                }
                strcpy(node->inferredType, "Int");
            }

            if (operand->isFoldable) foldUnaryExpression(node);
            return 1;
        }

        // TODO: Support function call and other node types 
        default: return 1;
    }
}

int analyzeCodeBlock(Analyzer *analyzer, ASTNode *node) {
    int result = 1;

    // Process each statement within the code block
    ASTNode *statement = node->left;
    while (statement != NULL) {
        result = analyzeStatement(analyzer, statement) && result;
        statement = statement->right;
    }

    // Recursively handle code blocks
    if (node->right) result = analyzeCodeBlock(analyzer, node->right) && result;
    return result;
}

int analyzeConditionalStatement(Analyzer *analyzer, ASTNode *node) {
    ASTNode *condition = node->left;
    ASTNode *conditionalBody = node->right;

    int result = analyzeExpression(analyzer, condition);
    
    // Condition must be a boolean value 
    if (!result || strcmp(condition->inferredType, "Bool")) {
        analyzer->analyzerError = ANALYZER_ERROR_INVALID_CONDITION;
        reportAnalyzerError(analyzer, node);
        return 0;
    }

    // If the condition is foldable, we can statically determine which condition body to execute
    int safeEliminateFirstCodeBlock = 0;
    int safeEliminateSecondCodeBlock = 0;

    if (condition->isFoldable) {
        if (condition->nodeValue.booleanValue) safeEliminateSecondCodeBlock = 1;
        else safeEliminateFirstCodeBlock = 1;
    }

    if (conditionalBody->nodeType == AST_CONDITIONAL_BODY) {
        // Analyze if-body
        if (conditionalBody->left && !safeEliminateFirstCodeBlock) { 
            enterNamespace(analyzer->symbolTable);
            result = analyzeCodeBlock(analyzer, conditionalBody->left) && result;
            exitNamespace(analyzer->symbolTable);
        }

        // Try to analyze else statement body if exists
        if (conditionalBody->right && !safeEliminateSecondCodeBlock) {
            enterNamespace(analyzer->symbolTable);
            result = analyzeCodeBlock(analyzer, conditionalBody->right) && result;
            exitNamespace(analyzer->symbolTable);
        }
    }

    return result;
}

void foldBinaryExpression(ASTNode* node) {
    TokenType operator = node->token->tokenType;
    ASTNode *lhs = node->left;
    ASTNode *rhs = node->right;

    // Check for the arithmetic binary expression
    if (operator == TOKEN_ARITHMETIC_ADDITION || operator == TOKEN_ARITHMETIC_SUBTRACTION ||
        operator == TOKEN_ARITHMETIC_MULTIPLICATION || operator == TOKEN_ARITHMETIC_DIVISION ||
        operator == TOKEN_ARITHMETIC_MODULO) {

        // Try to infer the result type, where it is Float if either operand is a Float; otherwise Int 
        int isFloat = (strcmp(lhs->inferredType, "Float") == 0 || strcmp(rhs->inferredType, "Float") == 0);

        // If either operand is a Float, perform floating point operation
        if (isFloat) {
            // Get the value from the lhs and rhs
            float lhsValue = (strcmp(lhs->inferredType, "Float") == 0) ? 
                             lhs->nodeValue.floatingValue : (float) lhs->nodeValue.integerValue;
            float rhsValue = (strcmp(rhs->inferredType, "Float") == 0) ? 
                             rhs->nodeValue.floatingValue : (float) rhs->nodeValue.integerValue;
            float result = 0.0f;

            // Perform arithmetic operation
            if (operator == TOKEN_ARITHMETIC_ADDITION) result = lhsValue + rhsValue;
            else if (operator == TOKEN_ARITHMETIC_SUBTRACTION) result = lhsValue - rhsValue;
            else if (operator == TOKEN_ARITHMETIC_MULTIPLICATION) result = lhsValue * rhsValue;
            else if (operator == TOKEN_ARITHMETIC_DIVISION) result = lhsValue / rhsValue;
            else if (operator == TOKEN_ARITHMETIC_MODULO) result = fmodf(lhsValue, rhsValue);

            node->isFoldable = 1;
            node->nodeValue.floatingValue = result;
            strcpy(node->inferredType, "Float");
        }

        // Otherwise, perform integer operation
        else {
            // Get the value from the lhs and rhs
            int lhsValue = lhs->nodeValue.integerValue;
            int rhsValue = rhs->nodeValue.integerValue;
            int result = 0;

            // Perform arithmetic operation
            if (operator == TOKEN_ARITHMETIC_ADDITION) result = lhsValue + rhsValue;
            else if (operator == TOKEN_ARITHMETIC_SUBTRACTION) result = lhsValue - rhsValue;
            else if (operator == TOKEN_ARITHMETIC_MULTIPLICATION) result = lhsValue * rhsValue;
            else if (operator == TOKEN_ARITHMETIC_DIVISION) result = lhsValue / rhsValue;
            else if (operator == TOKEN_ARITHMETIC_MODULO) result = lhsValue % rhsValue;

            node->isFoldable = 1;
            node->nodeValue.integerValue = result;
            strcpy(node->inferredType, "Int");
        }
    }

    // Check for the logical 'and' and 'or' binary expression 
    else if (operator == TOKEN_LOGICAL_AND_OPERATOR || operator == TOKEN_LOGICAL_OR_OPERATOR) {
        strcpy(node->inferredType, "Bool");

        if (lhs->isFoldable && rhs->isFoldable) {
            int lhsValue = lhs->nodeValue.booleanValue;
            int rhsValue = rhs->nodeValue.booleanValue;
            int result = (operator == TOKEN_LOGICAL_AND_OPERATOR) ? (lhsValue && rhsValue) : (lhsValue || rhsValue);
            node->nodeValue.booleanValue = result;
            node->isFoldable = 1;
        }
    }

    // Check for the logical '==' and '!=' binary expression
    else if (operator == TOKEN_LOGICAL_EQUIVALENCE || operator == TOKEN_NOT_EQUAL_TO_OPERATOR) {
        strcpy(node->inferredType, "Bool");
        
        int result = 0;

        if (strcmp(lhs->inferredType, "Int") == 0) 
            result = (lhs->nodeValue.integerValue == rhs->nodeValue.integerValue);

        else if (strcmp(lhs->inferredType, "Float") == 0)
            result = (lhs->nodeValue.floatingValue == rhs->nodeValue.floatingValue);

        else if (strcmp(lhs->inferredType, "Bool") == 0)

            result = (lhs->nodeValue.booleanValue == rhs->nodeValue.booleanValue);

        else if (strcmp(lhs->inferredType, "String") == 0)
            result = (strcmp(lhs->nodeValue.stringLiteral, rhs->nodeValue.stringLiteral) == 0);

        node->nodeValue.booleanValue = (operator == TOKEN_LOGICAL_EQUIVALENCE) ? result : !result;
        node->isFoldable = 1;
    }

    // Check for relational operators '>', '<', '>=' and '<='
    else if (operator == TOKEN_GREATER_THAN_OPERATOR || operator == TOKEN_LESS_THAN_OPERATOR ||
             operator == TOKEN_GREATER_OR_EQUAL_TO_OPERATOR || operator == TOKEN_LESS_OR_EQUAL_TO_OPERATOR) {
        strcpy(node->inferredType, "Bool");

        float lhsValue = (strcmp(lhs->inferredType, "Float") == 0) ?
                         lhs->nodeValue.floatingValue : (float) lhs->nodeValue.integerValue;

        float rhsValue = (strcmp(rhs->inferredType, "Float") == 0) ?
                         rhs->nodeValue.floatingValue : (float) rhs->nodeValue.integerValue;
        
        int result = 0;

        if (operator == TOKEN_GREATER_THAN_OPERATOR) result = lhsValue > rhsValue;
        else if (operator == TOKEN_LESS_THAN_OPERATOR) result = lhsValue < rhsValue;
        else if (operator == TOKEN_GREATER_OR_EQUAL_TO_OPERATOR) result = lhsValue >= rhsValue;
        else if (operator == TOKEN_LESS_OR_EQUAL_TO_OPERATOR) result = lhsValue <= rhsValue;

        node->nodeValue.booleanValue = result;
        node->isFoldable = 1;
    }
}

void foldUnaryExpression(ASTNode* node) {
    TokenType operator = node->token->tokenType;
    ASTNode* operand = node->left;

    // Unary minus for getting the negation of an numeric value
    if (operator == TOKEN_ARITHMETIC_SUBTRACTION) {
        if (strcmp(operand->inferredType, "Float") == 0) {
            node->isFoldable = 1;
            node->nodeValue.floatingValue = -(operand->nodeValue.floatingValue);
            strcpy(node->inferredType, "Float");
        } 

        else if (strcmp(operand->inferredType, "Int") == 0) {
            node->isFoldable = 1;
            node->nodeValue.integerValue = -(operand->nodeValue.integerValue);
            strcpy(node->inferredType, "Int");
        }
    }

    // Unary negation for getting the inverse of a boolean value
    else if (operator == TOKEN_LOGICAL_NEGATION) {
        if (operand->isFoldable) {
            node->isFoldable = 1;
            node->nodeValue.booleanValue = !(operand->nodeValue.booleanValue);
        }
        strcpy(node->inferredType, "Bool");
    }

    // Unary factorial operation
    else if (operator == TOKEN_ARITHMETIC_FACTORIAL) {
        int number = operand->nodeValue.integerValue;
        int result = 1;

        // Perform factorial operation
        for (int term = 1; term <= number; term++) result *= term;
        
        node->isFoldable = 1;
        node->nodeValue.integerValue = result;
        strcpy(node->inferredType, "Int");
    }
}

void reportAnalyzerError(Analyzer *analyzer, ASTNode *node) {
    switch (analyzer->analyzerError) {
        case ANALYZER_ERROR_REDECLARED_VARIABLE: {
            const char *identifier = node->token->lexeme;
            int line = node->token->location.line;
            int column = node->token->location.column;
            printf("[ERROR] Redeclared symbol '%s' at location %d:%d.\n", identifier, line, column); 
            break;
        }

        case ANALYZER_ERROR_UNDECLARED_VARIABLE: {
            const char *identifier = node->token->lexeme;
            int line = node->token->location.line;
            int column = node->token->location.column;
            printf("[ERROR] Undeclared symbol '%s' at location %d:%d.\n", identifier, line, column); 
            break;
        }

        case ANALYZER_ERROR_IMMUTABLE_MODIFICATION: {
            const char *identifier = node->token->lexeme;
            int line = node->token->location.line;
            int column = node->token->location.column;
            printf("[ERROR] Symbol '%s' is immutable at location %d:%d.\n", identifier, line, column); 
            break;
        }

        case ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH: {
            const char *operator = node->token->lexeme;
            int line = node->token->location.line;
            int column = node->token->location.column;
            printf("[ERROR] Unable to perform '%s' due to type missmatch at location %d:%d.\n", operator, line, column);
            break;
        }

        case ANALYZER_ERROR_INVALID_CONDITION: {
            const char *statement = node->token->lexeme;
            int line = node->token->location.line;
            int column = node->token->location.column;
            printf("[ERROR] Invalid condition for '%s' statement at location %d:%d.\n", statement, line, column);
            break;
        }
        default: printf("Unknown error!\n"); break;
    }
}

Analyzer *initAnalyzer(ASTNode *node, SymbolTable *symbolTable) {
    Analyzer *analyzer = (Analyzer*) malloc(sizeof(Analyzer));

    if (analyzer) {
        analyzer->symbolTable = symbolTable;
        analyzer->analyzerError = ANALYZER_ERROR_NONE;
    }

    return analyzer;
}

SymbolTable *initSymbolTable() {
    SymbolTable *symbolTable = (SymbolTable*) malloc(sizeof(SymbolTable));

    if (symbolTable) {
        symbolTable->currentNamespace = 0;
        symbolTable->headSymbol = NULL;
    }

    return symbolTable;
}

void addSymbol(SymbolTable *symbolTable, const char *identifier, const char *type, Location location) {
    Symbol *symbol = (Symbol*) malloc(sizeof(Symbol));

    if (symbol) {
        strcpy(symbol->identifier, identifier);
        strcpy(symbol->type, type);
        symbol->namespace = symbolTable->currentNamespace;
        symbol->declarationLocation = location;
        symbol->hasInitialized = 0;
        symbol->isMutable = 0;

        // Add to the beginning of the linked list
        symbol->nextSymbol = symbolTable->headSymbol;
        symbolTable->headSymbol = symbol;
    } 
}

Symbol *lookupSymbol(SymbolTable *symbolTable, const char *identifier) {
    Symbol *currentSymbol = symbolTable->headSymbol;

    while (currentSymbol) {
        if (strcmp(currentSymbol->identifier, identifier) == 0) {
            return currentSymbol;
        }
        currentSymbol = currentSymbol->nextSymbol;
    }

    return NULL;
}

void enterNamespace(SymbolTable *symbolTable) {
    // Enters a new namespace by incrementing the current namespace counter
    symbolTable->currentNamespace++; 
}

void exitNamespace(SymbolTable *symbolTable) {
    // Print header for removed symbols
    printf("\n------------------------ Removing Symbols from Namespace %d ------------------------\n", symbolTable->currentNamespace);
    printf("%-20s %-20s %-10s %-12s %-8s %s\n", "Identifier", "Type", "Namespace", "Initialized", "Mutable", "Location");

    // Remove all symbols that belong to the current namespace
    removeSymbolsFromCurrentNamespace(symbolTable);
    printf("-----------------------------------------------------------------------------------\n");
    
    // Decrement the namespace counter only if current namespace is not global (0)
    if (symbolTable->currentNamespace > 0) symbolTable->currentNamespace--;
}

Symbol *lookupSymbolFromCurrentNamespace(SymbolTable *symbolTable, const char *identifier) {
    Symbol *currentSymbol = symbolTable->headSymbol;

    while (currentSymbol) {
        if (strcmp(currentSymbol->identifier, identifier) == 0 && 
            currentSymbol->namespace <= symbolTable->currentNamespace) {
            return currentSymbol;
        }
        currentSymbol = currentSymbol->nextSymbol;
    }

    return NULL;
}

void removeSymbolsFromCurrentNamespace(SymbolTable *symbolTable) {
    Symbol **currentSymbol = &symbolTable->headSymbol;

    // Traverse the linked list while keeping a pointer to the pointer that points to the current symbol
    while (*currentSymbol) {
        // If found a symbol in the current namespace, we remove it
        if ((*currentSymbol)->namespace == symbolTable->currentNamespace) {
            Symbol *toRemove = *currentSymbol;

            // Display the symbol being removed.
            printf("%-20s %-20s %-10d %-12s %-8s %d:%d\n",
                   toRemove->identifier,
                   toRemove->type,
                   toRemove->namespace,
                   toRemove->hasInitialized ? "Yes" : "No",
                   toRemove->isMutable ? "Yes" : "No",
                   toRemove->declarationLocation.line,
                   toRemove->declarationLocation.column);

            *currentSymbol = (*currentSymbol)->nextSymbol;
            free(toRemove);
        }

        // Move to the next symbol
        else { currentSymbol = &((*currentSymbol)->nextSymbol); }
    }
}

void freeSymbolTable(SymbolTable *symbolTable) {
    Symbol *currentSymbol = symbolTable->headSymbol;

    while (currentSymbol) {
        Symbol *nextSymbol = currentSymbol->nextSymbol;
        free(currentSymbol);
        currentSymbol = nextSymbol;
    }

    free(symbolTable);
}

void displaySymbolTable(SymbolTable *symbolTable) {
    Symbol *currentSymbol = symbolTable->headSymbol;

    printf("\n---------------------------------- Symbol Table -----------------------------------\n");
    printf("%-20s %-20s %-10s %-12s %-8s %s\n", "Identifier", "Type", "Namespace", "Initialized", "Mutable", "Location");

    while (currentSymbol) {
        printf("%-20s %-20s %-10d %-12s %-8s %d:%d\n",
            currentSymbol->identifier,
            currentSymbol->type,
            currentSymbol->namespace,
            currentSymbol->hasInitialized ? "Yes" : "No",
            currentSymbol->isMutable ? "Yes" : "No",
            currentSymbol->declarationLocation.line,
            currentSymbol->declarationLocation.column 
        );

        currentSymbol = currentSymbol->nextSymbol;
    }

    printf("-----------------------------------------------------------------------------------\n");
}

int isNumeric(const char* type) {
    // Checks if the given type is numeric
    return (strcmp(type, "Int") == 0 || strcmp(type, "Float") == 0);
}
