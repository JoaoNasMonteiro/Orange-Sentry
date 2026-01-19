#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/arena.h"
#include "../include/exit_status.h"

#define BUF_SIZE 64 

typedef enum {
    STATE_CLOSED = 0,
    STATE_PASSIVE_LISTEN,
    STATE_HONEYPOT,
    STATE_DEVELOPMENT
} SystemState;

SystemState current_state, next_state;


int change_state();
int deactivate_state(uint8_t state);
int activate_state(uint8_t state);

int d_state_closed();
int d_state_dev();
int d_state_honey();
int d_state_pl();

int a_state_closed();
int a_state_dev();
int a_state_honey();
int a_state_pl();


int main(){
  next_state = STATE_CLOSED;
  change_state();
  int user_input;

  while(1){
    if(scanf("%d", &user_input) == 1){
      next_state = (uint8_t)user_input;
      change_state();
    }
    else{
      while(getchar() != '\n');
    }

  }
  return 0;
}

int change_state(){
  
  if (next_state == current_state) return OS_EXIT_SUCCESS;
  
  deactivate_state(current_state);
  activate_state(next_state);

  current_state = next_state;

  return OS_EXIT_SUCCESS;
}

int deactivate_state(uint8_t state){
  switch (state) {
    case STATE_CLOSED:
      d_state_closed();
      break;
    case STATE_DEVELOPMENT:
      d_state_dev();
      break;
    case STATE_HONEYPOT:
      d_state_honey();
      break;
    case STATE_PASSIVE_LISTEN:
      d_state_pl();
      break;
    default:
      printf("Invalid State for Activating\n");
      return OS_EXIT_GEN_FAILURE
  } 

  return OS_EXIT_SUCCESS;
}

int d_state_closed(){
  printf("Deactivating the Closed State\n");
  return OS_EXIT_SUCCESS;
}

int d_state_dev(){
  printf("Deactivating the Development State\n");
  return OS_EXIT_SUCCESS;
}

int d_state_honey(){
  printf("Deactivating the Honeypot State\n");
  return OS_EXIT_SUCCESS;
}

int d_state_pl(){
  printf("Deactivating the Passive Listen State\n");
  return OS_EXIT_SUCCESS;
}

int activate_state(uint8_t state){
  switch (state) {
    case STATE_CLOSED:
      a_state_closed();
      break;
    case STATE_DEVELOPMENT:
      a_state_dev();
      break;
    case STATE_HONEYPOT:
      a_state_honey();
      break;
    case STATE_PASSIVE_LISTEN:
      a_state_pl();
      break;
    default: 
      printf("Invalid State for Deactivating\n");
      break;
  }

  return OS_EXIT_SUCCESS;
}

int a_state_closed(){
  printf("Activating the Closed State\n");
  return OS_EXIT_SUCCESS;
}

int a_state_dev(){
  printf("Activating the Development State\n");
  return OS_EXIT_SUCCESS;
}

int a_state_honey(){
  printf("Activating the Honeypot State\n");
  return OS_EXIT_SUCCESS;
}

int a_state_pl(){
  printf("Activating the Passive Listen State\n");
  return OS_EXIT_SUCCESS;
}


