#include <stdio.h>
#include <stdlib.h>

void usage() {
  printf("ussage: \n  cp src_file_path des_file_path\n");
  return;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage();
    exit(1);
  }

  return 0;
}