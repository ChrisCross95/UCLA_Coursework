#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void forkever() {
   while (1) {
      fork();
      printf("Child processes created!\n"); 
   }
   return;
}

int main (int argc, char** argv) {
    forkever(); 
    return 0; 
}

