# Statement of Originality
CISC 458 Course Project: Design and Implement a Custom Mini Programming Language 
Processor (hereinafter "this project") represents **original work created by team**.
All development progress of the code has been committed to our Git repository. 
It serves as a transparent record of our contributions, where any growth of Opus can be 
verified by accessing the commit history.

**Opus was entirely designed and implemented without any usage of AI tools.** 
That is, all the C code was manually written and refined by the team. The only instances 
where AI tools were utilized were for generating function documentation within the code 
and aiding in the writing of reports and documentation to enhance clarity and consistency.

---

# Opus Lexer Implementation
Opus source code are the files with `.opus` extension. The lexer uses `isOpusSourceCode()` 
to check the file extension, then uses `openOpusSourceCode()` to return a pointer to the 
file stream.

```C
int isOpusSourceCode(const char *filename);
FILE *openOpusSourceCode(const char *filename);
```

Then the Opus source code are lexed by mainly using two functions, where 
`peekNextCharacter()` peeks and returns the next character without actually consuming it 
(means **no** advancing the current reading position in the input stream), 
while `consumeNextCharacter()` consumes and return it.

```C
int peekNextCharacter(FILE *sourceCode);
int consumeNextCharacter(Lexer *lexer, FILE *sourceCode);
```

Then with peek and consume actions, we can define functions listed below to freely locate
current reading position while lexing. Like, `locateStartOfNextToken()` was called at the 
start of each iteration, `locateStartOfNextLine()` was called for skipping line comment.
```C
int locateStartOfNextToken(Lexer *lexer, FILE *sourceCode);
int locateStartOfNextLine(Lexer *lexer, FILE *sourceCode);
```

If a literal is successfully lexed (means there is no token nor lexing error), lexer 
calls `initSafeToken()` to return an error-free token with a specific token 
type; if any error occurs, the lexer calls `initUnsafeToken()` to return a token 
with a specific error type.

```C
Token *initSafeToken(TokenType tokenType, Lexer *lexer, const char *lexeme);
Token *initUnsafeToken(TokenError tokenError, Lexer *lexer, const char *lexeme);
```

Opus does not use `;` as delimiters. Instead, going into a newline could be a termination
of a statement if and only if we are outside a closure (so use `\n` as delimiters). 
A closure means that we are inside a pair of brackets or square brackets. 
In these cases (between `(...)` or `[...]`), a newline character is not a delimiter.

```C
int isInClosure(Lexer *lexer);
```

Closures are tracked by the lexer using a vector, where `[0]` is tracking brackets,
`[1]` for curly brackets, and `[2]` for square brackets. Therefore, any unclosed brackets can be 
found if this vector does not equal to `0` and the end of the file.

```C
int lexer->isInClosure[3];
```

Function `getNextToken()` is called in a loop to return tokens until the end of the file
is reached. Essentially, it is a large complex _Deterministic Finite Automation_ (_DFA_),
using combinational `if` statement as transition functions, using peeking and consuming
behaviours to transition between states, and using return statement for the accept states.

```C
Token *getNextToken(Lexer *lexer, FILE* sourceCode);
```

Peeking behaviors is critical in Opus, since a same symbol could be different tokens
based on its context. For example, an exclamation mark (`!`) is an arithmetic factorial if
the previous token is a numeric value or an identifier, but is a logical negation when it 
stands alone, or a part of not-equal-to operator (`!=`). Although the regular grammar is
context-free, we must introduce some context in the program for an elegant implementation.

```C
TokenType lexer->previousTokenType;
```

### Read from Lexer Output
For any error-free tokens, the Opus lexer display a `<Token>` with its token type, 
its lexeme and its location in the source file (in the format of `line:colomn`), 
as an example displayed below.

```text
<Token:Numeric, Lexeme:"-56.7"> at location 2:14
```

