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
