// lexer.h
//
// Created by Boyan Fan, 2025/01/16
//

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

/// Checks if the given file to compile has the '.opus' extension.
///
/// @param filename The name of the file to check.
/// @return 1 (true) if the file has the ".opus" extension, 0 (false) otherwise.
///
int isOpusSourceCode(const char *filename);

/// Opens a file with the ".opus" extension for compiling.
///
/// @param filename The name of the file to open.
/// @return A pointer to the opened file (FILE*) if successful, or NULL if an error occurred.
///
FILE* openOpusFile(const char *filename);

#endif /* LEXER_H */
