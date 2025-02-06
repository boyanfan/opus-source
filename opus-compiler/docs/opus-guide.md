# Opus Programming Language

Opus provides fundamental data types (e.g. `Int` for integers, `String` for text literals 
and `Bool` for boolean values), support control flow statements (e.g. `if-else` for 
conditional branching, `for-in` for collection iteration), arithmetic and logical 
operations (e.g. addition, multiplication and negation), comments for annotating their 
code without affecting execution (e.g. any text following `//` on a line) and three 
unique features required by this course: a `repeat-until` loop for controlled iteration, 
a built-in factorial operation for computing factorials, and runtime error detection to 
handle issues like division by zero or out-of-bounds access.

Opus uses curly braces (`{}`) as block delimiters to define code structures, but does 
not require a semicolon (`;`) as a statement delimiter. Instead, statements are naturally
separated by line breaks to improve readability.

---

## Arithmetic

Opus supports six standard arithmetic operators for `Numeric` types - *addition* (`+`),
*subtraction* (`-`), *multiplication* (`*`), *division* (`/`), *modulo* (`%`), and 
*factorial* (`!`).

```opus
1 + 2    // Addition,       equals to 3
1 - 2    // Subtraction,    equals to 1
1 * 2    // Multiplication, equals to 2
1 / 2    // Division,       equals to 0.5
-5 % 2   // Modulo,         equals -1
3!       // Factorial,      equals 6
```

The sign of a `Numeric` value can be toggled using a prefixed minus sign (`-`), 
prepended directly before the value it operates on without any white space.

```opus
-3   // Representation of minus three
```

## Variables and Types

Variables associate a name with a value of a particular type annotated by a colon. 
Use keyword `var` to declare a variable, followed by its name and its type, then 
use the operator `=` to assign a value.

`````opus
var quizGrade: Int = 100
`````

Use keyword `let` to declare a constant, so the Opus compiler guarantees that its value 
will never be changed. A string literal is a sequence of characters wrapped by double 
quotation marks (`"`). Opus supports escape sequences within string literals 
(e.g. `\"` for double quotes, `\n` for a newline, and `\t` for a tab space).

`````opus
let studentName: String = "Boyan Fan"
`````

## Control Flow

Opus supports execute different pieces of code based on certain conditions. Use `if-else` 
statement to execute a set of statements only if its condition is `true`, and optionally 
provide an alternative set of statements after `else` for situations when the condition 
is `false`.

`````opus
let temperature: Float = -12.5

if (temperature < 0) { 
	print("It is so cold outside! I gonna skip this morning lecture!") 
} else { 
	print("It is not that cold outside, it's time to go!") 
}
`````

Use `for-in` statement to iterate over a sequence (e.g. ranges of numbers, or 
characters in a string) and performs a same set of statements for each of them. 
For example, the factorial operation could also be done manually by using the following 
loop.

`````opus
var result: Int = 3   // We want to calculate 3!, the factorial of 3

for number in 1...3 {
	result = result * number   // Looping over number equals to 1 and 2
}
`````

Use `repeat-until` to perform a single pass through the loop block first before 
considering the condition, then continues to repeat the body until the condition is `ture`.
For example, the modulo operation could also be done manually by using the following loop.

`````opus
var dividend: Int = 13   // We want to find 13 % 3, the remainder of 13 / 3

repeat {
	dividend = dividend - 3
} until (dividend < 0)   // After looping, the remainder = dividend + 3
`````

## Functions

Functions are defined to perform a specific task and are called when needed. 
A function is defined by using `func` keyword, providing a function name, optionally 
defining one or more named, typed values as its arguments, and define a type that the 
function returns.

`````opus
func isEven(number: Int) -> Bool {
	return number % 2 == 0
}
`````

Each function has a name that describes the task that the function performs, and it is 
used to call this function. A function’s arguments must always be provided in the same 
order as the function’s parameter list. For example, pass a value that matches the `Int` 
type to call the function `isEven()` displayed above.

`````opus
isEven(4)   // Returns true
`````

## List of Keywords

The following table summarises all the keywords reserved by Opus, even though some of them (which are marked by *experimental*) will not been fully supported during this project.

| Keyword  | Explanation                                                                     |
|:---------|:--------------------------------------------------------------------------------|
| var      | Declares a variable that can be modified.                                       |
| let      | Declares a constant whose value cannot be modified.                             |
| if       | Starts a conditional statement that executes code if a condition is met.        |
| else     | Provides an alternative block of code to execute when an if condition is false. |
| repeat   | Starts a loop until a condition is met.                                         |
| until    | Used to define a loop that runs until a condition becomes true.                 |
| for      | Initiates a loop that iterates over a sequence or range.                        |
| in       | Used with for loops to specify the sequence or range to iterate over.           |
| return   | Exits from a function and returns a value.                                      |
| *class*  | *Experimental: defines a reference type that is passed by reference..*          |
| *struct* | *Experimental: defines a value type that is passed by value.*                   |
| func     | Declares a function that encapsulates reusable code.                            |
| true     | Represents a Boolean value that is logically true.                              |
| false    | Represents a Boolean value that is logically false.                             |