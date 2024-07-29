/*
CLI interface for roblox-gen.
*/
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
  #include "unistd.h"
#else
  #include <unistd.h>
#endif
#include "roblox-gen/gen.h"

#define FORCE_INLINE [[gnu::always_inline]] inline

/* Strings. */
#define NO_ARGUMENTS_STRING  "roblox-gen: A middle-end for generating Luau code.\nPass \033[93m--help\033[0m for commands or \033[93m--info\033[0m for more information.\n"
#define HELP_STRING          "--version (--v) -- Version string.\n--compile (--c) <filepath> <filedest>: Compiles the given <filepath> into <filedest> (creates the <filedest> if it doesn't exist).\n"
#define VERSION_STRING       "Version %s. (C) All rights reserved.\n"

/* String utilities. */
static int string_size(const char* x) {
  int sz = -1;
  while (x[++sz] != '\0');
  return sz;
}
static int string_compare(const char* x, const char* y) {
  int i = -1;
  while(x[++i] != '\0' && x[i] == y[i]);
  return string_size(y) == i;
}

/* Prints, then exits program. */
#define printf_stop(...) printf(__VA_ARGS__); exit(EXIT_SUCCESS)

/*}==========================================*/

/* Commands. */
static inline void help() {
  printf_stop(HELP_STRING);
}
static inline void version() {
  printf_stop(VERSION_STRING, ROBLOX_GEN_VERSION);
}
static inline void compile(int argc, char* argv[]) {
  const char* file_path = argv[2];
  const char* out_path = argv[3];

  if (access(file_path, F_OK) == 0) { /* Check if src file exists. */
    RBXG_Compile_To(file_path, out_path);
  } else { /* It doesn't exist. */
    fprintf(stderr, "Invalid file %s.\n", file_path);
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}

/*
=====================================================
MAIN
=====================================================
*/
int main(int argc, char* argv[]) {
  if (argc < 2) {
    /* No argument. */
    printf_stop(NO_ARGUMENTS_STRING);
  }

  /* There is an argument. */
  const char* cmd = argv[1];

  if (string_compare(cmd, "--help")) {
    help();
  }
  if (string_compare(cmd, "--version") || string_compare(cmd, "--v")) {
    version();
  }
  if (string_compare(cmd, "--compile") || string_compare(cmd, "--c")) {
    compile(argc, argv);
  }

  /* Invalid argument. */
  fprintf(stderr, "Invalid argument %s.\n", cmd);
  return EXIT_FAILURE;
}