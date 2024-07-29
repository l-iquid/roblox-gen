/*
Error reporter.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

/*
* Uses the same error logging system as roblox-java.
* Difference in the _newline function (doesn't skip characters).
*/

/*
Parse Error!
54 | ...
55 | CLASSDEF  "abc"
              ^ Double space.
56 | ...
*/

static char* lines = NULL;
static char** formatted_lines = NULL;
static int total_lines = 1;
static char* current_filename = NULL;

// totally didnt steal code from roblox-java
static inline void _newline(int* counter, size_t* line_siz, int* start_line) {
  formatted_lines[*counter] = calloc((*line_siz), 1);
  char* formatted = formatted_lines[*counter];
  *start_line += *line_siz;
  int k = 0;

        for (int j = (*start_line)-(*line_siz); j < (*start_line); j++) {
          switch (lines[j]) {
            case '\n': case '\r': continue; /* Ignore line breaks. */
          }

          /* j is the line position, k is the formatted position */
          formatted[k] = lines[j];
          k++;
        }
        *line_siz = 1;
        (*counter)++;
}

void RBXG_Logger_Init(char* _lines, const char* filename) {
  /* Cleanup. */
  if (lines != NULL) {
    free(lines);
  }
  lines = NULL;
  if (formatted_lines != NULL) {
    for (int i = 0; i < total_lines; i++) {
      free(formatted_lines[i]);
    }
    free(formatted_lines);
    formatted_lines = NULL;
  }
  total_lines = 1;

  lines = strdup(_lines);
  current_filename = filename;

  /* count the total lines */
  char* i = lines;
  while (*i != '\0') {
    switch (*i) {
      case '\n': case '\r': {
        total_lines++;
        break;
      }
    }
    *i++;
  }

  /* formatted lines */
  /*
  ! ik this code is long and awkward
  */
  formatted_lines = malloc(sizeof(void*)*total_lines);
  i = lines;
  int counter = 0;
  int start_line = 0;
  size_t line_siz = 1;

  while (*i != '\0') {
    switch (*i) {
      case '\n': case '\r': { /* newline */
        _newline(&counter, &line_siz, &start_line);
        break;
      }

      default: { /* every other character */
        line_siz++;
        break;
      }
    }
    *i++;
  }

  _newline(&counter, &line_siz, &start_line); /* Run it again for the last line. */
}

void RBXG_Error(int line_num, int start, int end, const char* code, const char* type_of_err) {
  printf("roblox-gen: \033[93m%s\033[0m \033[91m%s Error!\033[0m\n", current_filename, type_of_err);

  if (line_num-1 > 0) {
    printf("%d | ...\n", line_num-1);
  }

  printf("\033[96m%d\033[0m | %s\n", line_num, formatted_lines[line_num-1]);

  for (int i = 0; i < 3; i++)
    printf(" ");

  for (int i = 0; i < start; i++) /* print the spaces before the ^^^*/
    printf(" ");

  for (int i = start; i < end; i++)
    printf("\033[91m^\033[0m");

  /* pink text */
  char* buf = malloc(strlen(code)+15);
  sprintf(buf, "\033[95m%s\033[0m\0", code);

  printf(" %s\n", buf);

  free(buf);

  if (line_num+1 <= total_lines) {
    printf("%d | ...\n", line_num+1);
  }
  exit(EXIT_FAILURE);
}