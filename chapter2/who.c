#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

#define SHOW_HOST 0

void show_time(long time_val) {
  char* cp = ctime(&time_val);
  printf("%12.12s", cp + 4);
}

void show_info(struct utmp* utmp_ptr) {
  if (utmp_ptr->ut_type != USER_PROCESS) { /* users only */
    return;
  }

  printf("% -8.8s", utmp_ptr->ut_name);
  printf(" ");
  printf("% -8.8s", utmp_ptr->ut_line);
  printf(" ");
  show_time(utmp_ptr->ut_time);
  printf(" ");

#ifdef SHOW_HOST
  printf("(%s)", utmp_ptr->ut_host);
#endif

  printf("\n");
  return;
}

int main() {
  int utmp_fd;
  struct utmp utmp_buf;

  ssize_t nbytes = sizeof(utmp_buf);

  if ((utmp_fd = open(UTMP_FILE, O_RDONLY)) == -1) {
    perror(UTMP_FILE);
    exit(1);
  }

  while (read(utmp_fd, &utmp_buf, nbytes) == nbytes) {
    show_info(&utmp_buf);
  }

  close(utmp_fd);

  return 0;
}