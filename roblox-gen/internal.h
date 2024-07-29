/*
Internal header. Not meant for external use.
*/
#pragma once
#include <stdio.h>

/* util.c */
int string_size(const char* x);
int string_compare(const char* x, const char* y);

/* err.c */
void RBXG_Logger_Init(char* lines, const char* filename);
void RBXG_Error(int line_num, int start, int end, const char* code, const char* type_of_err);
/* RBXG_Error with less parameters (only 1 ^) */
#define RBXG_Simple_Token_Error(line_num, start, code)  RBXG_Error(line_num, start, start+1, code, "Syntax")
#define RBXG_Simple_Parse_Error(line_num, start, code)  RBXG_Error(line_num, start, start+1, code, "Parse")

/* lexer.c */
typedef enum TK_Kind {
  TK_Newline,
  TK_Command,
  TK_String,
  TK_Numeric,
  TK_Identifier,
  TK_Boolean,
  
} TK_Kind;

typedef struct Token {
  TK_Kind kind;
  char* value;

  struct Token* prev;
  struct Token* next;

  int line;
  int column;
  int columns_traversed;
} Token;

typedef struct LexOut {
  Token** tks;
  size_t siz;
  size_t cap;
} LexOut;

typedef struct LexState {
  struct {
    char* buffer;
    size_t siz;
    size_t cap;
  } tkbuf;

  struct {
    Token** refs;
    size_t siz;
    size_t cap;
  } tks;


  int line;
  int column;
  int columns_traversed;
  int column_newline;

  int in_string;
} LexState;

LexOut* RBXG_Tokenize(char* contents);