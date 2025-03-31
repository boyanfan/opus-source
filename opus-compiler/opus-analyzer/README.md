# Opus Semantic Analyzer
This report details the design and implementation of the semantic analyzer for the 
Opus programming language. The analyzer works on the Abstract Syntax Tree produced 
by the parser, performing type checking, constant folding, and namespace management 
through code blocks.

---

## Overview
The semantic analyzer in Opus is responsible for verifying that the parsed program adheres to 
the language's semantic rules. Key responsibilities include: **Type Checking** - Ensuring 
that operations (arithmetic, logical, assignment) are performed on operands of compatible types; 
**Constant Folding** - Evaluating constant expressions at compile time to optimize the AST; 
**Scope Management** - Handling nested namespaces to enforce lexical scoping rules.

## Extended AST Structure
To support semantic analysis, the AST nodes have been extended with additional fields: 
`inferredType` - A character array representing the type of the expression deduced by the 
Opus Compiler; `isFoldable` - An integer flag indicating whether the expression can be computed 
at compile time; `value` - A union holding the computed constant value.

## Symbol Table Implementation
The `SymbolTable` is implemented as a singly linked list of `Symbol` entries, 
anchored by the `headSymbol` pointer. **Namespaces** are managed using an integer counter. 
When entering a new block, the `currentNamespace` is incremented. When a namespace is exited, 
The function removeSymbolsFromCurrentNamespace() traverses the symbol list and removes all 
entries associated with the current namespace.

One advantage of using integer counter is by performing `less or equal to` comparing, we can 
access variables from both the outer namespaces (just like other languages do) and the current
namespace, but the variables from any inner namespaces are invisible.

```C
currentSymbol->namespace <= symbolTable->currentNamespace
```

## Type Checking
The analyzer enforces strict rules regarding operand types, based on the operator: for
**Arithmetic Operators** (`+`, `-`, `*`, `/`, `%`), both operands must be of 
numeric types (`Int` or `Float`), and the result type is `Float` if either operand is `Float`, 
otherwise `Int`; for **Logical Operators** (`&&`, `||`, `!`), operands must be of type `Bool`; 
for **Relational Operators** (`==`, `!=`, `<`, `>`, `<=`, `>=`), both operands must be of 
compatible types (either numeric or boolean), and the result is always of type `Bool`; for
**Unary Operators** (`-`, `!`), factorial and negation requires a numeric operand, logical not 
requires a boolean operand.

## Error Handling Strategy
The `Analyzer` structure maintains the state of the semantic analyzer throughout 
the analysis of the AST, it has a single field `analyzerError` that holds the latest error 
status encountered during analysis. To report semantic issues in context, the analyzer uses 
a dedicated function, generates helpful diagnostic by taking an `analyzerError` from an 
`analyzer` entity and the location of a node where the error occurs. 
```C
void reportAnalyzerError(Analyzer *analyzer, ASTNode *node);
```

## Extensions and Enhancements
In addition to core type checking and scope management, the Opus semantic analyzer includes 
extended language features and compile-time optimizations that improve both 
correctness and performance.

### Immutable v.s. Mutable Bindings
The Opus language supports two kinds of variable declarations: `let` - **Immutable binding**, a
variable cannot be reassigned after initialization; `var` — **Mutable binding**, a variable 
can be reassigned freely within its scope. This behavior is enforced semantically through 
the `isMutable` field in the Symbol struct.

### Compile-Time Conditional Elimination
Another enhancement involves compile-time evaluation of conditional statements 
(`if`, `else if`, `else`) when the condition is a constant Boolean expression.
During analysis, if the `if` or `else if` condition expression is marked as 
`isFoldable` == 1 and its type is Bool, the analyzer can eliminate dead branches in the AST.
