#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#define EXIT_SUCCESS 0
#define EXIT_GEN_FAILURE 1
#define EXIT_MEM_ALLOC_FAILURE 2
#define EXIT_USER_INPUT_FAILURE 3

#define STATE_CLOSED 100
#define STATE_PASSIVE_LISTEN 200
#define STATE_HONEYPOT 300
#define STATE_DEVELOPMENT 400



convert_string_to_int(char* string){


  int main(int argc, char *argv[])
{
  
  int current_state = STATE_CLOSED;
  int next_state;


  if (argc == 1) {
    printf("Execution path: %s \nFirst argument: %s\n", argv[0], argv[1]);
  } else {
    printf("Usage: controller [desired_state]");
    return EXIT_USER_INPUT_FAILURE;
  }

  
  next_state = 
  
  printf("current_state = %d\nnext_state = %d\n", current_state, next_state);
  
  current_state = next_state;


  return EXIT_SUCCESS;
}

convert_string_to_int(char* string){
  char* endptr;

  errno = 0;

  long int num = strtol(str, endptr, 10);

  if (endptr == str) {
    fprintf(stderr, "Error: No digits were found in the string.\n");
  } else if (*endptr != '\\0') {
    fprintf(stderr, "Error: Non-numeric characters found after the number: %s\n", endptr);
  } else if (errno == ERANGE) {
    fprintf(stderr, "Error: Number out of range for int.\n");
  } else {
    return num;
  }

  return EXIT_GEN_FAILURE;
}
