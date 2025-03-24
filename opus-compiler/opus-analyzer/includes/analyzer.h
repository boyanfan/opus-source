// analyzer.h
//
// This file defines the `Analyzer` struct used by the semantic analyzer for the Opus 
// programming language. The semantic analyzer is responsible for verifying that the
// Abstract Syntax Tree (AST) conforms to the language's semantic rules, such as 
// proper variable declarations, type checking, and scope resolution using a symbol table.
//
// Created by Boyan Fan, 2025/03/24
//

#ifndef ANALYZER_H
#define ANALYZER_H

#include "ast.h"
#include "symbol.h"

/// Enumerates possible semantic errors encountered during analysis.
typedef enum {
    ANALYZER_ERROR_NONE,                  /// No semantic error occurred.
    ANALYZER_ERROR_UNDECLARED_VARIABLE,   /// An undeclared variable was referenced in the source code.
    ANALYZER_ERROR_REDECLARED_VARIABLE,   /// A variable was declared more than once in the same scope.
} AnalyzerError;

/// Represents the semantic analyzer, which holds context for analyzing
/// an AST in the Opus programming language.
typedef struct {
    SymbolTable *symbolTable;      /// Pointer to the symbol table used during semantic analysis.
    AnalyzerError analyzerError;   /// Holds the current error state of the analyzer.
} Analyzer;

/// Initializes a new semantic analyzer with the given AST and symbol table. This function allocates
/// memory for an `Analyzer` struct and assigns the provided  AST and symbol table to it.
///
/// @param node Pointer to the root of the AST to be analyzed.
/// @param symbolTable Pointer to the symbol table used for semantic checks.
/// @return A pointer to the initialized `Analyzer` instance, or NULL if memory allocation fails.
///
Analyzer *initAnalyzer(ASTNode *node, SymbolTable *symbolTable);

int analyzeProgram(Analyzer *analyzer, ASTNode *node);

int analyzeStatement(Analyzer *analyzer, ASTNode *node);

int analyzeDeclaration(Analyzer *analyzer, ASTNode *node);

#endif
