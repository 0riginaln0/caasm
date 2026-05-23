[cAASM](https://github.com/0riginaln0/caasm) is a single-header library for FSM definition with powerful DSL and a callbacks system.

It's an experiment of bringing Ruby's [AASM](https://github.com/aasm/aasm) into C.

Files:
- `caasm.h` - allows multiple callbacks per slot.
- `caasm_dsl.h` - Pretty scoped macros for eliminating the boilerplate.

Configurations:
- `#define AASM_OPTIMIZE_STATES_LOOKUP`: reduces state search complexity from O(N) to O(1).
- `#define AASM_OPTIMIZE_EVENTS_LOOKUP`: reduces event search complexity from O(N) to O(1).
- `#define AASM_OPTIMIZE_TRANSITIONS_LOOKUP`: reduces transition search complexity from O(N) to O(1), imposing the constraint: only one transition from each state-event pair.

Interactive demo examples:

Run them with `gcc job.c -o j.exe && ./j.exe`

- `job.c`: Separate arrays for states, events, transitions. Begin with this example.
- `job_optimized.c`: The same example as previous, but with configuration for O(1) states, events and transitions lookups.
- `job_inplace.c`: In‑place FSM definition using compound literals. (Check this one to compare with the next example)
- `job_dsl.c`: Fancy macros eliminating all of the boilerplate.
- `job_dsl_w_callbacks.c`: Callbacks usage example.

The Ruby's original AASM Job code:
```ruby
class Job
  include AASM

  aasm do
    state :sleeping, initial: true
    state :running, :cleaning

    event :run do
      transitions from: :sleeping, to: :running
    end

    event :clean do
      transitions from: :running, to: :cleaning
    end

    event :sleep do
      transitions from: [:running, :cleaning], to: :sleeping
    end
  end

end
```

The C AASM Job code:
```c
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

#include "caasm_dsl.h"
static AASM_Runtime runtime = {
  AASM_STATES(
    INITIAL_STATE(SLEEPING),
    STATE(RUNNING),
    STATE(CLEANING),
  ),

  AASM_EVENTS(
    EVENT(RUN,
      TRANSITIONS({FROM(SLEEPING), TO(RUNNING)})
    ),

    EVENT(CLEAN,
      TRANSITIONS({FROM(RUNNING), TO(CLEANING)})
    ),

    EVENT(SLEEP,
      TRANSITIONS({FROM(RUNNING, CLEANING), TO(SLEEPING)})
    ),
  ),
};
#include "caasm_dsl.h"

int main(void) {
  char *err = NULL;
  bool ok = aasm_init(&runtime, NULL, &err);
  bool transition_occurred = aasm_fire_event(&runtime, EVENT_RUN); // true SLEEPING -> RUNNING
  transition_occured = aasm_fire_event(&runtime, EVENT_CLEAN); // true RUNNING -> CLEANING
  transition_occured = aasm_fire_event(&runtime, EVENT_RUN); // false
  transition_occured = aasm_fire_event(&runtime, EVENT_SLEEP); // true CLEANING -> SLEEPING
}
```

All callbacks are optional.

- AASM_Runtime
  - Fields:
    - States (array of states)
    - Events (array of events)
  - Callbacks:
    - before_all_events
    - after_all_transitions
    - after_all_events

- AASM_State
  - Fields:
    - id (AASM_State_ID)
    - is_initial (bool)
  - Callbacks:
    - before_enter
    - enter
    - after_enter
    - before_exit
    - exit
    - after_exit

- AASM_Event
  - Fields:
    - id (AASM_Event_ID)
    - Transitions (array of transitions)
  - Callbacks:
    - before
    - guards
    - after

- AASM_Transition
  - Fields:
    - from (array of AASM_State_ID)
    - to (AASM_State_ID)
  - Callbacks:
    - guards
    - after

# Callback Execution Order

The original library contains numerous callbacks. The AI ​​claims that all of them can be useful.

| Callback / Step | Description | Notes |
|----------------|-------------|-------|
| `before_all_events` | Global hook before any event fires; useful for logging, metrics, or setup common to all events. | Runs regardless of which event is triggered. |
| `event.before` | Event‑specific preparation logic before guards; useful for setting context or validation. | |
| `event.guards` | Conditions that must pass for the event to proceed; e.g., user permissions, system status. | If false → abort, no state change. |
| `transition.guards` | Transition‑specific conditions; e.g., business rules for a particular state change. | If false → abort. |
| `old_state.before_exit` | Preparation before leaving the old state; validation or setup before cleanup. | |
| `old_state.exit` | Actual exit action for the old state; resource cleanup, stopping timers, releasing locks. | |
| `after_all_transitions` | Runs after transition logic but before state update; logging transition details (from/to). | |
| `transition.after` | Transition‑specific logic after the transition; actions for this particular state change. | |
| `new_state.before_enter` | Preparation before entering the new state; validation or setup before initialization. | |
| `new_state.enter` | Actual enter action for the new state; resource acquisition, starting timers, acquiring locks. | |
| **State Update** *(not a callback)* | **Actual state change occurs here.** The `current_state` is updated from old to new. | Critical point in the flow; happens between `new_state.enter` and `old_state.after_exit`. |
| `old_state.after_exit` | Cleanup after leaving the old state, runs **after** the state update. |  |
| `new_state.after_enter` | Cleanup after entering the new state, runs **after** the state update. |  |
| `event.after` | Event‑specific cleanup after the transition completes; notifications or side effects. | |
| `after_all_events` | Global cleanup hook after any event completes; final logging, metrics, or cleanup. | |

