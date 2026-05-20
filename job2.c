#include <stdbool.h>
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
  return 0;
}