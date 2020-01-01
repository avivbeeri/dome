internal void FILESYSTEM_loadEventHandler(void* task);

global_variable char* basePath = NULL;
char* getBasePath(void) {

  long path_max;
  size_t size;
  char *buf = NULL;
  char *ptr = NULL;

  if (basePath == NULL) {
#ifdef __MINGW32__
    path_max = PATH_MAX;
#else
    path_max = pathconf(".", _PC_PATH_MAX);
#endif
    if (path_max == -1) {
      size = 1024;
    } else if (path_max > 10240) {
      size = 10240;
    } else {
      size = path_max;
    }

    for (buf = ptr = NULL; ptr == NULL; size *= 2) {
      if ((buf = realloc(buf, size + sizeof(char) * 2)) == NULL) {
        abort();
      }

      ptr = getcwd(buf, size);
      if (ptr == NULL && errno != ERANGE) {
        abort();
      }
    }
    basePath = ptr;
    size_t len = strlen(basePath);
    *(basePath + len) = '/';
    *(basePath + len + 1) = '\0';
  }
  return basePath;
}

void freeBasePath(void) {
  if (basePath != NULL) {
    free(basePath);
  }
}

bool doesFileExist(char* path) {
  return access(path, F_OK) != -1;
}

char* readFileFromTar(mtar_t* tar, char* path, size_t* lengthPtr) {
  // We assume the tar open has been done already
  /* Open archive for reading */
  // mtar_t tar;
  // mtar_open(tar, "game.egg", "r");

  //

  printf("Reading from bundle: %s\n", path);
  mtar_header_t h;
  mtar_find(tar, path, &h);
  size_t length = h.size;
  char* p = calloc(1, length + 1);
  if (mtar_read_data(tar, p, length) != MTAR_ESUCCESS) {
    printf("Error: Couldn't read the data from the bundle.");
    abort();
  }

  if (lengthPtr != NULL) {
    *lengthPtr = length;
  }
  return p;
}

int writeEntireFile(char* path, char* data, size_t length) {
  printf("Writing to filesystem: %s\n", path);
  FILE* file = fopen(path, "wb+");
  if (file == NULL) {
    return errno;
  }
  fwrite(data, sizeof(char), length, file);
  fclose(file);
  return 0;
}

char* readEntireFile(char* path, size_t* lengthPtr) {
  printf("Reading from filesystem: %s\n", path);
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    return NULL;
  }
  char* source = NULL;
  if (fseek(file, 0L, SEEK_END) == 0) {
    /* Get the size of the file. */
    long bufsize = ftell(file);
    /* Allocate our buffer to that size. */
    source = malloc(sizeof(char) * (bufsize + 1));

    /* Go back to the start of the file. */
    if (fseek(file, 0L, SEEK_SET) != 0) { /* Error */ }

    /* Read the entire file into memory. */
    size_t newLen = fread(source, sizeof(char), bufsize, file);
    if ( ferror( file ) != 0 ) {
      fputs("Error reading file", stderr);
    } else {
      if (lengthPtr != NULL) {
        *lengthPtr = newLen;
      }
      source[newLen++] = '\0'; /* Just to be safe. */
    }
  }
  fclose(file);
  return source;
}


