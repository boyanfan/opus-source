// ast.h
//
// Abstract Syntax Tree (AST) definitions for the Opus programming language. The AST represents the 
// hierarchical structure of parsed source code.
//
// Created by Boyan Fan, 2025/03/02
//

#ifndef AST_H
#define AST_H

#include "token.h"

/// AST Node Types for representing different syntactic constructs in the language.
typedef enum {
    AST_PROGRAM,                  /// Root node of the entire program.
    AST_VARIABLE_DECLARATION,     /// Variable declaration node (e.g. "var number: Int").
    AST_ASSIGNMENT_STATEMENT,     /// Assignment statement node (e.g. "number = 5").
    AST_ARITHMETIC_EXPRESSION,    /// Arithmetic expression node (supports +-*/% and factorial '!').
    AST_BOOLEAN_EXPRESSION,       /// Boolean expression node (e.g. "condition1 && condition2").
    AST_CONDITIONAL_STATEMENT,    /// Conditional statement node (if-else).
    AST_FUNCTION_CALL,            /// Function call node (e.g. "range(1, 3)").
    AST_FORIN_LOOP_STATEMENT,     /// For loop node (e.g. "for number in range(1, 3) { body }").
    AST_IO_STATEMENT,             /// Input/Output statement node (e.g. "print(...)", "read(from: Int.self)").
    AST_REPEAT_UNTIL_STATEMENT,   /// Repeat-until loop node (e.g. "repeat { body } until (condition)").
} ASTNodeType;

/// AST Node structure for the abstract syntax tree.
typedef struct ASTNode {
    Token token;             /// The token associated with this AST node (if applicable).
    ASTNodeType nodeType;    /// The type of AST node.
    struct ASTNode* left;    /// Pointer to the first child node (or left operand).
    struct ASTNode* right;   /// Pointer to the next sibling or right operand node.
} ASTNode;

/// Allocates and initializes a new AST node.
///
/// @param nodeType The type of the AST node.
/// @param token The associated token (e.g., keyword, identifier, operator).
/// @return A pointer to the newly created ASTNode.
///
ASTNode* initASTNode(ASTNodeType nodeType, Token token);

/// Recursively frees memory associated with an AST and its children.
/// @param node Pointer to the root node of the AST (or subtree).
///
void freeAST(ASTNode* node);

/// Recursively prints the Abstract Syntax Tree (AST) in a structured format.
/// This function is useful for debugging and visualizing the tree structure.
///
/// @param node The root node of the AST (or subtree) to display.
/// @param level The indentation level used for hierarchical formatting.
///
void displayAST(ASTNode* node, int level);

#endif
