Experiment of bringing Ruby's [AASM](https://github.com/aasm/aasm) into C.

A single-header library for allocation-free FSM definition with powerful DSL.

Interactive demo examples:

Run them with `gcc job2.c -o j2.exe && ./j2.exe`

- `job.c`: separate arrays for states, events, transitions. Begin with this example.
- `job_inplace.c`: In‑place FSM definition using compound literals. (Check this one to compare with the next example)
- `job_macros.c`: Fancy macros eliminating all boilerplate.

All callbacks are optional. The order of calling the callbacks is documented in `caasm.h`.

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
