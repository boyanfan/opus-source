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
    AST_PROGRAM,                   /// Root node of the entire program.
    AST_VARIABLE_DECLARATION,      /// Variable declaration node (e.g. "var number: Int").
    AST_CONSTANT_DECLARATION,      /// Constant declaration node (e.g. "let name: String").
    AST_IDENTIFIER,                /// Identifier node (e.g. number).
    AST_LITERAL,                   /// Literals node (e.g. 3.14, "Boyan Fan")
    AST_BOOLEAN_LITERAL,           /// Boolean literals (e.g. "true", "flase")
    AST_TYPE_ANNOTATION,           /// Type annotations (e.g. number: Int).
    AST_ASSIGNMENT_STATEMENT,      /// Assignment statement node (e.g. "number = 5").
    AST_BINARY_EXPRESSION,         /// Binary expressions (e.g. "1 + 1")
    AST_UNARY_EXPRESSION,          /// Unary expressions (e.g. "!condition")
    AST_POSTFIX_EXPRESSION,        /// Postfix expressions (e.g. "5!")
    AST_FUNCTION_CALL,             /// Function call (e.g. "abs(-3.14)")
    AST_ARGUMENT,                  /// Argument label and its value.
    AST_ARGUMENT_LABEL,            /// The label of the argument.
    AST_ARGUMENT_LIST,             /// Argument list for the function call.
    AST_FUNCTION_IMPLEMENTATION,   /// Function implementation. 
    AST_FUNCTION_DEFINITION,       /// Function definitions (e.g. "func isPrime(number: Int) -> Bool").
    AST_FUNCTION_SIGNATURE,        /// Function signature (e.g. "for the above example: (Int) -> Bool").
    AST_PARAMETER,                 /// Parameter label and its type.
    AST_PARAMETER_LIST,            /// Function parameter list (e.g. "(number: Int, ...)")
    AST_PARAMETER_LABEL,           /// The label of the parameter.
    AST_FUNCTION_RETURN_TYPE,      /// Function return type (e.g. "-> Bool").
    AST_CODE_BLOCK,                /// A code block (e.g. "{...}").
    AST_RETURN_STATEMENT,          /// Return statement for the function body.
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
