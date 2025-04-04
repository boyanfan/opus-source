// parser.h 
//
// Parser for the Opus Programming Language using a top-down recursive descent method.
// It parses a sequence of tokens into an Abstract Syntax Tree (AST). The AST is then used for semantic 
// analysis, optimization, and code generation.
//
// Created by Boyan Fan, 2025/03/02
// 

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "lexer.h"

/// Error codes for parsing.
typedef enum {
    PARSE_ERROR_NONE,                            /// No error occurred during parsing.
    PARSE_ERROR_MISSING_IDENTIFIER,              /// A required identifier is missing.
    PARSE_ERROR_MISSING_TYPE_ANNOTATION,         /// A required type annotaion is missing.
    PARSE_ERROR_MISSING_TYPE_NAME,               /// A required type name is missing.
    PARSE_ERROR_MISSING_DELIMITER,               /// A required delimiter is missing.
    PARSE_ERROR_DECLARATION_SYNTAX,              /// Invalid declaration syntax.
    PARSE_ERROR_MISSING_RIGHT_VALUE,             /// A required right value to be assigned is missing.
    PARSE_ERROR_MISSING_ARGUMENT_LABEL,          /// A required argument label is missing.
    PARSE_ERROR_MISSING_PARAMETER_LABEL,         /// A required parameter label.
    PARSE_ERROR_MISSING_COLON_AFTER_LABEL,       /// A required colon after the label is missing.
    PARSE_ERROR_MISSING_FUNCTION_NAME,           /// A required function name is missing.
    PARSE_ERROR_MISSING_OPENING_BRACKET,         /// A required opening bracket is missing.
    PARSE_ERROR_MISSING_RIGHT_ARROW,             /// A required right arrow is missing.
    PARSE_ERROR_MISSING_RETURN_TYPE,             /// A required return type is missing.
    PARSE_ERROR_MISSING_OPENING_CURLY_BRACKET,   /// A required curly bracket is missing.
    PARSE_ERROR_MISSING_UNTIL_CONDITION,         /// A required until condition is missing.
    PARSE_ERROR_MISSING_IN_STATEMENT,            /// A required 'in' keyword is missing.
    PARSE_ERROR_MISSING_CONDITION,               /// A required condition for the statement is missing.
    PARSE_ERROR_UNRESOLVABLE,                    /// An unresolvable token occurred.
    PARSE_ERROR_MISSING_OPERAND,                 /// A required operand is missing.
    PARSE_ERROR_MISSING_ARGUMENT,                /// A required argument is missing.
} ParseError;

/// The parser for the Opus programming language.
/// It processes tokens from the lexer and constructs an Abstract Syntax Tree (AST).
typedef struct {
    ParseError parseError;    /// Stores the current parsing error state, if any.
    Lexer* lexer;             /// Pointer to the lexer instance responsible for tokenizing input.
    Token* currentToken;      /// Pointer to the current token being processed by the parser.
    Token* diagnosticToken;   /// Pointer to the previous token for generating diagnostic information.
} Parser;

/// Parses a Program in the Opus programming language.
///
/// This function serves as the entry point for parsing an Opus program. It constructs
/// an Abstract Syntax Tree (AST) representing the entire program by parsing a sequence
/// of statements. The function follows the grammar:
///
///     Program -> Statement Delimiter | Program
///
/// The resulting AST structure will be:
///
///     AST_PROGRAM
///     ├── AST_STATEMENT
///     ╰── AST_PROGRAM
///         ├── AST_STATEMENT (Statement 1)
///         ╰── AST_PROGRAM
///             ...
///             ├── AST_STATEMENT (Last Statement)
///             ╰── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed Program, or NULL if a parsing error occurs.
///
ASTNode *parseProgram(Parser *parser, FILE *sourceCode);

/// Parses a Statement in the Opus programming language.
///
/// This function determines the type of statement based on the current token and
/// delegates parsing to the appropriate function. It follows the grammar:
///
///     Statement -> VariableDeclaration
///                | AssignmentStatement
///                | ...
///
/// The resulting Abstract Syntax Tree (AST) will have a node representing the detected
/// statement type, with its corresponding children forming the substructure.
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed Statement, or NULL if a parsing error occurs.
///
ASTNode *parseStatement(Parser *parser, FILE *sourceCode);

