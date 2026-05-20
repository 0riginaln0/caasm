Experiment of bringing Ruby's AASM into C.

examples/traffic_light.c - the most straighforward way to use cAASM.

examples/job.c - fancy macros eliminating boilerplate, interactive demo for sending events, watching the transitions trigger or not!

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
    - before_success
    - success
    - after
- AASM_Transition
  - Fields:
    - from (array of AASM_State_ID)
    - to (AASM_State_ID)
  - Callbacks:
    - guards
    - after
    - success

```ruby
aasm do
  # Global callbacks
  before_all_events :before_all_events
  after_all_transitions :log_all_transitions
  after_all_events :after_all_events


  # State callbacks with all lifecycle hooks
  state :open, initial: true,
        before_exit: :before_exit_open,
        exit: :exit_open,
        after_exit: :after_exit_open

  state :closed,
        before_enter: :before_enter_closed,
        enter: :enter_closed,
        after_enter: :after_enter_closed

  # Event with all possible callbacks
  event :close,
        before_all_events: :before_all_events,
        before: :before_event,
        after: :after_event,
        after_all_events: :after_all_events,
        success: :event_success,
        error: :event_error,
        error_on_all_events: :error_on_all_events,
        ensure: :ensure_event,
        ensure_on_all_events: :ensure_on_all_events,
        before_success: :before_success_event do

    # Inside events
    transitions from: :open, to: :closed,
                after: :transition_after,
                success: :transition_success
  end
end
```