#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define AASM_IMPLEMENTATION
#include "caasm2.h"

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

static const char* state_to_string(AASM_State_ID state) {
  switch (state) {
    case STATE_SLEEPING: return "SLEEPING";
    case STATE_RUNNING:  return "RUNNING";
    case STATE_CLEANING: return "CLEANING";
    default:             return "UNKNOWN";
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

static const AASM_State fsm_states[] = {
  { .id = STATE_SLEEPING, .is_initial = false },
  { .id = STATE_RUNNING,  .is_initial = false },
  { .id = STATE_CLEANING, .is_initial = false },
};

static const AASM_Transition run_transitions[] = {{
    .from = (AASM_State_ID[]){STATE_SLEEPING},
    .from_count = 1,
    .to = STATE_RUNNING,
}};

static const AASM_Transition clean_transitions[] = {{
  .from = (AASM_State_ID[]) { STATE_RUNNING }, .from_count = 1,
  .to = STATE_CLEANING,
}};

static const AASM_Transition sleep_transitions[] = {{
  .from = (AASM_State_ID[]) { STATE_RUNNING, STATE_CLEANING }, .from_count = 2,
  .to = STATE_SLEEPING,
}};

static const AASM_Event fsm_events[] = {
  { .id = EVENT_SLEEP, .transitions = sleep_transitions, .transitions_count = 1 },
  { .id = EVENT_CLEAN, .transitions = clean_transitions, .transitions_count = 1 },
  { .id = EVENT_RUN,   .transitions = run_transitions,   .transitions_count = 1 }
};

static AASM_Runtime runtime = {
  .states = fsm_states,
  .states_count = 3,
  .events = fsm_events,
  .events_count = 3,
};

int main(void) {
  const char *err = NULL;
  bool ok = aasm_init(&runtime, NULL, &err);
  if (ok) {
    printf("Your FSM is fine.\n");
  } else {
    printf("Error with your FSM: %s\n", err);
  }

  printf("Available events: (r)un, (c)lean, (s)leap (type (q)uit to exit)\n\n");

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
      printf("Unknown event: '%s'.\n"
             "Please enter (r)un, (c)lean, (s)leap (type (q)uit to exit)\n\n", line);
      continue;
    }

    bool transition_occurred = aasm_fire_event(&runtime, ev);
    if (transition_occurred) {
      printf("--> Transition succeeded. New state: %s\n\n",
             state_to_string(runtime.current_state));
    } else {
      printf("--> Transition ignored (no valid transition from current state).\n\n");
    }
  }
  return 0;
}