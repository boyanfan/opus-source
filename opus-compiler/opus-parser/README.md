# Opus Parser Implementation

The Opus Parser is implemented as a top-down recursive descent parser, 
meaning it processes the input source code by breaking it down according
to the grammatical rules of the Opus programming language. At the 
highest level, the parser begins with the `parseProgram()` function, 
which sequentially processes statements in the source code. 

## Design Considerations
For the best convenience of team work, all parser functions are designed
in the same format with lexer functions. Let's see a few examples - both
Opus Lexer and Opus parser provides a single function to get or move 
to the next token, and both functions have similar signatures.

```C 
Token *getNextToken(Lexer *lexer, FILE* sourceCode);
Token *advanceParser(Parser *parser, FILE *sourceCode); 
```
There is a minor upgrade for Opus language - we make parentheses for 
conditions optional! That means, for `if` and `until` statements, 
the following two are completely equivalent, so the programmer can
write codes with their own styles.

```opus
if (condition) { ... }
if condition { ... }

repeat { ... } until (condition)
repeat { ... } until condition
```

## Panic Mode Recovery

The function `escapeParseError()` is used to skip current error code by 
locating the next delimiter, while the function `reportParseError()` 
generates the diagnostic information for the programmers to debug their 
Opus code.

```C
void escapeParseError(Parser *parser, FILE *sourceCode);
void reportParseError(Parser *parser);
```

Currently, the Opus parser could handle the following errors and 
generate useful diagnostic hints to the programmers.

| Parse Error                                 | Brief Explanation                                   |
|---------------------------------------------|-----------------------------------------------------|
| `PARSE_ERROR_MISSING_IDENTIFIER`            | Expected name for variable/constant missing.        |
| `PARSE_ERROR_MISSING_TYPE_ANNOTATION`       | Expected ':' for type annotation missing.           |
| `PARSE_ERROR_MISSING_TYPE_NAME`             | Expected type name missing after ':'.               |
| `PARSE_ERROR_DECLARATION_SYNTAX`            | Expected '=' or newline after declaration.          |
| `PARSE_ERROR_MISSING_RIGHT_VALUE`           | Missing right-hand assignment value after '='.      |
| `PARSE_ERROR_UNRESOLVABLE`                  | Unrecognized or invalid token encountered.          |
| `PARSE_ERROR_MISSING_ARGUMENT_LABEL`        | Argument label missing in function call.            |
| `PARSE_ERROR_MISSING_COLON_AFTER_LABEL`     | Expected ':' missing after argument label.          |
| `PARSE_ERROR_MISSING_FUNCTION_NAME`         | Function name missing after keyword.                |
| `PARSE_ERROR_MISSING_OPENING_BRACKET`       | Missing '(' for function parameter list.            |
| `PARSE_ERROR_MISSING_RIGHT_ARROW`           | Expected '->' missing after function parameters.    |
| `PARSE_ERROR_MISSING_RETURN_TYPE`           | Return type missing after '->'.                     |
| `PARSE_ERROR_MISSING_OPENING_CURLY_BRACKET` | Missing '{' for statement body.                     |
| `PARSE_ERROR_MISSING_UNTIL_CONDITION`       | 'until' keyword missing for loop termination.       |
| `PARSE_ERROR_MISSING_IN_STATEMENT`          | 'in' keyword missing for iterables in loops.        |
| `PARSE_ERROR_MISSING_DELIMITER`             | Expected newline missing after statement.           |
| `PARSE_ERROR_MISSING_CONDITION`             | Missing condition in conditional or loop statement. |
| `PARSE_ERROR_MISSING_OPERAND`               | Expected operand missing in expression.             |
| `PARSE_ERROR_MISSING_ARGUMENT`              | Function argument missing after ':'.                |

## Context Free Grammar Rules 
Opus parser uses a set of Context-Free Grammars (CFGs) to generate
an Abstract Syntax Tree (AST) that represents the syntactic structure 
of Opus code. `Program` defines the entry point of the parser, \
consists of a sequence of statements. 

$$
\text{Program} \rightarrow \text{Statement} \ \text{Delimiter} \ | \ \text{Program}
$$

The linked-list style representation where each statement points to 
the next one is used in Opus parser construction. That means, for 
each `AST_PROGRAM` node, the left node is a statement, while the 
right node is another `AST_PROGRAM` node with the same structure.
As a result, we will have a right-skewed tree structure as displayed 
below.

```text
AST_PROGRAM
├── AST_STATEMENT
└── AST_PROGRAM
    ├── AST_STATEMENT (Statement 1)
    └── AST_PROGRAM
        ...
        ├── AST_STATEMENT (Last Statement)
        └── AST_PROGRAM
```

### Statements

