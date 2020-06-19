
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <threads.h>

#define _POSIX_C_SOURCE >= 199309L

#include <time.h>

#include "common/helpers/sign.h"

#ifndef SEED
  #define SEED "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN"
#endif

#define IOTA_SECURITY_LEVEL 2

char **addresses;



struct time_diff {
  struct timespec start_time;
  struct timespec end_time;
};

typedef enum {

  E_OK = EXIT_SUCCESS,
  E_ERR,
  E_FARGV,
  E_CMDARGV
} cmd_argv_enum;

struct parsed_arguments {
  // -t <param, > 0 >
  char t_flag;
  size_t no_thread;

  // -a <param, > 0>
  char a_flag;
  size_t no_address;

  // help
  char h_flag;
};
static const char *help_msg = "-t <int value > 0> -a <int value >0>";

struct thread_gen_address_s{
  size_t start;
  size_t end;
  const char *seed;
};

void generate_addresses(size_t start, size_t end, const char *seed) {
  char *address = NULL;
  for (size_t i = start; i < end; i++) {
    address = iota_sign_address_gen_trytes(seed, i, IOTA_SECURITY_LEVEL);
    if (address == NULL) {
      fprintf(stderr, " Unable to create an address \n");
      exit(EXIT_FAILURE);
    }
    addresses[i] = address;

  }
}

int thr_gen_addr(void *argv) {

  struct thread_gen_address_s *struct_data = (struct thread_gen_address_s *) argv;
  generate_addresses(struct_data->start, struct_data->end, struct_data->seed);
}

cmd_argv_enum parse_cmd_argv(size_t arr_size, char *argv[static arr_size], struct parsed_arguments *parg_argv) {
  if (argv == NULL || parg_argv == NULL) {
    return E_FARGV;
  }

  for (size_t i = 0; i < arr_size; i++) {
    // no threads
    if (!strncmp("-t", argv[i], 2) && parg_argv->t_flag == 0) {
      (parg_argv)->no_thread = atol(argv[i + 1]);
      (parg_argv)->t_flag = 1;
      i++;
      // no address
    } else if (!strncmp("-a", argv[i], 2) && (parg_argv)->a_flag == 0) {
      (parg_argv)->no_address = atol(argv[i + 1]);
      (parg_argv)->a_flag = 1;
      i++;
    }  // help
    else if (!strncmp("-h", argv[i], 2) && (parg_argv)->h_flag == 0) {
      (parg_argv)->h_flag = 1;
    }
  }

  if ((parg_argv)->a_flag == 0 && (parg_argv)->t_flag == 0) {
    return E_CMDARGV;
  }

  return E_OK;
}


int main(int argc, char *argv[static argc]) {

  long double time_diff = 0;
  struct time_diff perf_time;
  memset(&perf_time, 0, sizeof perf_time);

  size_t no_address_per_thread = 0;

  struct parsed_arguments parsed_argv;
  memset(&parsed_argv, 0, sizeof(parsed_argv));

  cmd_argv_enum argv_ret_val = E_OK;
  argv_ret_val = parse_cmd_argv(argc, argv, &parsed_argv);

  if (argv_ret_val != E_OK || parsed_argv.h_flag != 0) {
    fprintf(stderr, "%s %s", argv[0], help_msg);
    return EXIT_FAILURE;
  }

  // ATTENTION: no_address should be divisible by no_thread, cause it could be rounded
  no_address_per_thread = parsed_argv.no_address / parsed_argv.no_thread;

  //Lazy to check it;
  addresses = calloc(parsed_argv.no_address, sizeof(char *));

  thrd_t threads[parsed_argv.no_thread];
  struct thread_gen_address_s thread_func_argv[parsed_argv.no_thread];

  clock_gettime(CLOCK_REALTIME, &(perf_time.start_time));
  for (size_t i = 0; i < parsed_argv.no_thread; i++) {


    if (i == (parsed_argv.no_thread - 1)) {
      thread_func_argv[i].start = i * no_address_per_thread;
      thread_func_argv[i].end =  ((i + 1) * no_address_per_thread) + parsed_argv.no_address % parsed_argv.no_thread;
      thread_func_argv[i].seed = SEED;

    } else {

      thread_func_argv[i].start = i * no_address_per_thread;
      thread_func_argv[i].end = (i + 1) * no_address_per_thread;
      thread_func_argv[i].seed = SEED;

    }

      thrd_create(&threads[i], thr_gen_addr, &thread_func_argv[i]);
  }

  for (size_t i = 0; i < parsed_argv.no_thread; i++) {

        thrd_join(threads[i], NULL);
  }

  for (size_t i = 0; i < parsed_argv.no_address; i++) {
    fprintf(stdout, "%ld: %s\n", i, addresses[i]);
  }

  clock_gettime(CLOCK_REALTIME, &(perf_time.end_time));


  time_diff = ((perf_time.end_time.tv_sec - perf_time.start_time.tv_sec) +
               (perf_time.end_time.tv_nsec - perf_time.start_time.tv_nsec) / 1e9);

  printf(
      "Number of threads: %lu "
      "Number of addresses: %lu "
      "Number of addresses per thread: %lu "
      "time to complete (sec):  %Lf \n",
      parsed_argv.no_thread, parsed_argv.no_address, no_address_per_thread, time_diff);



  for (size_t i = 0; i < parsed_argv.no_address; i++) {
    free(addresses[i]);
  }
  free(addresses);

  return EXIT_SUCCESS;
}

