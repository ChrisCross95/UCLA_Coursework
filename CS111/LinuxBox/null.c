#include<stdlib.h>
#include<stdio.h>

int
main (int argc, char** argv) {

  int *ptr;
  ptr = (int *) malloc(100*sizeof(int)); 
  ptr[100] = 69;
  ptr[101] = 0;
  ptr[102] = 0;

  free(&ptr[43]);
  free(ptr);
  ptr[43] = 2; 
  exit(0); 


}
