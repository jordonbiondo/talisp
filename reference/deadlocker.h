/* deadlocker.h --- 
 * 
 * Filename: deadlocker.h
 * Description: Type, Function, and Global variable definitions for deadlocker
 * Author: Jordon Biondo
 * Created: Wed Nov  6 21:53:39 2013 (-0500)
 * Last-Updated: Thu Nov  7 10:16:58 2013 (-0500)
 *           By: Jordon Biondo
 *     Update #: 26
 */

/* Commentary: 
 * 
 * Type, Function, and Global variable definitions for deadlocker
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

#ifndef DEADLOCKER_H
#define DEADLOCKER_H

/* **************************************************************
 * Includes
 * ************************************************************** */

#include <pthread.h>
#include <stdbool.h>

/* **************************************************************
 * Macros
 * ************************************************************** */

/**
 * make number of procs
 */
#ifndef PROC_LIMIT
#define PROC_LIMIT 32
#endif

/**
 * Max number of resources
 */
#ifndef RESOURCE_LIMIT
#define RESOURCE_LIMIT 32
#endif

/**
 * Pipe ends
 */
#define MSG_IN 1
#define MSG_OUT 0

/* **************************************************************
 * Types
 * ************************************************************** */

/**
 * Resource type
 */
typedef struct {
  int id;	// resource number
  int owner;	// proc index in procs
} resource;

/**
 * Simulated process type
 */
typedef struct {
  pthread_t simulation;		// the simulated process thread id
  pthread_t worker;		// the worker thread, reports deadlock
  pthread_t messager;		// sends/recieves/forwards probes, finds deadlock
  int messages[2];		// processes communication pipe
  pthread_mutex_t msg_mutex;    // mutex for safe pipe IO  
  int requesting;		// index of requested resource
  bool owning[RESOURCE_LIMIT];	// list of owned resource indices
  bool active;			// is the process active? (used in the config)
  bool deadlocked;		// true when deadlock found
  bool killed;		        // when to stop all process threads
} sim_process;


/**
 * Message data type
 */
typedef char byte;


/**
 * Probe data type
 */
typedef struct {
  int blocked_proc;
  int sender;
  int receiver;
} probe;


/**
 * Turn probe into byte* for pipe writing
 */
#define probe_bytes(probe) ((byte*)&probe)

/* **************************************************************
 * Globals
 * ************************************************************** */

/**
 * If true, processes will practically not sleep
 */
bool fast_mode;

/**
 * List of procs
 */
sim_process procs[PROC_LIMIT];


/**
 * List of available resources
 */
resource resources[RESOURCE_LIMIT];


/* **************************************************************
 * Functions
 * ************************************************************** */

/**
 * Initialize the procs array
 */
bool init_procs(void);

/**
 * Initialize proc mutexes, pipes, etc when necessary
 */
bool init_active_procs(void);

/**
 * Initialize the resource array
 */
bool init_resources(void);

/**
 * Cleanup the memory of procs
 */
bool clean_procs(void);

/**
 * PROC owns RESOURCE
 */
bool assign_owner(int proc, int resource);

/**
 * Declare that PROC is requesting RESOURCE
 */
bool request_ownership(int proc, int resource);

/**
 * Is PROC requresting RESOURCE?
 */
bool proc_is_requesting(int proc);

/**
 * Is PROC owning RESOURCE?
 */
bool proc_is_owning(int proc, int resource);

/**
 * Get the proc index that owns RESOURCE.
 */
int get_resource_owner(int resource);
  
/**
 * Send a message to PROC
 */
bool message_to_proc(int blocked_proc, int sender_proc, int receiver_proc);

/**
 * Have PROC read a message into DATA
 */
bool proc_read_message(int proc, probe* data);

/**
 * Thread function to simulate a process
 */
void* simulate_process_func(void* this_proc);

/**
 * Simulated process main thread function
 */
void* proc_worker_func(void* this_proc);

/**
 * Simulated process messaging thread function
 */
void* proc_messager_func(void* this_proc);

/**
 * Main entry point 
 */
void run_simulation(int argc, char* argv[]);

/**
 * Sleep
 */
void proc_sleep(void);
  
/**
 * Handle a sigint 
 */
void handle_sigint(int sig);


#endif /* DEADLOCKER_H */













