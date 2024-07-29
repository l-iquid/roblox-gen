/*
Source analyzer.
Analyses the source code and creates manageable tokens.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../internal.h"

/* config */
#define PRINT_TOKENS    1 // debug
#define TKLST_INIT_CAP  8
#define TKLST_REALLOC   8
/*}===============================*/

/* dictionary */
static const char* COMMAND_LIST[] = {
  "CLASSDEF",
};

#define is_in_tab(tab,v)                                     \
    for (int i = 0; i < sizeof(tab)/sizeof(tab[0]); i++) {   \
        if (strcmp(v, tab[i]) == 0)                          \
            return 1;                                        \
    }                                                        \
    return 0;

static inline int is_command(const char* v) {
  is_in_tab(COMMAND_LIST, v);
}
static inline int is_boolean(const char* v)
{return strcmp(v, "true") == 0 || strcmp(v, "false") == 0;}
static inline int is_num(const char* v)
{return v[0] >= '0' && v[0] <= '9';}


/*}===============================*/

/*}===============================*/

/* Token */
static Token* make_token(TK_Kind kind, char* value, int line, int column, int columns_traversed) {
  Token* tk = malloc(sizeof(Token));
  tk->kind = kind;
  tk->value = strdup(value);
  tk->line = line;
  tk->column = column-string_size(value);
  tk->columns_traversed = columns_traversed;
  tk->prev = NULL;
  tk->next = NULL;
  return tk;
}
/*}===============================*/

/* LexState utils */

/* Puts a token's pointer into the LexState. */
static void put_tk_into_ls(Token* tk, LexState* ls) {
  if (ls->tks.siz + 1 > ls->tks.cap) {
    ls->tks.cap += TKLST_REALLOC;
    ls->tks.refs = realloc(ls->tks.refs, sizeof(void*)*ls->tks.cap);
  }

  ls->tks.refs[ls->tks.siz] = tk;
  ls->tks.siz++;

  if (ls->tks.siz > 1) { /* last->next = current, current->prev = last. */
    //printf("%d: [%s %s]\n", ls->tks.siz, ls->tks.refs[ls->tks.siz-2]->value, ls->tks.refs[ls->tks.siz-1]->value);
    ls->tks.refs[ls->tks.siz-2]->next = tk;
    ls->tks.refs[ls->tks.siz-1]->prev = ls->tks.refs[ls->tks.siz-2];
  }
}

/* Generates a token from the LexState information. */
static inline Token* create_tk_from_ls(TK_Kind k, LexState* ls) {
  return make_token(k, ls->tkbuf.buffer, ls->line, ls->column, ls->columns_traversed);
}
/* Zeros the token buffer. Shrimple. */
static void wipe_ls_buffer(LexState* ls) {
  for (int i = 0; i < ls->tkbuf.siz; i++)
    ls->tkbuf.buffer[i] = 0;
  ls->tkbuf.siz = 0;
}
/* Creates a token from the ls, into the ls. */
#define create_token_into_ls(k,_ls) put_tk_into_ls(create_tk_from_ls(k, _ls), _ls)

static void push_chr_ls_buffer(LexState* ls, char CHR) {
  if (ls->tkbuf.siz + 2 > ls->tkbuf.cap) {
    ls->tkbuf.cap += 16;
    ls->tkbuf.buffer = realloc(ls->tkbuf.buffer, ls->tkbuf.cap);
    for (int i = ls->tkbuf.cap-17; i < ls->tkbuf.cap; i++)
      ls->tkbuf.buffer[i] = 0; /* Zero the new allocated space (to prevent string issues). */
  }

  ls->tkbuf.buffer[ls->tkbuf.siz] = CHR;
  ls->tkbuf.siz++;
}

/*}===============================*/

/*
=================================================
MAIN
=================================================
*/

/* inlined utils */
static inline void advance(LexState* ls, char** iptr) {
  ls->column++;
  ls->columns_traversed++;
  *(*iptr)++;
}

static inline void newline(LexState* ls) {
  if (ls->tkbuf.siz > 0) {
    create_token_into_ls(TK_Identifier, ls);
    wipe_ls_buffer(ls);
  }
  ls->line++;
  ls->column = 0;
  ls->column_newline = ls->columns_traversed;

  put_tk_into_ls(make_token(TK_Newline, "", ls->line-1, ls->column, ls->columns_traversed), ls);
}

static inline void whitespace(LexState* ls, char** _iptr) {
  char* iptr = *_iptr;
  if (ls->in_string) {
    /* nuffing to do */
    push_chr_ls_buffer(ls, ' ');
    return;
  }

  /* Not in string. */

  if ((ls->tks.siz < 1 && ls->tkbuf.siz < 1) || (ls->tkbuf.siz < 1 && ls->tks.refs[ls->tks.siz-1]->kind == TK_Newline)) {
    /* Space after new line */
    RBXG_Simple_Token_Error(ls->line, (ls->columns_traversed - ls->column_newline), "Invalid spacing.");
  }

  switch (iptr[1]) {
    case ' ': {
      /* Suspected spacing error. */
      RBXG_Simple_Token_Error(ls->line, (ls->columns_traversed - ls->column_newline) + 1, "Invalid spacing.");
    }

    case '\0':
    case '\n': case '\r': {
      /* Space at end of line */
      RBXG_Simple_Token_Error(ls->line, (ls->columns_traversed - ls->column_newline), "Invalid spacing.");
    }
  }

  if (ls->tkbuf.siz > 0) {
    create_token_into_ls(TK_Identifier, ls);
  }

  wipe_ls_buffer(ls);
}


