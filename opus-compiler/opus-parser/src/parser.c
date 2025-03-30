// parser.c 
//
// Created by Boyan Fan, 2025/03/02 
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

ASTNode *parseProgram(Parser *parser, FILE *sourceCode) {
    ASTNode *root = initASTNode(AST_PROGRAM, NULL);
    ASTNode *currentNode = root;

    while (!matchTokenType(parser, TOKEN_EOF)) {
        // Skip orphan delimiters like (delimiter delimiter...)
        if (matchTokenType(parser, TOKEN_DELIMITER)) {
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
        return parseDeclaration(parser, sourceCode);
    }

    // Try to parse function definition statement 
    else if (matchTokenType(parser, TOKEN_KEYWORD_FUNC)) return parseFunctionDefinition(parser, sourceCode); 

    // Try to parse return statement for the function body
    else if (matchTokenType(parser, TOKEN_KEYWORD_RETURN)) return parseReturnStatement(parser, sourceCode);

    // Try to parse conditional statement
    else if (matchTokenType(parser, TOKEN_KEYWORD_IF)) return parseConditionalStatement(parser, sourceCode);

    // Try to parse repeat-until statement
    else if (matchTokenType(parser, TOKEN_KEYWORD_REPEAT)) return parseRepeatUntilStatement(parser, sourceCode);

    // Try to parse for-in statement
    else if (matchTokenType(parser, TOKEN_KEYWORD_FOR)) return parseForInStatement(parser, sourceCode);
    
    // Try to parse an primary expression 
    else if (isExpression(parser)) {
        return parseExpression(parser, sourceCode);
    } 

    // If unable to parse the statement
    else {
        parser->parseError = PARSE_ERROR_UNRESOLVABLE;
        parser->diagnosticToken = parser->currentToken;

        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }
}

ASTNode *parseDeclaration(Parser *parser, FILE *sourceCode) {
    // Create a node for the variable declaration statement
    ASTNode *root = (parser->currentToken->tokenType == TOKEN_KEYWORD_VAR)
        ? initASTNode(AST_VARIABLE_DECLARATION, parser->currentToken)
        : initASTNode(AST_CONSTANT_DECLARATION, parser->currentToken);
    
    // Consume the current keyword token 'var' or 'let'
    parser->currentToken = advanceParser(parser, sourceCode);

    // Then try to parse the identifier
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_IDENTIFIER;
        parser->diagnosticToken = root->token;
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Create a node for the identifier
    ASTNode *identifierNode = initASTNode(AST_IDENTIFIER, parser->currentToken);

    // Consume the current identifier token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match colon 
    if (!matchTokenType(parser, TOKEN_COLON)) {
        parser->parseError = PARSE_ERROR_MISSING_TYPE_ANNOTATION;
        parser->diagnosticToken = identifierNode->token;

        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Consume the current colon token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the type identifier
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_TYPE_NAME;
        parser->diagnosticToken = parser->currentToken;
        reportParseError(parser);        
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
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
    
    else {
        parser->parseError = PARSE_ERROR_MISSING_DELIMITER;
        parser->diagnosticToken = typeAnnotationNode->token;
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }
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
        parser->diagnosticToken = root->right->token;

        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Comsume the current delimiter token
    parser->currentToken = advanceParser(parser, sourceCode);

    return root;
}

ASTNode *parseFunctionDefinition(Parser *parser, FILE *sourceCode) {
    ASTNode *functionDefinitionNode = initASTNode(AST_FUNCTION_DEFINITION, parser->currentToken);

    // Consume the 'func' keyword
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the function name
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_FUNCTION_NAME;
        parser->diagnosticToken = functionDefinitionNode->token;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    functionDefinitionNode->left = initASTNode(AST_IDENTIFIER, parser->currentToken);

    // Consume the identifier token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the opening bracket token for the parameter list
    if (!matchTokenType(parser, TOKEN_OPENING_BRACKET)) {
        parser->parseError = PARSE_ERROR_MISSING_OPENING_BRACKET;
        parser->diagnosticToken = functionDefinitionNode->left->token;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Comsume opening bracket token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match parameter list if present 
    ASTNode *parameterListNode = initASTNode(AST_PARAMETER_LIST, NULL);

    if (!matchTokenType(parser, TOKEN_CLOSING_BRACKET)) {
        parameterListNode = parseParameterList(parser, sourceCode);
    }

    // Opus Lexer guaranteed that the opening and closing brackets match
    // Therefore we do not need to explicitly check if we could match the closing bracket
    // Once the expression be parsed, the current token is guaranteed to be a closing bracket
    // We comsume it without checking
    parser->currentToken = advanceParser(parser, sourceCode);
    
    if (!matchTokenType(parser, TOKEN_RIGHT_ARROW)) {
        parser->parseError = PARSE_ERROR_MISSING_RIGHT_ARROW;
        parser->diagnosticToken = parser->currentToken;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL); 
    }
    
    // Comsume right arrow '->' token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the function return type
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_RETURN_TYPE;
        parser->diagnosticToken = parser->currentToken;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    ASTNode *functionSignatureNode = initASTNode(AST_FUNCTION_SIGNATURE, NULL);
    functionSignatureNode->left = parameterListNode;
    functionSignatureNode->right = initASTNode(AST_FUNCTION_RETURN_TYPE, parser->currentToken);

    functionDefinitionNode->right = functionSignatureNode;

    // Comsume the current return type token 
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the function body if it is provided 
    if (matchTokenType(parser, TOKEN_OPENING_CURLY_BRACKET)) {
        ASTNode *functionImplementationNode = initASTNode(AST_FUNCTION_IMPLEMENTATION, NULL);

        functionImplementationNode->left = functionDefinitionNode;
        functionImplementationNode->right = parseCodeBlock(parser, sourceCode);

        return functionImplementationNode;
    }

    return functionDefinitionNode;
}

ASTNode *parseParameterList(Parser *parser, FILE *sourceCode) { 
    // Each parameter must be labeled, so try to match the first labeled argument 
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_PARAMETER_LABEL;
        parser->diagnosticToken = parser->currentToken;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    ASTNode *parameterListNode = initASTNode(AST_PARAMETER_LIST, NULL);
    ASTNode *parameterNode = initASTNode(AST_PARAMETER, NULL);
    ASTNode *parameterLabelNode = initASTNode(AST_PARAMETER_LABEL, parser->currentToken);

    // Comsume the current token for the parameter label
    parser->currentToken = advanceParser(parser, sourceCode);

    if (!matchTokenType(parser, TOKEN_COLON)) {
        parser->parseError = PARSE_ERROR_MISSING_COLON_AFTER_LABEL;
        parser->diagnosticToken = parameterLabelNode->token;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Consume the colon token
    parser->currentToken = advanceParser(parser, sourceCode);

    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_TYPE_NAME;
        parser->diagnosticToken = parser->currentToken;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Try to parse type annotation
    parameterNode->right = initASTNode(AST_TYPE_ANNOTATION, parser->currentToken);
    parameterNode->left = parameterLabelNode;
    parameterListNode->left = parameterNode;

    // Comsume type annotation 
    parser->currentToken = advanceParser(parser, sourceCode);

    // Check for additional parameters separated by commas 
    if (matchTokenType(parser, TOKEN_COMMA)) {
        parser->currentToken = advanceParser(parser, sourceCode);
        parameterListNode->right = parseParameterList(parser, sourceCode);
    }

    else parameterListNode->right = initASTNode(AST_PARAMETER_LIST, NULL);
    return parameterListNode;
}

ASTNode *parseCodeBlock(Parser *parser, FILE *sourceCode) {
    // Comsume the opening curly bracket
    parser->currentToken = advanceParser(parser, sourceCode);

    ASTNode *codeBlockNode = initASTNode(AST_CODE_BLOCK, NULL);
    ASTNode *currentNode = codeBlockNode;

    // Try to parse statements until we reach '}'
    while (!matchTokenType(parser, TOKEN_CLOSING_CURLY_BRACKET) && !matchTokenType(parser, TOKEN_EOF)) {
        // Skip delimiter tokens 
        if (matchTokenType(parser, TOKEN_DELIMITER)) {
            parser->currentToken = advanceParser(parser, sourceCode);
            continue;
        }

        currentNode->left = parseStatement(parser, sourceCode);
        currentNode->right = initASTNode(AST_CODE_BLOCK, NULL);
        currentNode = currentNode->right;
    }

    // Opus Lexer guaranteed that the opening and closing brackets match
    // Therefore we do not need to explicitly check if we could match the closing bracket
    // Once the expression be parsed, the current token is guaranteed to be a closing bracket
    // We comsume the closing curly bracket without checking
    parser->currentToken = advanceParser(parser, sourceCode);

    return codeBlockNode;
}

ASTNode *parseReturnStatement(Parser *parser, FILE *sourceCode) {
    ASTNode *root = initASTNode(AST_RETURN_STATEMENT, parser->currentToken);

    // Consume the 'return' keyword
    parser->currentToken = advanceParser(parser, sourceCode);
    
    // Optionally parse a return expression if the next token is not a delimiter
    if (!matchTokenType(parser, TOKEN_DELIMITER)) {
        root->left = parseExpression(parser, sourceCode);
    }
    
    // Expect a delimiter to terminate the return statement
    if (!matchTokenType(parser, TOKEN_DELIMITER)) {
        parser->parseError = PARSE_ERROR_MISSING_DELIMITER;
        parser->diagnosticToken = root->left->token;

        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }
    
    // Consume the delimiter
    parser->currentToken = advanceParser(parser, sourceCode);
    return root;
}

ASTNode *parseConditionalStatement(Parser *parser, FILE *sourceCode) {
    ASTNode *conditionalStatementNode = initASTNode(AST_CONDITIONAL_STATEMENT, parser->currentToken);

    // Consume 'if' keyword token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match a condition
    if (!isExpression(parser)) {
        parser->parseError = PARSE_ERROR_MISSING_CONDITION;
        parser->diagnosticToken = conditionalStatementNode->token;

        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }
    
    // Parse the condition expression
    ASTNode *conditionNode = parseExpression(parser, sourceCode);

    // Expect an opening curly bracket for the conditional statement body
    if (!matchTokenType(parser, TOKEN_OPENING_CURLY_BRACKET)) {
        parser->parseError = PARSE_ERROR_MISSING_OPENING_CURLY_BRACKET;
        parser->diagnosticToken = parser->currentToken;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    ASTNode *codeBlockNode = parseCodeBlock(parser, sourceCode);
    ASTNode *statementBodyNode = initASTNode(AST_CONDITIONAL_BODY, NULL);

    statementBodyNode->left = codeBlockNode;
    statementBodyNode->right = NULL;

    conditionalStatementNode->left = conditionNode;
    conditionalStatementNode->right = statementBodyNode;

    // Skip delimiters if any, since newline characters between an if-else closure are whitespaces
    while (matchTokenType(parser, TOKEN_DELIMITER)) {
        parser->currentToken = advanceParser(parser, sourceCode);
    }

    // Check if there is an else statement
    if (matchTokenType(parser, TOKEN_KEYWORD_ELSE)) {
        // Consume the 'else' keyword token 
        parser->currentToken = advanceParser(parser, sourceCode);

        // Handle an 'else if' clause recursively
        if (matchTokenType(parser, TOKEN_KEYWORD_IF)) {
            statementBodyNode->right = parseConditionalStatement(parser, sourceCode);
        }

        // Otherwise, expect an else-block
        else {
            if (!matchTokenType(parser, TOKEN_OPENING_CURLY_BRACKET)) {
                parser->parseError = PARSE_ERROR_MISSING_OPENING_CURLY_BRACKET;
                parser->diagnosticToken = parser->currentToken;
                
                reportParseError(parser);
                escapeParseError(parser, sourceCode);
                return initASTNode(AST_ERROR, NULL);
            }

            statementBodyNode->right = parseCodeBlock(parser, sourceCode);
        }
    }

    return conditionalStatementNode;
}

ASTNode *parseRepeatUntilStatement(Parser *parser, FILE *sourceCode) {
    ASTNode *repeatUntilStatementNode = initASTNode(AST_REPEAT_UNTIL_STATEMENT, parser->currentToken);

    // Consume the 'repeat' keyword toekn
    parser->currentToken = advanceParser(parser, sourceCode);
    
    // Try to mathc an opening curly bracket for the loop body.
    if (!matchTokenType(parser, TOKEN_OPENING_CURLY_BRACKET)) {
        parser->parseError = PARSE_ERROR_MISSING_OPENING_CURLY_BRACKET;
        parser->diagnosticToken = parser->currentToken;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    ASTNode *statementBodyNode = parseCodeBlock(parser, sourceCode);

    // Skip delimiters if any, since newline characters between a repeat-until closure are whitespaces
    while (matchTokenType(parser, TOKEN_DELIMITER)) {
        parser->currentToken = advanceParser(parser, sourceCode);
    }

    // Try to match the 'until' keyword token
    if (!matchTokenType(parser, TOKEN_KEYWORD_UNTIL)) {
        parser->parseError = PARSE_ERROR_MISSING_UNTIL_CONDITION;
        parser->diagnosticToken = parser->currentToken;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Consume the 'until' keyword toekn
    parser->currentToken = advanceParser(parser, sourceCode);

    if (!isExpression(parser)) {
        parser->parseError = PARSE_ERROR_MISSING_CONDITION;
        parser->diagnosticToken = parser->currentToken;

        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    repeatUntilStatementNode->left = parseExpression(parser, sourceCode);
    repeatUntilStatementNode->right = statementBodyNode;

    // Expect a delimiter to finish the repeat-until loop.
    if (!matchTokenType(parser, TOKEN_DELIMITER) && !matchTokenType(parser, TOKEN_EOF)) {
        parser->parseError = PARSE_ERROR_MISSING_DELIMITER;
        parser->diagnosticToken = repeatUntilStatementNode->left->token;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    parser->currentToken = advanceParser(parser, sourceCode);
    return repeatUntilStatementNode;
}

ASTNode *parseForInStatement(Parser *parser, FILE *sourceCode) {
    ASTNode *forInStatementNode = initASTNode(AST_FOR_IN_STATEMENT, parser->currentToken);

    // Consume the 'for' keyword token
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the identifier for the loop variable
    if (!matchTokenType(parser, TOKEN_IDENTIFIER)) {
        parser->parseError = PARSE_ERROR_MISSING_IDENTIFIER;
        parser->diagnosticToken = forInStatementNode->token;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    ASTNode *identifierNode = initASTNode(AST_IDENTIFIER, parser->currentToken);
    parser->currentToken = advanceParser(parser, sourceCode);

    // Try to match the 'in' keyword token
    if (!matchTokenType(parser, TOKEN_KEYWORD_IN)) {
        parser->parseError = PARSE_ERROR_MISSING_IN_STATEMENT;
        parser->diagnosticToken = identifierNode->token;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Consume 'in' keyword token
    parser->currentToken = advanceParser(parser, sourceCode);
    
    if (!isExpression(parser)) {
        parser->parseError = PARSE_ERROR_MISSING_IDENTIFIER;
        parser->diagnosticToken = parser->currentToken;

        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    ASTNode *iterableNode = parseExpression(parser, sourceCode);

    // Expect an opening curly bracket for the loop body.
    if (!matchTokenType(parser, TOKEN_OPENING_CURLY_BRACKET)) {
        parser->parseError = PARSE_ERROR_MISSING_OPENING_CURLY_BRACKET;
        parser->diagnosticToken = iterableNode->token;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }
    
    ASTNode *forInContextNode = initASTNode(AST_FOR_IN_CONTEXT, NULL);
    forInContextNode->left = identifierNode;
    forInContextNode->right = iterableNode;

    ASTNode *statementBodyNode = parseCodeBlock(parser, sourceCode);
    forInStatementNode->left = forInContextNode;
    forInStatementNode->right = statementBodyNode;

    return forInStatementNode;
}

ASTNode *parseExpression(Parser *parser, FILE *sourceCode) {
    // Entry point for expression parsing, we start at the lowest precedence level (logical equivalence)
    return parseLogicalOr(parser, sourceCode);
}

ASTNode *parseLogicalOr(Parser *parser, FILE *sourceCode) {
    // Where logical and has higher precedence than the logical or, so try to parse it first
    ASTNode *root = parseLogicalAnd(parser, sourceCode);

    // Try to match logical or 
    while (matchTokenType(parser, TOKEN_LOGICAL_OR_OPERATOR)) {
        ASTNode *binaryNode = initASTNode(AST_BINARY_EXPRESSION, parser->currentToken);

        // Comsume the current operator token ('||')
        parser->currentToken = advanceParser(parser, sourceCode);

        binaryNode->left = root;
        binaryNode->right = parseLogicalAnd(parser, sourceCode);
        root = binaryNode;
    }

    return root;
}

ASTNode *parseLogicalAnd(Parser *parser, FILE *sourceCode) {
    ASTNode *root = parseComparison(parser, sourceCode);

    // Try to match logical and 
    while (matchTokenType(parser, TOKEN_LOGICAL_AND_OPERATOR)) {
        ASTNode *binaryNode = initASTNode(AST_BINARY_EXPRESSION, parser->currentToken);

        // Comsume the current operator token ('&&')
        parser->currentToken = advanceParser(parser, sourceCode);

        binaryNode->left = root;
        binaryNode->right = parseComparison(parser, sourceCode);
        root = binaryNode;
    }

    return root;
}

ASTNode *parseComparison(Parser *parser, FILE *sourceCode) {
    // Where addition expressions have higher precedence than relational operators,
    // so try to parse the addition expression first.
    ASTNode *root = parseAddition(parser, sourceCode);

    // Try to match any relational operator: '<', '>', '<=', '>='
    while (matchTokenType(parser, TOKEN_LESS_THAN_OPERATOR)    ||
           matchTokenType(parser, TOKEN_GREATER_THAN_OPERATOR) ||
           matchTokenType(parser, TOKEN_LESS_OR_EQUAL_TO_OPERATOR)   ||
           matchTokenType(parser, TOKEN_GREATER_OR_EQUAL_TO_OPERATOR) ||
           matchTokenType(parser, TOKEN_LOGICAL_EQUIVALENCE) ||
           matchTokenType(parser, TOKEN_NOT_EQUAL_TO_OPERATOR)) {

        ASTNode *binaryNode = initASTNode(AST_BINARY_EXPRESSION, parser->currentToken);

        // Consume the current operator token ('<', '>', '<=', or '>=')
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
        
        // Try if match assignment operator if there is an assignment statement after the declaration
        if (matchTokenType(parser, TOKEN_ASSIGNMENT_OPERATOR)) { 
            // The root now should be an assignment statement and the declaration is its left value
            return parseAssignmentStatement(parser, sourceCode, root);
        }

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
    parser->parseError = PARSE_ERROR_MISSING_OPERAND;
    parser->diagnosticToken = parser->currentToken;

    reportParseError(parser);
    escapeParseError(parser, sourceCode);
    return initASTNode(AST_ERROR, NULL);
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
        parser->diagnosticToken = parser->currentToken;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    ASTNode *argumentListNode = initASTNode(AST_ARGUMENT_LIST, NULL);
    ASTNode *argumentNode = initASTNode(AST_ARGUMENT, NULL);
    ASTNode *argumentLabelNode = initASTNode(AST_ARGUMENT_LABEL, parser->currentToken);

    // Comsume the current token for the argument label
    parser->currentToken = advanceParser(parser, sourceCode);

    if (!matchTokenType(parser, TOKEN_COLON)) {
        parser->parseError = PARSE_ERROR_MISSING_COLON_AFTER_LABEL;
        parser->diagnosticToken = argumentLabelNode->token;
        
        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

    // Consume the colon token
    parser->currentToken = advanceParser(parser, sourceCode);

    if (!isExpression(parser)) {
        parser->parseError = PARSE_ERROR_MISSING_ARGUMENT;
        parser->diagnosticToken = parser->currentToken;

        reportParseError(parser);
        escapeParseError(parser, sourceCode);
        return initASTNode(AST_ERROR, NULL);
    }

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

void escapeParseError(Parser *parser, FILE *sourceCode) {
    while (!matchTokenType(parser, TOKEN_DELIMITER) && !matchTokenType(parser, TOKEN_EOF)) {
        parser->currentToken = advanceParser(parser, sourceCode);
    }
}

int isExpression(Parser *parser) {
    return matchTokenType(parser, TOKEN_IDENTIFIER) || matchTokenType(parser, TOKEN_NUMERIC) ||
           matchTokenType(parser, TOKEN_STRING_LITERAL) || matchTokenType(parser, TOKEN_ARITHMETIC_SUBTRACTION) ||
           matchTokenType(parser, TOKEN_LOGICAL_NEGATION) || matchTokenType(parser, TOKEN_OPENING_BRACKET) ||
           matchTokenType(parser, TOKEN_KEYWORD_TRUE) || matchTokenType(parser, TOKEN_KEYWORD_FALSE);
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
    parser->diagnosticToken = NULL;

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

    /* Extension ASTNode where fields added for semantic analysis */ 
    strcpy(node->inferredType, "Any");
    node->isFoldable = 1;

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
        case AST_PROGRAM:                   printf("AST_PROGRAM\n"); break;
        case AST_VARIABLE_DECLARATION:      printf("AST_VARIABLE_DECLARATION (%s)\n", node->token->lexeme); break;
        case AST_CONSTANT_DECLARATION:      printf("AST_CONSTANT_DECLARATION (%s)\n", node->token->lexeme); break;
        case AST_IDENTIFIER:                printf("AST_IDENTIFIER (%s)\n", node->token->lexeme); break;
        case AST_TYPE_ANNOTATION:           printf("AST_TYPE_ANNOTATION (%s)\n", node->token->lexeme); break;
        case AST_ASSIGNMENT_STATEMENT:      printf("AST_ASSIGNMENT (%s)\n", node->token->lexeme); break;
        case AST_LITERAL:                   printf("AST_LITERAL (%s)\n", node->token->lexeme); break;
        case AST_BOOLEAN_LITERAL:           printf("AST_BOOLEAN_LITERAL (%s)\n", node->token->lexeme); break;
        case AST_BINARY_EXPRESSION:         printf("AST_BINARY_EXPRESSION (%s)\n", node->token->lexeme); break;
        case AST_UNARY_EXPRESSION:          printf("AST_UNARY_EXPRESSION (%s)\n", node->token->lexeme); break;
        case AST_POSTFIX_EXPRESSION:        printf("AST_POSTFIX_EXPRESSION (%s)\n", node->token->lexeme); break;
        case AST_FUNCTION_CALL:             printf("AST_FUNCTION_CALL\n"); break;
        case AST_ARGUMENT:                  printf("AST_ARGUMENT\n"); break;
        case AST_ARGUMENT_LABEL:            printf("AST_ARGUMENT_LABEL (%s)\n", node->token->lexeme); break;
        case AST_ARGUMENT_LIST:             printf("AST_ARGUMENT_LIST\n"); break;
        case AST_FUNCTION_DEFINITION:       printf("AST_FUNCTION_DEFINITION (%s)\n", node->token->lexeme); break;
        case AST_FUNCTION_SIGNATURE:        printf("AST_FUNCTION_SIGNATURE\n"); break;
        case AST_PARAMETER_LIST:            printf("AST_PARAMETER_LIST\n"); break;
        case AST_PARAMETER_LABEL:           printf("AST_PARAMETER_LABEL (%s)\n", node->token->lexeme); break;
        case AST_FUNCTION_RETURN_TYPE:      printf("AST_FUNCTION_RETURN_TYPE (%s)\n", node->token->lexeme); break;
        case AST_FUNCTION_IMPLEMENTATION:   printf("AST_FUNCTION_IMPLEMENTATION\n"); break;
        case AST_CODE_BLOCK:                printf("AST_CODE_BLOCK\n"); break;
        case AST_PARAMETER:                 printf("AST_PARAMETER\n"); break;
        case AST_RETURN_STATEMENT:          printf("AST_RETURN_STATEMENT (%s)\n", node->token->lexeme); break;
        case AST_CONDITIONAL_STATEMENT:     printf("AST_CONDITIONAL_STATEMENT (%s)\n", node->token->lexeme); break;
        case AST_CONDITIONAL_BODY:          printf("AST_CONDITIONAL_BODY\n"); break;
        case AST_REPEAT_UNTIL_STATEMENT:    printf("AST_REPEAT_UNTIL_STATEMENT (%s)\n", node->token->lexeme); break;
        case AST_FOR_IN_STATEMENT:          printf("AST_FOR_IN_STATEMENT (%s)\n", node->token->lexeme); break;
        case AST_FOR_IN_CONTEXT:            printf("AST_FOR_IN_CONTEXT\n"); break;
        case AST_ERROR:                     printf("AST_ERROR (x)\n"); break;
        default:                            printf("UNKNOWN NODE\n"); break;
    }

    if (node->left) displayAST(node->left, level + 1);
    if (node->right) displayAST(node->right, level + 1);
}

void reportParseError(Parser *parser) {
    Token *token = parser->diagnosticToken;
    printf("Parsing Error at %d:%d\n", token->location.line, token->location.column);

    // Return if there is no error to display
    if (parser->parseError == PARSE_ERROR_NONE) return;

    switch (parser->parseError) {
        case PARSE_ERROR_MISSING_IDENTIFIER:
            printf("[ERROR] Expecting a name for the variable/constant after '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_TYPE_ANNOTATION:
            printf("[ERROR] Expecting ':' for the type annotation after '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_TYPE_NAME:
            printf("[ERROR] Expecting a type name after ':'.\n"); break;
        case PARSE_ERROR_DECLARATION_SYNTAX:
            printf("[ERROR] Expecting '=' or a newline after '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_RIGHT_VALUE:
            printf("[ERROR] Expecting something to be assigned to '%s' after '='.\n", token->lexeme); break;
        case PARSE_ERROR_UNRESOLVABLE:
            printf("[ERROR] Unresolvable token for token '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_ARGUMENT_LABEL:
            printf("[ERROR] Expecting label for argument %s in the function call.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_COLON_AFTER_LABEL:
            printf("[ERROR] Expecting ':' after the label '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_FUNCTION_NAME:
            printf("[ERROR] Expecting a name for the function after '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_OPENING_BRACKET:
            printf("[ERROR] Expecting '(' for defining parameter list after '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_RIGHT_ARROW:
            printf("[ERROR] Expecting '->' after ')' for function return type annotation.\n"); break;
        case PARSE_ERROR_MISSING_RETURN_TYPE:
            printf("[ERROR] Expecting a type name after '->'.\n"); break;
        case PARSE_ERROR_MISSING_OPENING_CURLY_BRACKET:
            printf("[ERROR] Expecting '{' to provide a body for the statement.\n"); break;
        case PARSE_ERROR_MISSING_UNTIL_CONDITION:
            printf("[ERROR] Expecting 'until' to provide a termination condition.\n"); break;
        case PARSE_ERROR_MISSING_IN_STATEMENT:
            printf("[ERROR] Expecting 'in' to provide an Iterable after '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_DELIMITER:
            printf("[ERROR] Expecting a newline after '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_CONDITION:
            printf("[ERROR] Expecting a condition after '%s'.\n", token->lexeme); break;
        case PARSE_ERROR_MISSING_OPERAND:
            printf("[ERROR] Expecting another operand.\n"); break;
        case PARSE_ERROR_MISSING_ARGUMENT:
            printf("[ERROR] Expecting an argument after ':'.\n"); break;
        default:
            printf("[ERROR] Unable to generate diagnostic information...\n");
    }
}