A statement in the Opus language can take several forms, as shown in the grammar below:

$$
\begin{align*}
\text{Statement} &\rightarrow\text{VariableDeclaration} \\
                 &\ | \quad\text{ConstantDeclaration} \\
                 &\ | \quad\text{AssignmentStatement} \\
                 &\ | \quad\text{FunctionDefinition} \\
                 &\ | \quad\text{FunctionImplementation} \\
                 &\ | \quad\text{FunctionCall} \\
                 &\ | \quad\text{ReturnStatement} \\
                 &\ | \quad\text{ConditionalStatement} \\ 
                 &\ | \quad\text{RepeatUntilStatement} \\
                 &\ | \quad\text{ForInStatement} 
\end{align*}
$$

### Variable and Constant Declarations

Variable and constant declarations define new identifiers along with their associated types. The grammar rules are:

$$
\text{VariableDeclaration} \rightarrow \text{"var"} \ \text{Identifier} : \text{Type}
$$

$$
\text{ConstantDeclaration} \rightarrow \text{"let"} \ \text{Identifier} : \text{Type}
$$

The corresponding AST structure is:

```text
AST_VARIABLE_DECLARATION ("var" or "let")
├── AST_IDENTIFIER ("variable_name")
└── AST_TYPE ("type_name")
```

### Assignment Statements

An assignment statement assigns an expression to a variable or constant:

$$
\begin{align*}
\text{AssignmentStatement} \rightarrow \text{LeftValue} = \text{Expression} \ \text{Delimiter} \\
\text{LeftValue} \rightarrow \text{Identifier} \ | \ \text{DeclarationStatement}
\end{align*}
$$

Example AST for `var number: Int = 42`:

```text
AST_ASSIGNMENT (=)
├── AST_VARIABLE_DECLARATION (var)
│   ├── AST_IDENTIFIER (number)
│   └── AST_TYPE_ANNOTATION (Int)
└── AST_LITERAL (42)
```

### Function Definitions

Function definitions include an identifier, parameters, and return type:

$$
\text{FunctionDefinition} \rightarrow \text{func } \ \text{Identifier} ( \text{ParameterList}? ) \rightarrow \text{ReturnType}
$$

Example AST for `func greeting() -> String`:

```text
AST_FUNCTION_DEFINITION (func)
├── AST_IDENTIFIER (greeting)
└── AST_FUNCTION_SIGNATURE
    ├── AST_PARAMETER_LIST
    └── AST_FUNCTION_RETURN_TYPE (String)
```

### Parameter Lists

Defines labeled parameters for functions:

$$
\begin{align*}
\text{ParameterList} & \rightarrow \text{LabeledParameter} (\text{, LabeledParameter})^* \\
\text{LabeledParameter} & \rightarrow \text{Identifier} \text{ : } \text{Type}
\end{align*}
$$

Corresponding AST:

```text
AST_PARAMETER_LIST
├── AST_PARAMETER
│   ├── AST_PARAMETER_LABEL (firstArg)
│   └── AST_TYPE_ANNOTATION (Int)
└── AST_PARAMETER_LIST
    ├── AST_PARAMETER
    │   ├── AST_PARAMETER_LABEL (secondArg)
    │   └── AST_TYPE_ANNOTATION (String)
    └── AST_PARAMETER_LIST
```

### Code Blocks

Code blocks encapsulate statements within curly braces:

$$
\text{CodeBlock} \rightarrow \{ \text{ Statement } \}
$$

AST representation:

```text
AST_CODE_BLOCK
├── AST_STATEMENT
└── AST_CODE_BLOCK
```

### Return Statements

Defines optional return expressions:

$$
\text{ReturnStatement} \rightarrow \text{return} \ \text{Expression}? \ \text{Delimiter}
$$

Example AST:

```text
AST_RETURN_STATEMENT (return)
└── AST_BINARY_EXPRESSION (+)
    ├── AST_LITERAL (1)
    └── AST_LITERAL (2)
```

### Conditional Statements

`if-else` statements with optional branches:

$$
\begin{align*}
\text{ConditionalStatement} \rightarrow & \text{ if } \ \text{Expression} \\
& \text{ CodeBlock } (\text{ else if } \ \text{Expression} \ \text{CodeBlock})^* \\
& (\text{ else } \ \text{CodeBlock})?
\end{align*}
$$

Example AST:

```text
AST_CONDITIONAL_STATEMENT (if)
├── AST_BOOLEAN_LITERAL (true)
└── AST_CONDITIONAL_BODY
    ├── AST_CODE_BLOCK (if-execution)
    └── AST_CODE_BLOCK (else-execution)
```

### Repeat-Until Loops

Loop executed at least once until a condition is met:

