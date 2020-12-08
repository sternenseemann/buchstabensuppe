#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef TEST_EXIT_ON_FAIL
#define TEST_EXIT_ON_FAIL false
#endif

#ifndef TEST_INFO_WIDTH
#define TEST_INFO_WIDTH 50
#endif

static bool test_result = true;

void test_case(char *info, bool result) {
  char *result_str = "okay";
  FILE *output = stdout;

  if(!result) {
    result_str = "FAIL";
    output = stderr;
  }

  int w = TEST_INFO_WIDTH;

  while(*info != '\0') {
    fputc(*info, output);
    w--; info++;
  }

  fputs(":", output);
  w--;

  while(w > 0) {
    fputc(' ', output);
    w--;
  }

  fputs(info, output);
  fputs(result_str, output);
  fputc('\n', output);

  test_result = test_result && result;

  if(!test_result && TEST_EXIT_ON_FAIL) {
    exit(EXIT_FAILURE);
  }
}