/* Check if a char is a valid identifier. */
#define is_valid_chr(_chr) ((_chr >= 'A' && _chr <= 'Z') || (_chr >= 'a' && _chr <= 'z') || (_chr >= '0' && _chr <= '9') || (_chr == '_'))
/* Look at each character individually. */
static inline void analyze_chr(LexState* ls, char** _iptr) {
  char* iptr = *_iptr;

  switch (*iptr) {
    /* Line break. */
    case '\n': case '\r': {
      newline(ls);
      break;
    }
    /* Whitespace. */
    case ' ': case '\f': case '\t': case '\v': {
      whitespace(ls, &iptr);
      break;
    }

    /* Symbols. */
    case '"': {
      if (ls->in_string == 0) {
        /* start string */
        ls->in_string = 1;
        if (ls->tkbuf.siz > 0) {
          create_token_into_ls(TK_Identifier, ls);
        }
      } else {
        /* end string */
        ls->in_string = 0;
        if (ls->tkbuf.siz > 0) {
          create_token_into_ls(TK_String, ls);
        }
      }
      wipe_ls_buffer(ls);
      break;
    }

    /* Every other character. */
    default: {
      if (is_valid_chr(*iptr) || ls->in_string) {
        push_chr_ls_buffer(ls, *iptr);
        break;
      }
      /* Invalid and not in string. */
      RBXG_Simple_Token_Error(ls->line, ls->columns_traversed - ls->column_newline, "Bad character.");
      break;
    }
  }
  
}

/* Create new types. */
static inline void analyze_types(LexState* ls) {

  for (int i = 0; i < ls->tks.siz; i++) {
    Token* tk = ls->tks.refs[i];

    //printf("[%s]\n", tk->value);

    switch (tk->kind) {
      case TK_Identifier: {
        if (is_num(tk->value)) {
          tk->kind = TK_Numeric;
          break;
        }
        if (is_boolean(tk->value)) {
          tk->kind = TK_Boolean;
          break;
        }
        if (is_command(tk->value)) {
          if (tk->prev != NULL) {
            switch (tk->prev->kind) {
              case TK_Newline: {
                /* Valid */
                tk->kind = TK_Command;
                break;
              }
              default: {
                RBXG_Simple_Token_Error(tk->line, tk->column, "Invalid command placement.");
                break;
              }
            }
          } else { // valid
            tk->kind = TK_Command;
          }
          break;
        }

        break;
      }

      case TK_Newline: {
        if (tk->prev->kind == TK_Newline) {
          /* Empty line (or newline repeat). */
          RBXG_Simple_Token_Error(tk->line, tk->column+1, "Empty line detected.");
        }
        break;
      }
    }
  }

}

/* Main loop. */
static inline void main_loop(LexState* ls, char* contents) {
  char* iptr = contents;

  while (*iptr != '\0') {
    analyze_chr(ls, &iptr);
    advance(ls, &iptr);
  }

  newline(ls); /* Run a newline check. */

  analyze_types(ls);
}

/* API */
LexOut* RBXG_Tokenize(char* contents) {
  LexOut* lo = malloc(sizeof(LexOut));

  LexState ls = {
    .tkbuf = {
      .buffer = calloc(32, 1),
      .cap = 32,
      .siz = 0,
    },

    .tks = {
      .refs = malloc(sizeof(void*)*TKLST_INIT_CAP),
      .cap = TKLST_INIT_CAP,
      .siz = 0,
    },

    .line = 1,
    .column = 1,
    .columns_traversed = 1,
    .column_newline = 0,

    .in_string = 0,
  };

  main_loop(&ls, contents);

  /* Finish */
  lo->cap = ls.tks.cap;
  lo->siz = ls.tks.siz;
  lo->tks = malloc(sizeof(void*)*lo->cap);
  for (int i = 0; i < ls.tks.siz; i++)
    lo->tks[i] = ls.tks.refs[i];

#if(PRINT_TOKENS == 1)
  for (int i = 0; i < lo->siz; i++) {
    Token* tk = lo->tks[i];
    printf("kind: %d. value: %s. :%d:%d:\n", tk->kind, tk->value, tk->line, tk->column);
  }
#endif

  /* Free up memory. */
  free(ls.tkbuf.buffer);
  free(ls.tks.refs);
  return lo;
}
void RBXG_Free_LexOut(LexOut* lo) {
  for (int i = 0; i < lo->siz; i++) {
    free(lo->tks[i]->value);
    free(lo->tks[i]);
  }
  free(lo->tks);
  free(lo);
}