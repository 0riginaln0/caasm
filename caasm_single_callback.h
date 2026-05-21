#ifndef AASM_H
#define AASM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define AASM_STATES(...) \
  .states = (AASM_State[]){ __VA_ARGS__ }, \
  .states_count = (sizeof((AASM_State[]){ __VA_ARGS__ }) / sizeof(AASM_State))

#define AASM_EVENTS(...) \
  .events = (AASM_Event[]){ __VA_ARGS__ }, \
  .events_count = (sizeof((AASM_Event[]){ __VA_ARGS__ }) / sizeof(AASM_Event))

#define AASM_TRANSITIONS(...) \
  .transitions = (AASM_Transition[]){ __VA_ARGS__ }, \
  .transitions_count = (sizeof((AASM_Transition[]){ __VA_ARGS__ }) / sizeof(AASM_Transition))

#define AASM_FROM(...) \
  .from = (AASM_State_ID[]){ __VA_ARGS__ }, \
  .from_count = (sizeof((AASM_State_ID[]){ __VA_ARGS__ }) / sizeof(AASM_State_ID))

#define AASM_STATE(state_id, ...)   { .id = (state_id), __VA_ARGS__ }
#define AASM_EVENT(event_id, ...)   { .id = (event_id), __VA_ARGS__ }

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

  AASM_State_ID current_state;
  AASM_State_ID from_state;
  AASM_State_ID to_state;
  AASM_State_ID current_event;
  void          *ctx;
} AASM_Runtime;

bool aasm_init(AASM_Runtime *runtime, void *ctx, const char **err);
bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id);

#ifdef AASM_IMPLEMENTATION

bool aasm_init(AASM_Runtime *runtime, void *ctx, const char **err) {
  runtime->ctx = ctx;

  // TODO: Maybe some magic for a more optimized event dispatching

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

  // Init to NULL all non-existing callbacks. though, maybe they are already...

  return true;
}

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
bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id) {
  void *ctx = runtime->ctx;

  runtime->current_event = event_id;

  // 1. Runtime-level: before_all_events
  if (runtime->before_all_events) runtime->before_all_events(ctx);

  // Let's find the event to fire!
  const AASM_Event *event = NULL;
  for (int i = 0; i < runtime->events_count; i++) {
    if (runtime->events[i].id == event_id) {
      event = &runtime->events[i];
      goto found_event;
    }
  }
  return false;

 found_event:

  // 2. Event-level: before
  if (event->before) event->before(ctx);

  // 3. Event-level: guards
  if (event->guard) {
    bool event_guards_passed = false;
    event_guards_passed = event->guard(ctx);
    if (!event_guards_passed) return false;
  }

  // Is there even a transition for the event from the current state?
  const AASM_Transition *transition = NULL;
  for (int i = 0; i < event->transitions_count; i++) {
    const AASM_Transition *temp_transition = &event->transitions[i];
    for (int i = 0; i < temp_transition->from_count; i++) {
      if (temp_transition->from[i] == runtime->current_state) {
        transition = temp_transition;
        goto found_transition;
      }
    }
  }
  return false;

 found_transition:

  // 4. Transition-level: guards (for the matching transition)
  if (transition->guard) {
    bool transition_guards_passed = false;
    transition_guards_passed = transition->guard(ctx);
    if (!transition_guards_passed) return false;
  }

  // 5. Old state: before_exit
  const AASM_State *old_state = NULL;
  for (int i = 0; i < runtime->states_count; i++) {
    if (runtime->current_state == runtime->states[i].id) {
      old_state = &runtime->states[i];
      break;
    }
  }
  if (old_state->before_exit) old_state->before_exit(ctx);

  // 6. Old state: exit
  if (old_state->exit) old_state->exit(ctx);

  runtime->from_state = runtime->current_state;
  runtime->to_state = transition->to;

  // 7. Runtime-level: after_all_transitions
  if (runtime->after_all_transitions) runtime->after_all_transitions(ctx);

  // 8. Transition-level: after
  if (transition->after) transition->after(ctx);

  // 9. New state: before_enter
  const AASM_State *new_state = NULL;
  for (int i = 0; i < runtime->states_count; i++) {
    if (transition->to == runtime->states[i].id) {
      new_state = &runtime->states[i];
      break;
    }
  }
  if (new_state->before_enter) new_state->before_enter(ctx);

  // 10. New state: enter
  if (new_state->enter) new_state->enter(ctx);

  // 11. UPDATE STATE (runtime->current_state = new_state)
  runtime->current_state = new_state->id;

  // 12. Old state: after_exit
  if (old_state->after_exit) old_state->after_exit(ctx);

  // 13. New state: after_enter
  if (new_state->after_enter) new_state->after_enter(ctx);

  // 14. Event-level: after
  if (event->after) event->after(ctx);

  // 15. Runtime-level: after_all_events
  if (runtime->after_all_events) runtime->after_all_events(ctx);

  return true;
}

#endif
#endif