/// Parses a Declaration statement in the Opus programming language.
///
/// This function follows the grammar:
///     Declaration -> ("var" | "let") Identifier ":" Type
///
/// It constructs an Abstract Syntax Tree (AST) node representing the variable declaration.
/// The resulting AST structure will be:
///
///     AST_VARIABLE_DECLARATION ("var" or "let")
///         ├── AST_IDENTIFIER ("variable_name")
///         └── AST_TYPE ("type_name")
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the VariableDeclaration, or NULL if a parsing error occurs.
///
ASTNode *parseDeclaration(Parser *parser, FILE *sourceCode);

/// Parses an assignment statement and constructs the corresponding AST node. It takes the left-value, which 
/// can be either an identifier or a declaration statement, verifies the assignment syntax, and processes 
/// the right-hand side (RHS) expression to construct an `ASTNode` representing the assignment. 
/// The assignment statement follows the grammar:
///
///     AssignmentStatement -> LeftValue '=' Expression Delimiter
///     LeftValue           -> Identifier | DeclarationStatement
///     Expression          -> Literal
///                         |  Identifier
///                         |  FunctionCall
///                         |  BinaryExpression
///                         |  UnaryExpression
///                         |  ...
///
/// An example of the resulting AST structure for 'var number: Int = 42' will be:
///     
///     AST_ASSIGNMENT (=)
///     │   ├── AST_VARIABLE_DECLARATION (var)
///     │   │   ├── AST_IDENTIFIER (number)
///     │   │   ├── AST_TYPE_ANNOTATION (Int)
///     │   ├── AST_LITERAL (42)
///
/// @param parser Pointer to the `Parser` instance that maintains the parsing state.
/// @param sourceCode Pointer to the source code file being parsed.
/// @param leftValue Pointer to an `ASTNode` representing the left-value, which could be an identifier or 
///                  a declaration statement.
/// @return Pointer to an `ASTNode` representing the assignment statement in the AST. Returns `NULL` 
///         if an error occurs during parsing.
///
ASTNode *parseAssignmentStatement(Parser *parser, FILE *sourceCode, ASTNode *leftValue);

/// Parses a function definition in the Opus programming language.
///
/// This function handles function definitions, which include the function signature and optional body.
/// It follows the grammar:
///
///     FunctionDefinition -> "func" Identifier "(" ParameterList? ")" "->" ReturnType
///
/// The resulting AST structure for a function definition ("func greeting() -> String") will be:
///
///     ├── AST_FUNCTION_DEFINITION (func)
///     │   ├── AST_IDENTIFIER (greeting)
///     │   ├── AST_FUNCTION_SIGNATURE
///     │   │   ├── AST_PARAMETER_LIST
///     │   │   ├── AST_FUNCTION_RETURN_TYPE (String)
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed function definition, or NULL if a parsing error occurs.
///
ASTNode *parseFunctionDefinition(Parser *parser, FILE *sourceCode);

/// Parses a parameter list for a function in the Opus programming language.
///
/// This function parses the parameters for a function definition and builds the AST for the parameter list.
/// It follows the grammar:
///
///     ParameterList -> LabeledParameter (',' LabeledParameter)*
///     LabeledParameter -> Identifier ':' Type
///
/// The resulting AST structure will be:
///
/// AST_PARAMETER_LIST
///    ├── AST_PARAMETER
///    │   ├── AST_PARAMETER_LABEL (firstArg)
///    │   ├── AST_TYPE_ANNOTATION (Int)
///    ├── AST_PARAMETER_LIST
///    │   ├── AST_PARAMETER
///    │   │   ├── AST_PARAMETER_LABEL (secondArg)
///    │   │   ├── AST_TYPE_ANNOTATION (String)
///    │   ├── AST_PARAMETER_LIST
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed parameter list.
///
ASTNode *parseParameterList(Parser *parser, FILE *sourceCode);

/// Parses a code block in the Opus programming language.
///
/// This function handles the parsing of statements inside a block, typically between opening and closing
/// curly braces (`{}`), and constructs the AST for the block. It follows the grammar:
///
///     CodeBlock -> '{' Statement '}'
///
/// The resulting AST structure for a code block will be:
///
///     AST_CODE_BLOCK
///         ├── AST_STATEMENT
///         ├── AST_CODE_BLOCK
/// 
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed code block.
///
ASTNode *parseCodeBlock(Parser *parser, FILE *sourceCode);

