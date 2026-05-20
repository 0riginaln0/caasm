#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define AASM_IMPLEMENTATION
#include "caasm.h"

#define RED    "\033[31m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

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

void printf_before_all_events(void *ctx) {
  (void)ctx;
  printf("I run before all events\n");
}
void printf_after_all_events(void *ctx) {
  (void)ctx;
  printf("I run after all events\n");
}
void printf_after_all_transitions(void *ctx) {
  (void)ctx;
  printf("I run after all transitions\n");
}

static AASM_Runtime runtime = {
  AASM_STATES(
    AASM_STATE(STATE_SLEEPING, .is_initial = true),
    AASM_STATE(STATE_RUNNING),
    AASM_STATE(STATE_CLEANING)
  ),

  AASM_EVENTS(
    AASM_EVENT(EVENT_RUN,
      AASM_TRANSITIONS({
        AASM_FROM(STATE_SLEEPING), .to = STATE_RUNNING
      })
    ),
    
    AASM_EVENT(EVENT_CLEAN,
      AASM_TRANSITIONS({
        AASM_FROM(STATE_RUNNING), .to = STATE_CLEANING
      })
    ),
    
    AASM_EVENT(EVENT_SLEEP,
      AASM_TRANSITIONS({
        AASM_FROM(STATE_RUNNING, STATE_CLEANING), .to = STATE_SLEEPING
      })
    )
  ),
  .before_all_events = printf_before_all_events,
  .after_all_events = printf_after_all_events,
  .after_all_transitions = printf_after_all_transitions,
};

int main(void) {
  const char *err = NULL;

  bool ok = aasm_init(&runtime, NULL, &err);
  if (!ok) {
    printf("Error with your FSM: %s\n", err);
    return 1;
  }
  printf("Your FSM is fine.\n");

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
      printf(YELLOW"Unknown event: '%s'.\n"RESET
             "Please enter (r)un, (c)lean, (s)leap (type (q)uit to exit)\n\n", line);
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