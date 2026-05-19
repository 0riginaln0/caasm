#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#define SLEEP(seconds) sleep(seconds)
#endif


#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define AASM_IMPLEMENTATION
#include "../caasm.h"

enum traffic_light_state {
  STATE_RED,
  STATE_GREEN,
  STATE_YELLOW,
};

const char* state_to_string(enum traffic_light_state state) {
  switch (state) {
    case STATE_RED:    return "\033[31mSTATE_RED\033[0m";
    case STATE_GREEN:  return "\033[32mSTATE_GREEN\033[0m";
    case STATE_YELLOW: return "\033[33mSTATE_YELLOW\033[0m";
    default:           return "UNKNOWN";
  }
}

enum traffic_light_event {
  EVENT_TIMER,
};

static void red_to_green(void *ctx) {
  (void)ctx;
  printf("\tRED -> GREEN\n");
}

static void green_to_yellow(void *ctx) {
  (void)ctx;
  printf("\tGREEN -> YELLOW\n");
}

static void yellow_to_red(void* ctx) {
  (void)ctx;
  printf("\tYELLOW -> RED\n");
}

static const AASM_Transition timer_transitions[] = {
  {.from = STATE_RED,    .to = STATE_GREEN,  .action = red_to_green   },
  {.from = STATE_GREEN,  .to = STATE_YELLOW, .action = green_to_yellow},
  {.from = STATE_YELLOW, .to = STATE_RED,    .action = yellow_to_red  },
};

static const AASM_Event events[] = {
  {.event_id = EVENT_TIMER, 
   .transitions = timer_transitions, 
   .transitions_count = ARRAY_LEN(timer_transitions)},
};

static const AASM_Description description = {
  .initial_state = STATE_RED,
  .events = events,
  .events_count = ARRAY_LEN(events),
};

int main(void) {
  printf("Traffic light: demo started!\n");
  
  AASM_Runtime runtime = {0};
  aasm_init(&runtime, &description, NULL);
  printf("Initial state: %s\n", state_to_string(runtime.current_state));
  for (int i = 0; i < 9; i++) {
    switch (runtime.current_state) {
      case STATE_RED:    SLEEP(4); break;
      case STATE_GREEN:  SLEEP(5); break;
      case STATE_YELLOW: SLEEP(3); break;
    }
    aasm_fire_event(&runtime, EVENT_TIMER);
    printf("Current state: %s\n", state_to_string(runtime.current_state));
  }
  
  printf("Traffic light: demo ended!\n");
}
