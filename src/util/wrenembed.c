// Converts a Wren source file to a C include file
// Using standard IO only
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WRENEMBED_c
#define WRENEMBED_c

char* WRENEMBED_readEntireFile(char* path, size_t* lengthPtr)
{
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    return NULL;
  }

  char* source = NULL;
  if (fseek(file, 0L, SEEK_END) == 0) {

    // Get the size of the file.
    long bufsize = ftell(file);

    // Allocate our buffer to that size.
    source = malloc(sizeof(char) * (bufsize + 1));

    // Go back to the start of the file.
    if (fseek(file, 0L, SEEK_SET) != 0) {
      // Error
    }

    // Read the entire file into memory.
    size_t newLen = fread(source, sizeof(char), bufsize, file);

    if (ferror(file) != 0) {
      fclose(file);
      return NULL;
    }

    if (lengthPtr != NULL) {
      *lengthPtr = newLen;
    }

    // Add NULL, Just to be safe.
    source[newLen++] = '\0';
  }

  fclose(file);
  return source;
}

int WRENEMBED_encodeAndDump(int argc, char* args[])
{
  if (argc < 2) {
    fputs("Not enough arguments\n", stderr);
    return EXIT_FAILURE;
  }

  size_t length;
  char* fileName = args[1];
  char* fileToConvert = WRENEMBED_readEntireFile(fileName, &length);

  if (fileToConvert == NULL) {
    fputs("Error reading file\n", stderr);
    return EXIT_FAILURE;
  }

  // TODO: Maybe use the filename as a default identifier
  char* moduleName = "wren_module_test";

  if(argc > 2) {
    // TODO: Maybe sanitize moduleName to be valid C identifier?
    moduleName = args[2];
  }

  FILE *fp;
  if(argc > 3) {
    fp = fopen(args[3], "w+");
  } else {
    // Example: main.wren.inc
    fp = fopen(strcat(strdup(fileName), ".inc"), "w+");
  }

  fputs("// auto-generated file, do not modify\n", fp);
  fputs("const char ", fp);
  fputs(moduleName, fp);
  fputs("[", fp);
  fprintf(fp, "%li", length + 1);
  fputs("] = {", fp);

  // Encode chars
  for (size_t i = 0; i < length; i++ ) {
    char* ptr = fileToConvert + i;
    if (*ptr == '\n') {
      fputs("'\\n',", fp);
      fputs("\n", fp);
    } else {
      fputs("'", fp);

      // TODO: Properly test the encoding with different source files
      if (*ptr == '\'') {
        fputs("\\\'", fp);
      } else if (*ptr == '\\') {
        fputs("\\\\", fp);
      } else {
        fwrite(ptr, sizeof(char), 1, fp);
      }

      fputs("', ", fp);
    }
  }

  fputs("\0", fp);
  fputs(" };\n", fp);

  fclose(fp);
  free(fileToConvert);

  return EXIT_SUCCESS;
}

void WRENEMBED_usage() {
  fputs("dome -e | --embed sourceFile [moduleName] [destinationFile]\n", stderr);
}

int WRENEMBED_encodeAndDumpInDOME(int argc, char* args[])
{
  // Function to be used inside DOME that adapts argc and args
  // Removing the first arg.
  int count = argc - 1;

  if (count < 2) {
    fputs("Not enough arguments\n", stderr);
    WRENEMBED_usage();
    return EXIT_FAILURE;
  }

  char* argv[count];
  int index;

  for(index = 0; index < count; index++) {
    argv[index] = args[index + 1];
  }

  int exit = WRENEMBED_encodeAndDump(count, argv);
  if (exit == EXIT_FAILURE) {
    WRENEMBED_usage();
  }
  return exit;
}

#endif
