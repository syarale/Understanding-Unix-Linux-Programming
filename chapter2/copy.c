#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

void usage() {
  printf("usage: cp source dest\n");
  return;
}

int main(int argc, char* argv[]) {
  static const struct option long_opts[] = {
      {"help", no_argument, NULL, 'h'},
      {NULL, 0, NULL, 0},
  };

  int opt = 0;
  while ((opt = getopt_long(argc, argv, "ih", &long_opts, NULL)) != -1) {
    switch (opt) {
      case 'i':
        printf("arg: i\n");
        break;
      case 'h':
        usage();
        exit(0);
      case '?':
        printf("Try 'copy --help' for more information.\n");
        exit(1);
      default:
        break;
    }
  }

  char* src_path = argv[optind];
  char* des_path = argv[optind + 1];

  // Check source file path
  int src_fd = open(src_path, O_RDWR);
  if (src_fd == -1) {
    if (errno == EISDIR) {
      printf("[ERROR] Copying directory is not supported.\n", src_path);
    } else {
      printf("[ERROR] Failed to source file: %s.\n", src_path);
    }
    exit(-1);
  }
  char* src_file_name = basename(src_path);

  // Check destnation file path
  struct stat des_stat;
  if (stat(des_path, &des_stat) != -1) {
    if (S_ISDIR(des_stat.st_mode)) {
      if (des_path[strlen(des_path) - 1] != '/') {
        strcat(des_path, "/");
      }
      strcat(des_path, src_file_name);
    }
  } else if (errno != ENOENT) {
    printf("[ERROR] Failed to stat %s.\n", des_path);
    exit(-1);
  }

  // printf("[INFO] src_path: %s\n", src_path);
  // printf("[INFO] des_path: %s\n", des_path);

  // If errno is ENOENT, means that file is not exist, create a new file.
  int des_fd = open(des_path, O_RDWR | O_CREAT | O_TRUNC | O_APPEND,
                    S_IRUSR | S_IWUSR | S_IXUSR);
  if (des_fd == -1) {
    printf("[ERROR] Failed to open destnation file: %s\n", des_path);
    exit(-1);
  }

  char* buffer[BUFFER_SIZE];
  ssize_t read_bytes = 0;
  while ((read_bytes = read(src_fd, &buffer, BUFFER_SIZE)) > 0) {
    if (write(des_fd, buffer, read_bytes) < read_bytes) {
      printf("[ERROR] Failed to write data to %s.\n", des_path);
      exit(-1);
    }
  }

  if (read_bytes == -1) {
    printf("[ERROR] Failed to read data from %s.\n", src_path);
    exit(-1);
  }

  close(src_fd);
  close(des_fd);
  return 0;
}