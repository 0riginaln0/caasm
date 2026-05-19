#include <stdio.h>
#include <string.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define AASM_IMPLEMENTATION
#include "../caasm.h"

enum Job_State {
  STATE_SLEEPING,
  STATE_RUNNING,
  STATE_CLEANING,
};

enum Job_Event {
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
      strcmp(input, "R") == 0 || strcmp(input, "r") == 0) {
    *event = EVENT_RUN;
    return 1;
  }
  if (strcmp(input, "CLEAN") == 0 || strcmp(input, "clean") == 0 ||
      strcmp(input, "C") == 0 || strcmp(input, "c") == 0) {
    *event = EVENT_CLEAN;
    return 1;
  }
  if (strcmp(input, "SLEEP") == 0 || strcmp(input, "sleep") == 0 ||
      strcmp(input, "S") == 0 || strcmp(input, "s") == 0) {
    *event = EVENT_SLEEP;
    return 1;
  }

  return 0;
}

static const AASM_Event events[] = {
  AASM_EVENT(EVENT_RUN,
    {.from = STATE_SLEEPING, .to = STATE_RUNNING}
  ),

  AASM_EVENT(EVENT_CLEAN,
    {.from = STATE_RUNNING,  .to = STATE_CLEANING}
  ),

  AASM_EVENT(EVENT_SLEEP,
    {.from = STATE_RUNNING,  .to = STATE_SLEEPING},
    {.from = STATE_CLEANING, .to = STATE_SLEEPING},
  )
};

static const AASM_Description description = {
  .initial_state = STATE_SLEEPING,
  .events = events,
  .events_count = ARRAY_LEN(events),
};

int main(void) {
  printf("Job: demo started!\n");

  printf("Available events: (r)un, (c)lean, (s)leap (type 'quit' to exit)\n\n");

  AASM_Runtime runtime = {0};
  aasm_init(&runtime, &description, NULL);

  char line[100];
  while (1) {
    printf("Current state: %s\n", state_to_string(runtime.current_state));
    printf("Enter event: ");

    if (fgets(line, sizeof(line), stdin) == NULL) {
      printf("\nEOF detected. Exiting.\n");
      break;
    }

    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
      printf("Exiting.\n");
      break;
    }

    AASM_Event_ID ev;
    if (!parse_event(line, &ev)) {
      printf("Unknown event: '%s'. Please enter RUN, CLEAN, or SLEEP.\n\n", line);
      continue;
    }

    bool transition_occurred = aasm_fire_event(&runtime, ev);
    if (transition_occurred) {
      printf("--> Transition succeeded. New state: %s\n\n",
             state_to_string(runtime.current_state));
    } else {
      printf(
          "--> Transition ignored (no valid transition from current "
          "state).\n\n");
    }
  }

  printf("Job: demo ended!\n");
}