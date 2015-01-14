/* deadlocker.c --- 
 * 
 * Filename: deadlocker.c
 * Description: Type, Function, and Global variable implementations for deadlocker
 * Author: Jordon Biondo
 * Created: Wed Nov  6 21:53:39 2013 (-0500)
 * Last-Updated: Wed Jul  2 18:27:55 2014 (-0400)
 *           By: Jordon Biondo
 *     Update #: 19
 */

/* Commentary: 
 * 
 * Type, Function, and Global variable implementations for deadlocker
 */

/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 */

/* Code: */

/* **************************************************************
 * Includes
 * ************************************************************** */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include "deadlocker.h"


/* **************************************************************
 * Function implementations
 * ************************************************************** */


/**
 * Initialize the procs array
 */
bool init_procs(void) {
  static bool happened = false;
  if (!happened) {
    for (int i = 0; i < PROC_LIMIT; i++) {
      procs[i].active = false;
      procs[i].deadlocked = false;
      for (int j = 0; j < RESOURCE_LIMIT; j++) {
	procs[i].owning[j] = false;
	procs[i].requesting = -1;
      }
    }
    happened = true;
    return true;
  } else {
    return false;
  }
}


/**
 * Initialize proc mutexes, pipes, etc when necessary
 */
bool init_active_procs(void) {
  static bool happened = false;
  if (!happened) {
    for (int i = 0; i < PROC_LIMIT; i++) {
      if (procs[i].active) {

	if (pthread_mutex_init(&(procs[i].msg_mutex), NULL) != 0) {
	  printf("Could not create mutex! %s\n", strerror(errno));
	  clean_procs();
	  exit(-1);
	}
	
	if(pipe(procs[i].messages) < 0) {
	  printf("Could not create pipe! %s\n", strerror(errno));
	  clean_procs();
	  exit(-1);

	} else {
	  // reading proc pipes won't block!
	  int flags = fcntl(procs[0].messages[MSG_OUT], F_GETFL, 0);
	  fcntl(procs[i].messages[MSG_OUT], F_SETFL, flags | O_NONBLOCK);
	}
      }
    }
    happened = true;
    return true;
  } else {
    return false;
  }
    
}


/**
 * Initialize the resource array
 */
bool init_resources(void) {
  static bool happened = false;
  if (!happened) {
    for (int i = 0; i < RESOURCE_LIMIT; i++) {
      resources[i].id = i+1;
      resources[i].owner = -1;
    }
    happened = true;
    return true;
  } else {
    return false;
  }
}


/**
 * Cleanup the memory of procs
 */
bool clean_procs(void) {
  static bool happened = false;
  if (!happened) {
    for (int i = 0; i < PROC_LIMIT; i++) {
      if (procs[i].simulation != (intptr_t)NULL) {
	procs[i].killed = true;
	pthread_join(procs[i].simulation, (void*)NULL);
      }
    }
    for (int i = 0; i < PROC_LIMIT; i++) {
      if (procs[i].killed) {
	pthread_mutex_destroy(&(procs[i].msg_mutex));
	close(procs[i].messages[MSG_IN]);
	close(procs[i].messages[MSG_OUT]);
      }
    }

    happened = true;
    return true;
  } else {
    return false;
  }
}


/**
 * PROC owns RESOURCE
 */
bool assign_owner(int proc, int resource){
  printf("p%d now owns r%d\n", proc + 1 , resource + 1);
  resources[resource].owner = proc;
  procs[proc].owning[resource] = true;
  procs[proc].active = true;
  return true;
}


/**
 * Declare that PROC is requesting RESOURCE
 */
bool request_ownership(int proc, int resource){
  printf("p%d is blocked on r%d\n", proc + 1 , resource + 1);
  procs[proc].requesting = resource;
  procs[proc].active = true;
  return true;
}


/**
 * Is PROC requresting RESOURCE?
 */
bool proc_is_requesting(int proc) {
  return procs[proc].requesting >= 0;
}


/**
 * Is PROC owning RESOURCE?
 */
bool proc_is_owning(int proc, int resource) {
  return procs[proc].owning[resource];
}


/**
 * Get the proc index that owns RESOURCE.
 */
int get_resource_owner(int resource) {
  return resources[resource].owner;
}


/**
 * Send a message to PROC
 */