$$
\text{RepeatUntilStatement} \rightarrow \text{repeat} \ \text{CodeBlock} \ \text{ until } \ \text{Expression} \ \text{Delimiter}
$$

AST:

```text
AST_REPEAT_UNTIL_STATEMENT (repeat)
├── AST_BOOLEAN_EXPRESSION
└── AST_CODE_BLOCK
```

### For-In Loops

Iterates over elements in an iterable object:

$$
\text{ForInStatement} \rightarrow \text{for} \ \text{Identifier} \ \text{in} \ \text{Expression} \ \text{CodeBlock}
$$

AST:

```text
AST_FOR_IN_STATEMENT (for)
├── AST_FOR_IN_CONTEXT
│   ├── AST_IDENTIFIER (number)
│   └── AST_IDENTIFIER (numbers)
└── AST_CODE_BLOCK
```

### Expressions

Expressions involve logical operations, comparisons, arithmetic, and function calls:

$$
\begin{align*}
\text{Expression} \rightarrow & \text{ LogicalOr} \\
\text{LogicalOr} \rightarrow & \text{ LogicalAnd} \ || \ \text{LogicalAnd} \\
\text{LogicalAnd} \rightarrow & \text{ Comparison} \ \&\& \ \text{Comparison} \\
\text{Comparison} \rightarrow & \text{ Addition } (\gt | \lt | \geq | \leq | == | !=) \text{ Addition } \\
\text{Addition} \rightarrow & \text { Multiplication } (+ | -) \text{ Multiplication} \\
\text{Multiplication} \rightarrow & \text{ Prefix } ( \ * \ | \ / \ | \ \%) \text{ Prefix} \\
\text{Prefix} \rightarrow & \ (\ - \ | \ ! \ ) \text{ Prefix } | \text{ Postfix } \\
\text{Postfix} \rightarrow & \text{ Primary } (\ ! | \text{ FunctionCall }) \\
\text{Primary} \rightarrow & \text{ Numeric } | \text{ String } | \text{ Identifier } | \text{ Boolean } | ( \text{Expression} ) \\
\text{FunctionCall} \rightarrow & \text{ Identifier } ( \text{ArgumentList}? ) \\
\text{ArgumentList} \rightarrow & \text{ LabeledArgument } (, \text{ArgumentList})? \\
\text{LabeledArgument} \rightarrow & \text{ Identifier} : \text{Expression} \\
\end{align*}
$$

Example Logical OR AST (`lhs || rhs`):

```text
AST_BINARY_EXPRESSION (||)
├── AST_IDENTIFIER (lhs)
└── AST_IDENTIFIER (rhs)
```

Example arithmetic expression AST (`1 + 2 * 3`):

```text
AST_BINARY_EXPRESSION (+)
├── AST_LITERAL (1)
└── AST_BINARY_EXPRESSION (*)
    ├── AST_LITERAL (2)
    └── AST_LITERAL (3)
```

### Function Calls

Labeled arguments in function calls:

$$
\begin{align*}
\text{FunctionCall} \rightarrow & \text{ Identifier} ( \text{ArgumentList}? ) \\
\text{ArgumentList} \rightarrow  & \text{ LabeledArgument} (, \text{ ArgumentList})? \\
\text{LabeledArgument} \rightarrow & \text{ Identifier} : \text{Expression} \\
\end{align*}
$$

AST example for parsing `func getRandomInt(between: 1, and: 100)`:

```text
AST_FUNCTION_CALL
├── AST_IDENTIFIER (getRandomInt)
└── AST_ARGUMENT_LIST
    ├── AST_ARGUMENT
    │   ├── AST_ARGUMENT_LABEL (between)
    │   └── AST_LITERAL (1)
    └── AST_ARGUMENT_LIST
        ├── AST_ARGUMENT
        │   ├── AST_ARGUMENT_LABEL (and)
        │   └── AST_LITERAL (100)
        └── AST_ARGUMENT_LIST
```

### Function Implementations

Function implementations consist of a function definition (signature) 
and an associated code block representing the body of the function. 
The example AST would be:

```text
AST_PROGRAM
├── AST_FUNCTION_IMPLEMENTATION
│   ├── AST_FUNCTION_DEFINITION (func)
│   │   ├── AST_IDENTIFIER (function_name)
│   │   └── AST_FUNCTION_SIGNATURE
│   │       ├── AST_PARAMETER_LIST
│   │       ├── AST_PARAMETER
│   │       │   ├── AST_PARAMETER_LABEL (first_parameter)
│   │       │   └── AST_TYPE_ANNOTATION (type)
│   │       └── AST_PARAMETER_LIST
│   │           └── ...
│   │   └── AST_FUNCTION_RETURN_TYPE (type)
│   └── AST_CODE_BLOCK (function_body)
```