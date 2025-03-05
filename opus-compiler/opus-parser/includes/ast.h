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
    AST_PROGRAM,                /// Root node of the entire program.
    AST_VARIABLE_DECLARATION,   /// Variable declaration node (e.g. "var number: Int").
    AST_CONSTANT_DECLARATION,   /// Constant declaration node (e.g. "let name: String").
    AST_IDENTIFIER,             /// Identifier node (e.g. number).
    AST_LITERAL,                /// Literals node (e.g. 3.14, "Boyan Fan")
    AST_TYPE_ANNOTATION,        /// Type annotations (e.g. number: Int).
    AST_ASSIGNMENT_STATEMENT,   /// Assignment statement node (e.g. "number = 5").
} ASTNodeType;

/// AST Node structure for the abstract syntax tree.
typedef struct ASTNode {
    Token *token;             /// The token associated with this AST node (if applicable).
    ASTNodeType nodeType;    /// The type of AST node.
    struct ASTNode* left;    /// Pointer to the first child node (or left operand).
    struct ASTNode* right;   /// Pointer to the next sibling or right operand node.
} ASTNode;

/// Allocates and initializes a new AST node.
///
/// @param nodeType The type of the AST node.
/// @param token A pointer to the associated token (e.g., keyword, identifier, operator).
/// @return A pointer to the newly created ASTNode.
///
ASTNode* initASTNode(ASTNodeType nodeType, Token *token);

/// Recursively frees memory associated with an AST and its children.
/// @param node Pointer to the root node  of the AST (or subtree).
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