If any error occurs, the Opus lexer display a `<ERROR>` with its error type,
its lexeme and the location where the error occurs (in the format of `line:colomn`),
as an example displayed below.

```text
<ERROR:UndefinedOperator, Lexeme:"==="> at location 16:2
```

### Error Recovery
Once an error occurs, the Opus lexer reports it and keeps lexing source codes until
the end of the file has been reached. The lexer will call `skipCurrenToken()` to collect 
all the remaining characters of the current tokens.

```C
int skipCurrenToken(Lexer *lexer, FILE* sourceCode, char *lexeme, char *skippedSequence);
```

### Design Considerations
When designing the Opus programming language, one of the key decisions was
whether to use a keyword-based primitive type system similar to C (e.g., `int`, `float`), 
or to adopt a type identifier-based system like Swift (e.g., `:Int`, `:Float`). We ultimately 
chose the latter, so that both native types and user-defined types share the same type 
annotation syntax.

By making the native types behaves same as user-customized types, we built a powerful type
system that is, **safe**, ensuring uniform type safety; **flexible**, allowing native 
types to be extended like custom types; and **expressive**, providing a unified syntax 
for type annotation and customization.

Therefore, for example, when we lexing the following statement, token `Int` should be 
**an identifier that associates with the integer value type**, instead of a keyword. 

```Opus
let assignmentGrade: Int = 100
```

# Error Codes
The table displayed below provides an overview of error codes used in the *Opus* programming 
language lexer. These error codes help identify issues encountered during lexical analysis 
and syntax parsing.

---

## Token Errors
The errors listed below are related to individual tokens during lexical analysis.

| Error Code                  | Description                                                | How to Fix                                                                    |
|-----------------------------|------------------------------------------------------------|-------------------------------------------------------------------------------|
| `ERROR_TOKEN_NONE`          | No error occurred.                                         | No action needed.                                                             |
| `ERROR_UNRECOGNIZABLE`      | An invalid or unrecognized character was encountered.      | Check for unintended special characters or typos in the source code.          |
| `ERROR_MALFORMED_NUMERIC`   | A number was malformed or invalid.                         | Ensure numeric values follow the correct format, avoiding invalid characters. |
| `ERROR_UNDEFINED_OPERATOR`  | An invalid or unrecognized operator was detected.          | Use only supported operators and check for typos in expressions.              |
| `ERROR_OVERFLOW`            | A lexeme exceeded the buffer limit, causing an overflow.   | Shorten overly long identifiers, strings, or numerical values.                |
| `ERROR_ORPHAN_UNDERSCORE`   | An underscore was used incorrectly (e.g., standing alone). | Ensure underscores are part of valid identifiers or variable names.           |
| `ERROR_UNTERMINATED_STRING` | A string literal was missing a closing double quote (`"`). | Add the missing closing quote (`"`) at the end of the string.                 |


## Lexing Errors
The errors listed below are related to overall lexical structure, described by `opus-guide` document.

| Error Code                      | Description                                | How to Fix                                               |
|---------------------------------|--------------------------------------------|----------------------------------------------------------|
| `ERROR_LEXER_NONE`              | No error occurred.                         | No action needed.                                        |
| `ERROR_UNCLOSED_BRACKET`        | A closing parenthesis (`)`) is missing.    | Add the missing `)` to properly close the expression.    |
| `ERROR_UNCLOSED_SQUARE_BRACKET` | A closing square bracket (`]`) is missing. | Ensure each opening `[` has a corresponding closing `]`. |
| `ERROR_UNCLOSED_CURLY_BRACKET`  | A closing curly bracket (`}`) is missing.  | Add the missing `}` to properly close the code block.    |


---

## Next Step
In Phase 2 we extend the existing lexer by adding a parser that constructs a syntax tree,
based on a manually defined Context-Free Grammars (CFGs). The parser will validate 
the syntactic structure of Opus programs to ensure that statements and expressions 
conform to Opus grammar rules.
