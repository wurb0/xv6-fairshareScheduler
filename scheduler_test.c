#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  int n = 5; // fewer to make visible output

  for(i = 0; i < n; i++){
    if(fork() == 0){
      int pid = getpid();
      for(int j = 0; j < 20000000; j++){
        if(j % 5000000 == 0){
          printf("Child %d running\n", pid);
        }
      }
      exit(0);
    }
  }

  for(i = 0; i < n; i++){
    wait(0);
  }
  printf("DONE\n");
  exit(0);
}
