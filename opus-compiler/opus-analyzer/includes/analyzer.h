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
    ANALYZER_ERROR_NONE,                       /// No semantic error occurred.
    ANALYZER_ERROR_UNDECLARED_VARIABLE,        /// An undeclared variable was referenced in the source code.
    ANALYZER_ERROR_REDECLARED_VARIABLE,        /// A variable was declared more than once in the same scope.
    ANALYZER_ERROR_IMMUTABLE_MODIFICATION,     /// Modifying an immutable instance like a constant.
    ANALYZER_ERROR_OPERATION_TYPE_MISSMATCH,   /// Type missmatch for operators.
} AnalyzerError;

/// Represents the semantic analyzer, which holds context for analyzing
/// an AST in the Opus programming language.
typedef struct {
    SymbolTable *symbolTable;      /// Pointer to the symbol table used during semantic analysis.
    AnalyzerError analyzerError;   /// Holds the current error state of the analyzer.
} Analyzer;

/// Analyzes the semantic correctness of an entire Opus program AST.
///
/// This function recursively analyzes all statements in the AST representing a program,
/// performing type checking, constant propagation, and symbol resolution for each statement.
///
/// @param analyzer Pointer to the Analyzer instance.
/// @param node Pointer to the root AST node representing the program.
/// @return 1 (True) if semantic analysis succeeds; 0 (False) if an error occurs.
///
int analyzeProgram(Analyzer *analyzer, ASTNode *node);

/// Analyzes a single statement node for semantic correctness.
///
/// This function dispatches to specialized analyzers depending on the type of statement.
///
/// @param analyzer Pointer to the Analyzer instance.
/// @param node Pointer to the AST node representing the statement.
/// @return 1 (True) if the statement is semantically valid; 0 (False) if an error occurs.
///
int analyzeStatement(Analyzer *analyzer, ASTNode *node);

/// Analyzes a variable or constant declaration statement.
///
/// This function verifies that the type annotation is valid, the identifier is not redefined,
/// and that the symbol is added to the symbol table with appropriate mutability and type info.
///
/// @param analyzer Pointer to the Analyzer instance.
/// @param node Pointer to the AST node representing the declaration.
/// @return 1 (True) if the declaration is semantically valid; 0 (False) if an error occurs.
///
int analyzeDeclarationStatement(Analyzer *analyzer, ASTNode *node);

/// Analyzes an assignment statement for semantic correctness.
///
/// This function verifies that the target identifier exists and is mutable (if reassigned),
/// and that the right-hand expression is type compatible. If the RHS is a constant expression,
/// the value may be propagated into the symbol table.
///
/// @param analyzer Pointer to the Analyzer instance.
/// @param node Pointer to the AST node representing the assignment.
/// @return 1 (True) if the assignment is semantically valid; 0 (False) if an error occurs.
///
int analyzeAssignmentStatement(Analyzer *analyzer, ASTNode *node);

/// Analyzes an expression for type correctness and performs constant folding.
/// This function traverses the AST of the expression to infer types, validate operations,
/// and precompute constant expressions. Results are annotated in the AST node fields.
///
/// @param analyzer Pointer to the Analyzer instance.
/// @param node Pointer to the AST node representing the expression.
/// @return 1 (True) if the expression is semantically valid; 0 (False) if an error occurs.
///
int analyzeExpression(Analyzer *analyzer, ASTNode *node);

/// Evaluates a binary expression at compile time and folds it into a constant node.
///
/// This function assumes the expression is valid and both child nodes are constant.
/// It performs arithmetic computation and stores the folded result into the parent node.
///
/// @param node Pointer to the AST node representing the binary expression.
///
void foldBinaryExpression(ASTNode* node);

/// Evaluates a unary expression at compile time and folds it into a constant node.
/// This function assumes the operand is valid and constant. It handles logical negation,
/// arithmetic negation, and factorial operators, computing and storing the result.
///
/// @param node Pointer to the AST node representing the unary expression.
///
void foldUnaryExpression(ASTNode* node);

/// Reports a semantic analysis error related to a specific AST node.
/// This function uses the Analyzer's error state and the node location to emit an
/// error message, typically for undeclared identifiers, type mismatches, or invalid use.
///
/// @param analyzer Pointer to the Analyzer instance.
/// @param node Pointer to the AST node where the error occurred.
///
void reportAnalyzerError(Analyzer *analyzer, ASTNode *node); 

/// Initializes a new semantic analyzer with the given AST and symbol table. This function allocates
/// memory for an `Analyzer` struct and assigns the provided  AST and symbol table to it.
///
/// @param node Pointer to the root of the AST to be analyzed.
/// @param symbolTable Pointer to the symbol table used for semantic checks.
/// @return A pointer to the initialized `Analyzer` instance, or NULL if memory allocation fails.
///
Analyzer *initAnalyzer(ASTNode *node, SymbolTable *symbolTable);

/// Determines whether a given type name represents a numeric type.
/// This helper checks if the type is "Int" or "Float", which are considered numeric
/// and usable in arithmetic expressions in the Opus language.
///
/// @param type A string representing a type name.
/// @return 1 (True) if the type is numeric; 0 (False) otherwise.
///
int isNumeric(const char* type);

#endif
