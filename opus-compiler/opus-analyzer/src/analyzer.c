// analyzer.c 
//
// Created by Boyan Fan, 2025/03/23
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "analyzer.h"
#include "ast.h"
#include "symbol.h"

int analyzeProgram(Analyzer *analyzer, ASTNode *node) {
    // Return successful indication (True) if there is no node to analyze
    if (!node) return 1;
    
    int result = 1;

    if (node->nodeType == AST_PROGRAM) {
        if (node->left) result = analyzeStatement(analyzer, node->left);
        if (node->right) result = analyzeProgram(analyzer, node->right);
    }

    return result;
}

int analyzeStatement(Analyzer *analyzer, ASTNode *node) {
    if (node->nodeType == AST_VARIABLE_DECLARATION || node->nodeType == AST_CONSTANT_DECLARATION) {
        return analyzeDeclaration(analyzer, node);
    } 

    return 1;
}

int analyzeDeclaration(Analyzer *analyzer, ASTNode *node) {
    if (node->nodeType != AST_VARIABLE_DECLARATION && node->nodeType != AST_CONSTANT_DECLARATION) {
        return 0;
    }

    const char *identifier = node->left->token->lexeme;
    const char *type = node->right->token->lexeme;

    Symbol *currentSymbol = lookupSymbolFromCurrentNamespace(analyzer->symbolTable, identifier);
    
    if (currentSymbol) return 0;

    addSymbol(analyzer->symbolTable, identifier, type, node->token->location);
    return 1;
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

    printf("----------------------------- Symbol Table -----------------------------\n");
    printf("%-20s %-15s %-10s %-15s %s\n", "Identifier", "Type", "Namespace", "Initialized", "Location");

    while (currentSymbol) {
        printf("%-20s %-15s %-10d %-15s %d:%d\n",
            currentSymbol->identifier,
            currentSymbol->type,
            currentSymbol->namespace,
            currentSymbol->hasInitialized ? "Yes" : "No",
            currentSymbol->declarationLocation.line,
            currentSymbol->declarationLocation.column 
        );

        currentSymbol = currentSymbol->nextSymbol;
    }

    printf("------------------------------------------------------------------------\n");
}
