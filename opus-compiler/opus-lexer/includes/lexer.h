// lexer.h
//
// Created by Boyan Fan, 2025/01/16
//

#ifndef LEXER_H
#define LEXER_H

#define OPUS_FILE_EXTENSION ".opus"
#define READONLY_ACCESS "r"

#define NOT_OPUS_SOURCE_CODE_ERROR "[FileTypeError]: File '%s' is not the Opus source code. (Must be .opus files)\n"
#define ACCESS_OPUS_SOURCE_CODE_ERROR "[AccessError]: File '%s' could not be found. (Double-check the file path)\n"

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
FILE* openOpusSourceCode(const char *filename);

#endif /* LEXER_H */