bool message_to_proc(int blocked_proc, int sender_proc, int receiver_proc){
  probe outgoing_probe = {blocked_proc, sender_proc, receiver_proc};
  int write_amt = 0;
  sim_process* receiver = &(procs[receiver_proc]);
  
  if (!(pthread_mutex_lock(&(receiver->msg_mutex)) == 0 && 
	({
	  bool result = true;
	  write_amt = write(procs[receiver_proc].messages[MSG_IN],
			    probe_bytes(outgoing_probe),
			    sizeof(probe));
	  if (write_amt != sizeof(probe)) {
	    printf("Write failed! %s | %d:%d:%d\n", 
		   strerror(errno), blocked_proc, sender_proc, receiver_proc);
	    result = false;
	  }
	  result;
	}) && (pthread_mutex_unlock(&(receiver->msg_mutex)) == 0))) {
    printf("Warning: Error sending message \"%d:%d:%d\"", 
	   blocked_proc, sender_proc, receiver_proc);
  }
    
  if (write_amt != sizeof(probe)) {
    return false;
  }
  return true;
}


/**
 * Have PROC read a message into DATA,
 * Reading is nonblocking, so if there is no data,
 * false is immediately returned
 */
bool proc_read_message(int proc, probe* data) {
  int read_amt;
  int read_amt_desired = sizeof(probe);
  sim_process* this = &(procs[proc]);

  pthread_mutex_lock(&(this->msg_mutex));
  read_amt = read(procs[proc].messages[MSG_OUT], (byte*)data, read_amt_desired);
  pthread_mutex_unlock(&(this->msg_mutex));
  
  return (read_amt == read_amt_desired);
}


/**
 * Simulated process main thread function
 * Pretends to be a process
 * Starts up the worker and messager threads and waits
 */
void* simulate_process_func(void* this_proc) {
  const int this_index = (int)(intptr_t)this_proc;
  sim_process* this = &(procs[this_index]);
  pthread_create(&(this->worker), NULL, proc_worker_func, this_proc);
  pthread_create(&(this->messager), NULL, proc_messager_func, this_proc);
  pthread_join(this->worker, (void*)NULL);
  pthread_join(this->messager, (void*)NULL);
  pthread_exit(NULL);
}


/**
 * Simulated process main thread function
 */
void* proc_worker_func(void* this_proc) {
  int this_index = (int)(intptr_t)this_proc;
  sim_process* this = &(procs[this_index]);
  while(this->active && !this->killed) {
    proc_sleep();
    if (this->deadlocked) {
      printf("Process %d: reporting deadlock!\n", this_index);
      this->active = false;
    }
  }
  pthread_exit(NULL);
}


/**
 * Simulated process messaging thread function
 */
void* proc_messager_func(void* this_proc) {
  const int this_index = (int)(intptr_t)this_proc;
  sim_process* this = &(procs[this_index]);

  while(!this->killed) {
    proc_sleep();
    if (!this->deadlocked) {
      if (this->requesting >= 0) {
	printf("p%d: new probe:       %d:%d:%d\n", 
	       this_index + 1, this_index + 1, this_index + 1, get_resource_owner(this->requesting) + 1);
	message_to_proc(this_index, this_index, get_resource_owner(this->requesting));
      }
      
      probe incoming_data;
      if (proc_read_message(this_index, &incoming_data)) {
	printf("p%d: reading:         %d:%d:%d\n", 
	       this_index + 1, incoming_data.blocked_proc  + 1, 
	       incoming_data.sender + 1, incoming_data.receiver + 1);

	if (incoming_data.blocked_proc == incoming_data.receiver) {
	  this->deadlocked = true;
	} else if (this->requesting >= 0) {
	  printf("p%d: forwarding:      %d:%d:%d\n", 
		 this_index + 1, incoming_data.blocked_proc  + 1, 
		 incoming_data.sender + 1, incoming_data.receiver + 1);
	  
	  message_to_proc(incoming_data.blocked_proc, 
			  this_index, 
			  get_resource_owner(this->requesting));
	}
      }
    }
  }
  pthread_exit(NULL);
}


/**
 * Main function for process simulation used after parsing
 */
void run_simulation(int argc, char* argv[]) {
  init_active_procs();
  signal(SIGINT, handle_sigint);
  for (int i = 0; i < PROC_LIMIT; i++) {
    if (procs[i].active) {
      pthread_create(&(procs[i].simulation), NULL, simulate_process_func, (void*)(intptr_t)i);
    }
  }
  for (int i = 0; i < PROC_LIMIT; i++) {
    if (procs[i].active) pthread_join(procs[i].simulation, (void*)NULL);
  }
}

/**
 * Sleep
 */
void proc_sleep(void) {
  if (fast_mode) usleep(1000);
  else sleep(2);
}


/**
 * Handle a sigint 
 */
void handle_sigint(int sig) {
  clean_procs();
  pthread_exit(NULL);
  exit(1);
}









