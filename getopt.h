#pragma once
#include <string.h>
#include <stdio.h>

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

int
  getopt(int nargc, char * const nargv[], const char *ostr);