/// Parses a return statement in the Opus programming language.
///
/// This function handles return statements, which optionally include an expression.
/// It follows the grammar:
///
///     ReturnStatement -> "return" Expression? Delimiter
///
/// The resulting AST structure for a return statement ("return 1 + 2") will be:
/// 
///     AST_PROGRAM
///     ├── AST_RETURN_STATEMENT (return)
///     │   ├── AST_BINARY_EXPRESSION (+)
///     │   │   ├── AST_LITERAL (1)
///     │   │   ├── AST_LITERAL (2)
///     ├── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed return statement.
///
ASTNode *parseReturnStatement(Parser *parser, FILE *sourceCode);

/// Parses a conditional statement (if-else) in the Opus programming language.
///
/// This function handles `if` statements with optional `else if` and `else` branches.
/// It follows the grammar:
///
///     ConditionalStatement -> "if" Expression CodeBlock ("else if" Expression CodeBlock)* ("else" CodeBlock)?
///
/// The resulting AST structure for an `if-else` statement will be:
/// 
///     AST_PROGRAM
///     ├── AST_CONDITIONAL_STATEMENT (if)
///     │   ├── AST_BOOLEAN_LITERAL (true)
///     │   ├── AST_CONDITIONAL_BODY
///     │   │   ├── AST_CODE_BLOCK (if-execution)
///     │   │   ├── AST_CODE_BLOCK (else-execution)
///     ├── AST_PROGRAM
/// 
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed conditional statement.
///
ASTNode *parseConditionalStatement(Parser *parser, FILE *sourceCode);

/// Parses a repeat-until loop in the Opus programming language.
///
/// This function handles `repeat-until` loops, which execute the loop body at least once
/// before checking the termination condition. It follows the grammar:
///
///     RepeatUntilStatement -> "repeat" CodeBlock "until" Expression Delimiter
///
/// The resulting AST structure for a `repeat-until` loop will be:
/// 
///     AST_PROGRAM
///     ├── AST_REPEAT_UNTIL_STATEMENT (repeat)
///     │   ├── AST_BOOLEAN_EXPRESSION
///     │   ├── AST_CODE_BLOCK
///     ├── AST_PROGRAM
/// 
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed repeat-until loop.
///
ASTNode *parseRepeatUntilStatement(Parser *parser, FILE *sourceCode);

/// Parses a for-in loop in the Opus programming language.
///
/// This function handles `for-in` loops, which iterate over a collection.
/// It follows the grammar:
///
///     ForInStatement -> "for" Identifier "in" Expression CodeBlock
///
/// The resulting AST structure for a `for-in` loop will be:
///
///     AST_PROGRAM
///     ├── AST_FOR_IN_STATEMENT (for)
///     │   ├── AST_FOR_IN_CONTEXT
///     │   │   ├── AST_IDENTIFIER (number)
///     │   │   ├── AST_IDENTIFIER (numbers)
///     │   ├── AST_CODE_BLOCK
///     ├── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed for-in loop.
///
ASTNode *parseForInStatement(Parser *parser, FILE *sourceCode);

/// Parses an expression in the Opus programming language.
///
/// This function serves as the entry point for parsing expressions. It constructs an
/// Abstract Syntax Tree (AST) that represents various types of expressions, including
/// arithmetic, boolean, and function calls. It follows the grammar:
///
///     Expression -> LogicalOr
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed expression, or NULL if a parsing error occurs.
///
ASTNode *parseExpression(Parser *parser, FILE *sourceCode);

/// Parses a logical OR expression in the Opus programming language.
///
/// This function handles logical OR operations (`||`) and constructs an AST node
/// representing the binary expression. The function follows the grammar:
///
///     LogicalOr -> LogicalAnd "||" LogicalAnd
/// 
/// The resulting AST structure will be:
///
///     AST_PROGRAM
///     ├── AST_BINARY_EXPRESSION (||)
///     │   ├── AST_IDENTIFIER (lsh)
///     │   ├── AST_IDENTIFIER (rhs)
///     ├── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed logical OR expression.
///
ASTNode *parseLogicalOr(Parser *parser, FILE *sourceCode);

