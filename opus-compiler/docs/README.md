# CISC 458 Course Project

Introducing *Opus*, a custom mini programming language designed to be modern and elegant, 
implemented to meets the requirements outlined by the criteria. As part of Phase 1,
we focus on lexical analysis by defining a set of regular expressions to tokenize 
*Opus* source code and reporting  any lexical errors.

## Statement of Originality
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

## Build and Run Opus
To build and run Opus Compiler, **download and uncompress the submission**, or clone
the Opus repository by using the following commands.
```shell
git clone https://github.com/boyanfan/opus-source.git
cd opus-source/opus-compiler
```
If you choose to download and uncompress the submission, make sure the current 
working directory is `../opus-source/opus-compiler` by using `pwd` to check. Then use
the following commands to build.
```shell
mkdir build && cd build
cmake ..
cmake --build .
```
Once built, you can run Opus to compile a specific Opus source file (`.opus` files). 
For example, the following command compiles `correct.opus` from `test` folder.
```shell
./Opus ../tests/correct.opus
```

## Next Step
In Phase 2 we extend the existing lexer by adding a parser that constructs a syntax tree,
based on a manually defined Context-Free Grammars (CFGs). The parser will validate 
the syntactic structure of Opus programs to ensure that statements and expressions 
conform to Opus grammar rules.