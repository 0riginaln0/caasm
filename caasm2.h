#ifndef AASM_H
#define AASM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// #define AASM_EVENT(event_id, ...) \
//   { .id = (event_id), \
//     .transitions = (AASM_Transition[]){ __VA_ARGS__ }, \
//     .transitions_count = (sizeof((AASM_Transition[]){ __VA_ARGS__ }) / sizeof(AASM_Transition)) }

typedef uint8_t AASM_State_ID;
typedef uint8_t AASM_Event_ID;

typedef bool (*AASM_Guard_Callback)(void *ctx);
typedef void (*AASM_Callback)(void *ctx);

typedef struct {
  const AASM_State_ID *from;
  uint8_t              from_count;
  AASM_State_ID        to;

  // TODO: These fields should be arrays of callbacks, not a single callback
  AASM_Guard_Callback guard;
  AASM_Callback       after;
} AASM_Transition;

typedef struct {
  AASM_Event_ID          id;
  const AASM_Transition *transitions;
  uint8_t                transitions_count;

  // TODO: These fields should be arrays of callbacks, not a single callback
  AASM_Guard_Callback guard;
  AASM_Callback       before;
  AASM_Callback       after;
} AASM_Event;

typedef struct {
  AASM_State_ID id;
  bool          is_initial;

  // TODO: These fields should be arrays of callbacks, not a single callback
  AASM_Callback before_enter;
  AASM_Callback enter;
  AASM_Callback after_enter;
  AASM_Callback before_exit;
  AASM_Callback exit;
  AASM_Callback after_exit;
} AASM_State;

typedef struct {
  const AASM_State *states;
  uint8_t           states_count;
  const AASM_Event *events;
  uint8_t           events_count;

  // These one must only accept one callback per each. Not an array of callbacks.
  AASM_Callback before_all_events;
  AASM_Callback after_all_transitions;
  AASM_Callback after_all_events;

  AASM_State_ID  current_state;
  void          *ctx;
} AASM_Runtime;

bool aasm_init(AASM_Runtime *runtime, void *ctx, const char **err);
bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id);

#ifdef AASM_IMPLEMENTATION

bool aasm_init(AASM_Runtime *runtime, void *ctx, const char **err) {
  runtime->ctx = ctx;

  // TODO: Maybe some magic for a more optimized event dispatching
  // can return false if no intial state found or several marked as such.
  int initial_states = 0;
  for (int i = 0; i < runtime->states_count; i++) {
    if (runtime->states[i].is_initial) initial_states++;
  }
  if (initial_states != 1) {
    if (initial_states == 0) {
      *err = "Your FSM does not have an initial state";
    } else {
      *err = "Your FSM has several initial states";
    }
    return false;
  }

  return true;
}

bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id) {
  /*
    // Callback execution order
    // 1. Runtime-level: before_all_events
    runtime->before_all_events

    // 2. Event-level: before
    event->before

    // 3. Event-level: guards
    event->guards

    // 4. Transition-level: guards (for the matching transition)
    transition->guards

    // If any guard fails (steps 3 or 4), the transition is aborted and
    // no further callbacks execute

    // 5. Old state: before_exit
    old_state->before_exit

    // 6. Old state: exit
    old_state->exit

    // 7. Runtime-level: after_all_transitions
    runtime->after_all_transitions

    // 8. Transition-level: after
    transition->after

    // 9. New state: before_enter
    new_state->before_enter

    // 10. New state: enter
    new_state->enter

    // 11. UPDATE STATE (runtime->current_state = new_state)

    // 12. Old state: after_exit
    old_state->after_exit

    // 13. New state: after_enter
    new_state->after_enter

    // 14. Event-level: after
    event->after

    // 15. Runtime-level: after_all_events
    runtime->after_all_events
  */


  // Let's find the event to fire!
  const AASM_Event *event = NULL;
  // TODO: find an event to dispatch
  if (event == NULL) return false;

  // Let's try to trigger the transitions of this event
  // TODO: find transition. if transition successed, return true



  return false;
}

#endif
#endif