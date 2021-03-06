#define _GNU_SOURCE
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "contain.h"

char *progname;

char *append(char **destination, const char *format, ...) {
  char *extra, *result;
  va_list args;

  va_start(args, format);
  if (vasprintf(&extra, format, args) < 0)
    die(errno, "asprintf");
  va_end(args);

  if (*destination == NULL) {
    *destination = extra;
    return extra;
  }

  if (asprintf(&result, "%s%s", *destination, extra) < 0)
      die(errno, "asprintf");
  free(*destination);
  free(extra);
  *destination = result;
  return result;
}

void die(int errnum, char *format, ...) {
  va_list args;

  fprintf(stderr, "%s: ", progname);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  if (errnum != 0)
    fprintf(stderr, ": %s\n", strerror(errnum));
  else
    fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

char *string(const char *format, ...) {
  char *result;
  va_list args;

  va_start(args, format);
  if (vasprintf(&result, format, args) < 0)
    die(errno, "asprintf");
  va_end(args);
  return result;
}

char *tmpdir(void) {
  char *dir;

  if (!(dir = strdup("/tmp/XXXXXX")))
    die(errno, "strdup");
  else if (!mkdtemp(dir))
    die(0, "Failed to create temporary directory");
  return dir;
}

void waitforexit(pid_t child) {
  int status;

  if (waitpid(child, &status, 0) < 0)
    die(errno, "waitpid");
  else if (WEXITSTATUS(status) != EXIT_SUCCESS)
    exit(WEXITSTATUS(status));
}

void waitforstop(pid_t child) {
  int status;

  if (waitpid(child, &status, WUNTRACED) < 0)
    die(errno, "waitpid");
  if (!WIFSTOPPED(status))
    exit(WEXITSTATUS(status));
}
