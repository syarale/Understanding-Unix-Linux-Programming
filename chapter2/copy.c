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

static bool is_overwrite_prompt = false;

void usage() {
  printf("[INFO] usage: cp SOURCE DEST\n");
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
        is_overwrite_prompt = true;
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

  if (optind + 2 != argc) {
    printf("[ERROR] SOURCE and DEST arguments are required.\n");
    usage();
    return 0;
  }

  char* src_path = argv[optind];
  char* des_path = argv[optind + 1];

  // Check source file path
  int src_fd = open(src_path, O_RDWR);
  if (src_fd == -1) {
    if (errno == EISDIR) {
      printf("[ERROR] Copying directory is not supported.\n", src_path);
    } else {
      printf("[ERROR] Failed to open SOURCE: %s\n", src_path);
    }
    exit(-1);
  }
  char* src_file_name = basename(src_path);

  // Check destnation file path
  struct stat src_stat, des_stat;
  if (stat(src_path, &src_stat) == -1) {
    printf("[ERROR] Failed to stat SOURCE: %s", src_path);
    exit(-1);
  }

  if (stat(des_path, &des_stat) != -1) {
    if (src_stat.st_ino == des_stat.st_ino &&
        src_stat.st_dev == des_stat.st_dev) {
      printf("[INFO] SOURCE and DEST are the same file.\n");
      return 0;
    }

    if (S_ISDIR(des_stat.st_mode)) {
      if (des_path[strlen(des_path) - 1] != '/') {
        strcat(des_path, "/");
      }
      strcat(des_path, src_file_name);
    } else {
      char overwrite_info[4];
      if (is_overwrite_prompt) {
        scanf("%4s", overwrite_info);
        if (strcmp(overwrite_info, "yes") != 0 &&
            strcmp(overwrite_info, "y") != 0) {
          return 0;  // not overwrite destination file and exit
        }
      }
    }
  } else if (errno != ENOENT) {
    printf("[ERROR] Failed to stat  DEST: %s\n", des_path);
    exit(-1);
  }

  // If errno is ENOENT, means that file is not exist, create a new file.
  int des_fd = open(des_path, O_RDWR | O_CREAT | O_TRUNC | O_APPEND,
                    S_IRUSR | S_IWUSR | S_IXUSR);
  if (des_fd == -1) {
    printf("[ERROR] Failed to open DEST: %s\n", des_path);
    exit(-1);
  }

  char* buffer[BUFFER_SIZE];
  ssize_t read_bytes = 0;
  while ((read_bytes = read(src_fd, &buffer, BUFFER_SIZE)) > 0) {
    if (write(des_fd, buffer, read_bytes) < read_bytes) {
      printf("[ERROR] Failed to write data to DEST: %s\n", des_path);
      exit(-1);
    }
  }

  if (read_bytes == -1) {
    printf("[ERROR] Failed to read data from SOURCE: %s\n", src_path);
    exit(-1);
  }

  close(src_fd);
  close(des_fd);
  return 0;
}