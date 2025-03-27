// symbol.h 
//
// This header defines the `Symbol` struct and related data structures for managing 
// identifiers and their properties in the symbol table. It supports scope management, 
// identifier type tracking, and chaining via linked lists for collision resolution.
//
// Created by Boyan Fan, 2025/03/23
//

#ifndef SYMBOL_H
#define SYMBOL_H

#include "token.h"

/// Represents a symbol in the symbol table.
typedef struct Symbol {
    char identifier[LEXEME_LENGTH];   /// The name of the variable, constant and function.
    char type[LEXEME_LENGTH];         /// The type name of the identifier or of the label.
    int namespace;                    /// The namespace (i.e. scope level) of the symbol.
    int hasInitialized;               /// Whether the symbol has been initialized.
    int isMutable;                    /// Whether it is a constant.
    Location declarationLocation;     /// The location where the symbol declarated.
    struct Symbol *nextSymbol;        /// Pointer to the next symbol for linked list implementation.
} Symbol;

/// Represents the symbol table used during semantic analysis.
typedef struct {
    int currentNamespace;   /// Current namespace (i.e. scope level) of the symbol.
    Symbol *headSymbol;     /// First symbol in the symbol table.
} SymbolTable;

/// Initializes a new, empty symbol table with the namespace set to 0.
/// @return A pointer to the newly allocated SymbolTable structure.
///
SymbolTable *initSymbolTable();

/// Adds a new symbol to the symbol table with the given identifier, type, and declaration location.
/// The symbol is added to the front of the linked list and assigned the current namespace.
///
/// @param symbolTable The symbol table to add the symbol to.
/// @param identifier The name of the symbol (e.g., variable or function).
/// @param type The type of the symbol (e.g., "int", "string").
/// @param location The source code location where the symbol was declared.
///
void addSymbol(SymbolTable *symbolTable, const char *identifier, const char *type, Location location);

/// Looks up a symbol in the symbol table by identifier, searching all namespaces 
/// from most recent to outer.
///
/// @param symbolTable The symbol table to search.
/// @param identifier The name of the symbol to look for.
/// @return A pointer to the matching Symbol, or NULL if not found.
///
Symbol *lookupSymbol(SymbolTable *symbolTable, const char *identifier);

/// Enters a new nested namespace (i.e. scope level) by incrementing the current namespace counter.
/// @param symbolTable The symbol table to update.
///
void enterNamespace(SymbolTable *symbolTable);

/// Exits the current namespace (i.e. scope level), removing all symbols declared within it.
/// @param symbolTable The symbol table to update.
///
void exitNamespace(SymbolTable *symbolTable);

/// Looks up a symbol in the current namespace only (ignores symbols in outer scopes).
///
/// @param symbolTable The symbol table to search.
/// @param identifier The name of the symbol to look for.
/// @return A pointer to the matching Symbol from the current namespace, or NULL if not found.
///
Symbol *lookupSymbolFromCurrentNamespace(SymbolTable *symbolTable, const char *identifier);

/// Removes all symbols that belong to the current namespace from the symbol table.
/// @param symbolTable The symbol table to clean.
///
void removeSymbolsFromCurrentNamespace(SymbolTable *symbolTable);

/// Frees all memory associated with the symbol table and its symbols.
/// @param symbolTable The symbol table to free.
///
void freeSymbolTable(SymbolTable *symbolTable);

///
void displaySymbolTable(SymbolTable *symbolTable);

#endif
