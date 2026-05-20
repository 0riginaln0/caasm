Experiment of bringing Ruby's [AASM](https://github.com/aasm/aasm) into C.

A single-header library for allocation-free FSM definition with powerful DSL and a callbacks system.

Interactive demo examples:

Run them with `gcc job.c -o j.exe && ./j.exe`

- `job.c`: separate arrays for states, events, transitions. Begin with this example.
- `job_inplace.c`: In‑place FSM definition using compound literals. (Check this one to compare with the next example)
- `job_macros.c`: Fancy macros eliminating all boilerplate.


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
  )
};
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

