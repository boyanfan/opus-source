// lexer.c
//
// Created by Boyan Fan, 2025/01/16
//

#include <string.h>
#include "lexer.h"

int isOpusSourceCode(const char *filename) {
    const char *extension = OPUS_FILE_EXTENSION;
    size_t filenameLength = strlen(filename);
    size_t extensionLength = strlen(extension);

    // Check if the filename is long enough to include the extension
    if (filenameLength < extensionLength) return 0;

    // Compare the end of the filename with the ".opus" extension
    return strcmp(filename + filenameLength - extensionLength, extension) == 0;
}

FILE* openOpusSourceCode(const char *filename) {
    // Check if the file is Opus source code (with .opus extension)
    if (!isOpusSourceCode(filename)) {
        fprintf(stderr, NOT_OPUS_SOURCE_CODE_ERROR, filename);
        return NULL;
    }

    FILE *file = fopen(filename, READONLY_ACCESS);

    // If unable to read from the given Opus source code
    if (!file) {
        fprintf(stderr, ACCESS_OPUS_SOURCE_CODE_ERROR, filename);
        return NULL;
    }

    // Return the pointer to the file only if the file is successfully loaded
    return file;
}
