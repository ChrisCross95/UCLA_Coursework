#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>


/*
This program should take one commandline 
argument: the number of megabytes of memory 
it will use. When run, it should allocate an
array, and constantly stream through the array, 
touching each entry. The program should do this
indefinitely, or, perhaps, for a certain amount
of time also specified at the command line.
*/

#define MB 1000000
int 
main (int argc, char** argv) {

  if (argc != 2) { 
    fprintf(stderr, "Error: argument no.\n");
    exit(1);
  }
  
  int array_size = atoi(argv[1])*1000000*sizeof(int); 
  
  int* useless_array;
  if  ( (useless_array = (int *) malloc(array_size)) == NULL) {
    printf("oh shit\n");
    exit(1);
  }

  int index; 
  while (1) {
    for (index = 0; index < array_size; ++index) {
      useless_array[index] = 0; 

      printf("%d\n",index);

      if (index == array_size - 1) {
        index = 0;
      }

    }
  }

  exit(0);
}