/// Parses a logical AND expression in the Opus programming language.
///
/// This function processes logical AND operations (`&&`) and ensures proper
/// precedence by deferring to lower precedence expressions. The function follows
/// the grammar:
///
///     LogicalAnd -> Comparison "&&" Comparison
///
/// The resulting AST structure will be:
///
///     AST_PROGRAM
///     ├── AST_BINARY_EXPRESSION (&&)
///     │   ├── AST_IDENTIFIER (lsh)
///     │   ├── AST_IDENTIFIER (rhs)
///     ├── AST_PROGRAM
///     
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed logical AND expression.
///
ASTNode *parseLogicalAnd(Parser *parser, FILE *sourceCode);

/// Parses a relational expression ('>', '<', '>=', '<=', '==' and '!=').
///
/// This function parses relational expressions that compare two operands 
/// using relational operators (`>`, `<`, `>=`, `<=`). It constructs and 
/// returns an `ASTNode` representing the relational operation in the 
/// abstract syntax tree (AST). The function follows the grammar:
///
///     Relational -> Addition (">" | "<" | ">=" | "<=" | "==" | "!=") Addition
///
/// The resulting AST structure for "42 >= 3.14 + 1" will be:
///
///     AST_PROGRAM
///     ├── AST_BINARY_EXPRESSION (>=)
///     │   ├── AST_LITERAL (42)
///     │   ├── AST_BINARY_EXPRESSION (+)
///     │   │   ├── AST_LITERAL (3.14)
///     │   │   ├── AST_LITERAL (1)
///     ├── AST_PROGRAM
/// 
/// @param parser Pointer to the `Parser` structure maintaining the current parsing state.
/// @param sourceCode The file pointer to the source code being parsed, used for token advancement.
/// @return A pointer to an `ASTNode` representing the parsed relational expression.
///
ASTNode *parseComparison(Parser *parser, FILE *sourceCode);

/// Parses an addition or subtraction expression in the Opus programming language.
///
/// This function handles addition (`+`) and subtraction (`-`) operations, ensuring
/// left-associativity and proper precedence over multiplication. It follows the grammar:
///
///     Addition -> Multiplication ("+" | "-") Multiplication
///
/// The resulting AST structure for ("1 + 2 * 3") will be:
///
///     AST_PROGRAM
///     ├── AST_BINARY_EXPRESSION (+)
///     │   ├── AST_LITERAL (1)
///     │   ├── AST_BINARY_EXPRESSION (*)
///     │   │   ├── AST_LITERAL (2)
///     │   │   ├── AST_LITERAL (3)
///     ├── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed addition or subtraction expression.
///
ASTNode *parseAddition(Parser *parser, FILE *sourceCode);

/// Parses a multiplication, division, or modulo expression in the Opus programming language.
///
/// This function handles multiplication (`*`), division (`/`), and modulo (`%`)
/// operations, ensuring left-associativity and higher precedence than addition and
/// subtraction. It follows the grammar:
///
///     Multiplication -> Prefix ("*" | "/" | "%") Prefix
///
/// The resulting AST structure for ("1 + 2 * 3") will be:
///
///     AST_PROGRAM
///     ├── AST_BINARY_EXPRESSION (+)
///     │   ├── AST_LITERAL (1)
///     │   ├── AST_BINARY_EXPRESSION (*)
///     │   │   ├── AST_LITERAL (2)
///     │   │   ├── AST_LITERAL (3)
///     ├── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed multiplication, division, or modulo expression.
///
ASTNode *parseMultiplication(Parser *parser, FILE *sourceCode);

/// Parses a prefix unary expression in the Opus programming language.
///
/// This function handles unary operations such as negation (`-`) and logical NOT (`!`).
/// It ensures correct right-associativity and higher precedence than multiplication.
/// The function follows the grammar:
///
///     Prefix -> ("-" | "!") Prefix | Postfix 
///
/// The resulting AST structure will be:
///
///     AST_PROGRAM
///     ├── AST_UNARY_EXPRESSION (!)
///     │   ├── AST_IDENTIFIER (condition)
///     ├── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed prefix unary expression.
///
ASTNode *parsePrefix(Parser *parser, FILE *sourceCode);

/// Parses a postfix expression in the Opus programming language.
///
/// This function handles postfix operations such as the factorial operator (`!`)
/// and function calls. It follows the grammar:
///
///     Postfix -> Primary ("!" | FunctionCall)
///
/// The resulting AST structure will be:
///
///     AST_PROGRAM
///     ├── AST_POSTFIX_EXPRESSION (!)
///     │   ├── AST_LITERAL (42)
///     ├── AST_PROGRAM
/// 
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed postfix expression.
///
ASTNode *parsePostfix(Parser *parser, FILE *sourceCode);

