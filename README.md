# The Opus Programming Language

Opus is an open-source modern programming language designed to be safe, fast, and elegant. It empowers developers to easily extend the language itself, ensuring it adapts seamlessly to everywhere. It embodies simplicity and beauty, making it easy to write and read for even new programmers, without sacrificing the power and flexibility that experienced experts need.

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
./Opus ../tests/phase-2/function.opus
```
You can always open an `.opus` file using any text editor or IDEs. Now it is time to start 
writing your own Opus codes! Create a plain text file, write codes that confirms the grammar
described by `opus-guide` document, save your code with `.opus` extension, 
and use the compiler you just built to compile it using the following command!
```shell
./Opus <your-opes-source-code>
```

### **Troubleshooting Build Issues**

If you encounter issues while building *Opus*, first ensure that all **prerequisites** are 
correctly installed. Run `cmake --version` to confirm that **CMake 3.20+** is available 
and `gcc --version` or `clang --version` to check for a valid **C compiler**. If CMake 
fails to locate a compiler, ensure your system's environment variables 
(e.g., `PATH` on Linux/macOS or system variables on Windows) include the compiler's 
installation directory.

If errors occur during `cmake --build .`, try removing the `build` directory 
(`rm -rf build` on Linux/macOS or `rmdir /s /q build` on Windows), then re-run the CMake 
configuration process. For missing dependencies, ensure all necessary libraries are 
installed. If using **Make**, verify that it is correctly installed (`make --version`).
