#include <stdio.h>
#include <stdlib.h>
#include "gen.h"
#include "internal.h"
//#include "../unistd.h"

void RBXG_Compile_To(const char* src_path, const char* out_path) {
  FILE* src_f = fopen(src_path, "r");
  FILE* out_f = fopen(out_path, "w"); /* Automatically makes a file if it doesn't exist. */

  /* Init */
  fseek(src_f, 0, SEEK_END);
  long fsize = ftell(src_f);
  fseek(src_f, 0, SEEK_SET); /* same as rewind(f) */
  char* contents = calloc(fsize + 1, 1);
  fread(contents, fsize, 1, src_f);

  RBXG_Logger_Init(contents, src_path);

  /* Parse */
  LexOut* lo = RBXG_Tokenize(contents);

  /* Done. */
  fclose(src_f);
  fclose(out_f);
  RBXG_Free_LexOut(lo);
}