/// Parses a primary expression in the Opus programming language.
///
/// This function handles literals, identifiers, boolean values, and parenthesized expressions.
/// It follows the grammar:
///
///     Primary -> NUMERIC
///             | STRING_LITERAL
///             | IDENTIFIER
///             | BOOLEAN_LITERAL
///             | '(' Expression ')'
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed primary expression.
///
ASTNode *parsePrimary(Parser *parser, FILE *sourceCode);

/// Parses a function call in the Opus programming language.
///
/// This function handles function calls, ensuring that each argument is labeled.
/// It follows the grammar:
///
///     FunctionCall -> IDENTIFIER '(' ArgumentList? ')'
///
/// The resulting AST structure will be:
///
///     AST_FUNCTION_CALL
///         ├── AST_IDENTIFIER (function name)
///         └── AST_ARGUMENT_LIST (arguments)
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @param callee A pointer to an ASTNode representing the function name.
/// @return A pointer to the ASTNode representing the function call expression.
///
ASTNode* parseFunctionCall(Parser *parser, FILE *sourceCode, ASTNode* callee);

/// Parses an argument list for a function call in the Opus programming language.
///
/// This function ensures that all arguments are labeled, following the grammar:
///
///     ArgumentList -> LabeledArgument ( ',' ArgumentList )?
///     LabeledArgument -> IDENTIFIER ':' Expression
///
/// The resulting AST structure for "getRandomInt(between: 1, and: 100)" will be:
///
///     AST_PROGRAM
///     ├── AST_FUNCTION_CALL
///     │   ├── AST_IDENTIFIER (getRandomInt)
///     │   ├── AST_ARGUMENT_LIST
///     │   │   ├── AST_ARGUMENT
///     │   │   │   ├── AST_ARGUMENT_LABEL (between)
///     │   │   │   ├── AST_LITERAL (1)
///     │   │   ├── AST_ARGUMENT_LIST
///     │   │   │   ├── AST_ARGUMENT
///     │   │   │   │   ├── AST_ARGUMENT_LABEL (and)
///     │   │   │   │   ├── AST_LITERAL (100)
///     │   │   │   ├── AST_ARGUMENT_LIST
///     ├── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed argument list.
///
ASTNode* parseArgumentList(Parser *parser, FILE *sourceCode);

/// Checks if the current token matches the expected token type.
///
/// @param parser Pointer to the Parser instance.
/// @param type The expected token type to check against.
/// @return Ture (1) if the current token matches the expected type, False (0) otherwise.
///
int matchTokenType(Parser *parser, TokenType type);

/// Advances the parser to the next token in the source code.
///
/// @param parser Pointer to the Parser instance.
/// @param sourceCode Pointer to the source file being parsed.
/// @return Pointer to the newly retrieved Token from the lexer.
///
Token *advanceParser(Parser *parser, FILE *sourceCode); 

/// Skips tokens until a delimiter is encountered, enabling error recovery.
/// This function advances the parser until a `TOKEN_DELIMITER` is found, 
/// allowing parsing to resume at a safe synchronization point. It is typically 
/// used for error recovery to prevent cascading errors caused by unexpected tokens.
/// 
/// @param parser Pointer to the `Parser` structure, which maintains the current parsing state.
/// @param sourceCode The file pointer to the source code being parsed, used for fetching new tokens.
///
void escapeParseError(Parser *parser, FILE *sourceCode);

/// Reports the current parsing error with diagnostic information.
/// @param parser Pointer to the Parser instance.
///
void reportParseError(Parser *parser);

/// Checks if the current token represents the start of an expression.
///
/// This function determines whether the parser's current token can begin 
/// an expression. Expressions can start with identifiers, numeric literals, 
/// string literals, unary operators (`-`, `!`), opening brackets `(` for 
/// grouping, or boolean keywords (`true`, `false`).
///
/// @param parser Pointer to the `Parser` structure containing the current parsing state.
/// @return `1` (True) if the current token can start an expression, otherwise `0` (False).
///
int isExpression(Parser *parser);

/// Initializes a new parser instance.
/// @return Pointer to a newly allocated Parser instance, or NULL if memory allocation fails.
///
Parser *initParser();

#endif
