#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define RED    "\033[31m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

#define AASM_PRETTIER_MACROS
#define AASM_IMPLEMENTATION
#include "caasm.h"

enum State {
  STATE_SLEEPING,
  STATE_RUNNING,
  STATE_CLEANING,
};

enum Event {
  EVENT_RUN,
  EVENT_CLEAN,
  EVENT_SLEEP,
};

typedef struct JobContext {
  AASM_Runtime *runtime;
  char job_name[64];       // job identifier
  char process_name[64];   // name of the current process (for set_process)
  time_t start_time;       // when the job entered RUNNING state
  int run_count;           // number of times the job has been run
  int clean_count;         // number of cleaning cycles
} JobContext;

static const char* state_to_string(AASM_State_ID state) {
  switch (state) {
    case STATE_SLEEPING: return "SLEEPING";
    case STATE_RUNNING:  return "RUNNING";
    case STATE_CLEANING: return "CLEANING";
    default:             return "UNKNOWN";
  }
}
static const char* event_to_string(AASM_Event_ID state) {
  switch (state) {
    case EVENT_RUN:   return "RUN";
    case EVENT_CLEAN: return "CLEAN";
    case EVENT_SLEEP: return "SLEEP";
    default:          return "UNKNOWN";
  }
}

static int parse_event(const char* input, AASM_Event_ID* event) {
  if (strcmp(input, "RUN") == 0 || strcmp(input, "run") == 0 ||
      strcmp(input, "R") == 0   || strcmp(input, "r") == 0) {
    *event = EVENT_RUN;
    return 1;
  }
  if (strcmp(input, "CLEAN") == 0 || strcmp(input, "clean") == 0 ||
      strcmp(input, "C") == 0     || strcmp(input, "c") == 0) {
    *event = EVENT_CLEAN;
    return 1;
  }
  if (strcmp(input, "SLEEP") == 0 || strcmp(input, "sleep") == 0 ||
      strcmp(input, "S") == 0     || strcmp(input, "s") == 0) {
    *event = EVENT_SLEEP;
    return 1;
  }
  return 0;
}

void before_all_events(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  printf("  [%s] I run before all events\n", job->job_name);
}

void after_all_events(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  printf("  [%s] I run after all events\n", job->job_name);
}

void log_status_change(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  AASM_Runtime *rt = job->runtime;

  const char *from = state_to_string(rt->from_state);
  const char *to   = state_to_string(rt->to_state);
  printf("  [%s] changing from %s to %s (event: %s)\n",
         job->job_name, from, to, event_to_string(rt->current_event));
}

void do_something(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  printf("  [%s] do_something: Preparing state...\n", job->job_name);
}

void notify_somebody(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  printf("  [%s] notify_somebody: Sending notification...\n", job->job_name);
}

void state_running_before_enter(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  do_something(ctx);
  notify_somebody(ctx);
  job->start_time = time(NULL);
  job->run_count++;
  char time_buf[9];
  struct tm *tm_info = localtime(&job->start_time);
  strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);
  printf("  [%s] Entered RUNNING state at %s (run #%d)\n",
         job->job_name, time_buf, job->run_count);
}

void state_cleaning_before_enter(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  job->clean_count++;
  printf("  [%s] Entered CLEANING state (clean #%d)\n",
          job->job_name, job->clean_count);
}

void event_run_before(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  printf("  [%s] Preparing to run\n", job->job_name);
}

void event_sleep_after(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  printf("  [%s] Entering sleep mode (cleaning count: %d)\n",
         job->job_name, job->clean_count);
}

void set_process(void *ctx, const char *name) {
  JobContext *job = (JobContext*)ctx;
  strncpy(job->process_name, name, sizeof(job->process_name) - 1);
  job->process_name[sizeof(job->process_name)-1] = '\0';
  printf("  [%s] set_process: Starting process '%s'\n", job->job_name, job->process_name);
}

void transition_after_run(void *ctx) {
  // In a real app, you would retrieve argument from context or from a queue.
  // Here we pass a fixed string, but you could store an argument in JobContext.
  set_process(ctx, "my_job");
}

void log_run_time(void *ctx) {
  JobContext *job = (JobContext*)ctx;
  if (job->start_time != 0) {
    time_t now = time(NULL);
    double seconds = difftime(now, job->start_time);
    printf("  [%s] Job was running for %.0f seconds\n", job->job_name, seconds);
  } else {
    printf("  [%s] Job runtime unknown (start time not set)\n", job->job_name);
  }
}

#include "caasm_dsl.h"
static AASM_Runtime runtime = {
  AASM_STATES(
    INITIAL_STATE(SLEEPING, BEFORE_ENTER(do_something)),

    STATE(RUNNING, BEFORE_ENTER(state_running_before_enter)),

    STATE(CLEANING, BEFORE_ENTER(state_cleaning_before_enter))
  ),
  
  AASM_EVENTS(
    EVENT(RUN, 
      BEFORE(event_run_before), AFTER(notify_somebody),
      TRANSITIONS({FROM(SLEEPING), TO(RUNNING), AFTER(transition_after_run)})),
    
    EVENT(CLEAN,
      TRANSITIONS({FROM(RUNNING), TO(CLEANING), AFTER(log_run_time)})),
    
    EVENT(SLEEP,
      AFTER(event_sleep_after),
      TRANSITIONS({FROM(RUNNING, CLEANING), TO(SLEEPING)})),
  ),
  
  .after_all_transitions = log_status_change,
  .before_all_events = before_all_events,
  .after_all_events = after_all_events
};
#include "caasm_dsl.h"

int main(void) {
  char *err = NULL;

  JobContext job_ctx = {
    .runtime = &runtime,
    .job_name = "MyJob",
    .process_name = "",
    .start_time = 0,
    .run_count = 0,
    .clean_count = 0
  };

  bool ok = aasm_init(&runtime, &job_ctx, &err);
  if (!ok) {
    printf("Error with your FSM: %s\n", err);
    return 1;
  }
  printf("Your FSM is fine.\n");

  printf("Available events: (r)un, (c)lean, (s)leep (type (q)uit to exit)\n\n");

  char line[100];
  while (1) {
    printf("Current state: %s\n", state_to_string(runtime.current_state));
    printf("Enter event: ");

    if (fgets(line, sizeof(line), stdin) == NULL) {
      printf("\nEOF detected. Exiting.\n");
      break;
    }

    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0 ||
        strcmp(line, "q") == 0    || strcmp(line, "e") == 0) {
      printf("Exiting.\n");
      break;
    }

    AASM_Event_ID ev;
    if (!parse_event(line, &ev)) {
      printf(YELLOW"Unknown event: '%s'.\n"RESET
             "Please enter (r)un, (c)lean, (s)leep (type (q)uit to exit)\n\n", line);
      continue;
    }

    bool transition_occurred = aasm_fire_event(&runtime, ev);
    if (transition_occurred) {
      printf(GREEN"Transition succeeded.\n"RESET);
    } else {
      printf(RED"Transition ignored (no valid transition found).\n"RESET);
    }
  }
  return 0;
}