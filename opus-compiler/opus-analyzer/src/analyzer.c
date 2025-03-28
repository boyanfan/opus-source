// analyzer.c 
//
// Created by Boyan Fan, 2025/03/23
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "analyzer.h"
#include "ast.h"

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
        reportAnalyzerError(analyzer, node);
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
        reportAnalyzerError(analyzer, node);
        return 0;
    }

    // Check if trying to modify a constant
    if (!symbol->isMutable && symbol->hasInitialized) {
        analyzer->analyzerError = ANALYZER_ERROR_IMMUTABLE_MODIFICATION;
        reportAnalyzerError(analyzer, node);
        return 0;
    }

    // Initialize symbol by assigning a value to it
    symbol->hasInitialized = 1;

    return result;
}

void reportAnalyzerError(Analyzer *analyzer, ASTNode *node) {
    switch (analyzer->analyzerError) {
        case ANALYZER_ERROR_REDECLARED_VARIABLE: {
            const char *identifier = node->left->token->lexeme;
            int line = node->left->token->location.line;
            int column = node->left->token->location.column;
            printf("[ERROR] Redeclared symbol '%s' at location %d:%d\n", identifier, line, column); 
            break;
        }

        case ANALYZER_ERROR_UNDECLARED_VARIABLE: {
            const char *identifier = node->left->token->lexeme;
            int line = node->left->token->location.line;
            int column = node->left->token->location.column;
            printf("[ERROR] Undeclared symbol '%s' at location %d:%d\n", identifier, line, column); 
            break;
        }

        case ANALYZER_ERROR_IMMUTABLE_MODIFICATION: {
            const char *identifier = node->left->token->lexeme;
            int line = node->left->token->location.line;
            int column = node->left->token->location.column;
            printf("[ERROR] Symbol '%s' is immutable at location %d:%d\n", identifier, line, column); 
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
    // Remove all symbols that belong to the current namespace.
    removeSymbolsFromCurrentNamespace(symbolTable);

    // Decrement the namespace counter only if current namespace is not global (0)
    if (symbolTable->currentNamespace > 0) symbolTable->currentNamespace--;
}

Symbol *lookupSymbolFromCurrentNamespace(SymbolTable *symbolTable, const char *identifier) {
    Symbol *currentSymbol = symbolTable->headSymbol;

    while (currentSymbol) {
        if (strcmp(currentSymbol->identifier, identifier) == 0 && 
            currentSymbol->namespace == symbolTable->currentNamespace) {
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

    printf("---------------------------------- Symbol Table -----------------------------------\n");
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
