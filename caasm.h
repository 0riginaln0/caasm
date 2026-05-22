#ifndef AASM_H
#define AASM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef uint8_t AASM_State_ID;
typedef uint8_t AASM_Event_ID;

typedef bool (*AASM_Guard_Callback)(void *ctx);
typedef void (*AASM_Callback)(void *ctx);

typedef struct {
  const AASM_State_ID *from; uint8_t from_count;
  AASM_State_ID        to;

  AASM_Guard_Callback *guards; uint8_t guards_count;
  AASM_Callback       *after;  uint8_t after_count;
} AASM_Transition;

typedef struct {
  AASM_Event_ID          id;
  const AASM_Transition *transitions; uint8_t transitions_count;

  AASM_Guard_Callback *guards; uint8_t guards_count;
  uint8_t before_count,
          after_count;
  AASM_Callback *before,
                *after;
} AASM_Event;

typedef struct {
  AASM_State_ID id;
  bool          is_initial;

  uint8_t before_enter_count, enter_count, after_enter_count,
          before_exit_count,  exit_count,  after_exit_count;
  AASM_Callback *before_enter, *enter, *after_enter,
                *before_exit,  *exit,  *after_exit;
} AASM_State;

typedef struct {
  const AASM_State *states; uint8_t states_count;
  const AASM_Event *events; uint8_t events_count;

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

#ifdef AASM_PRETTIER_MACROS
#  include "caasm_prettier_macros.h"
#elif defined(AASM_USE_MACROS)
#  include "caasm_macros.h"
#endif

#ifdef AASM_IMPLEMENTATION

bool aasm_init(AASM_Runtime *runtime, void *ctx, const char **err) {
  runtime->ctx = ctx;

  // TODO: Maybe some magic for a more optimized event dispatching

  int initial_states = 0;
  for (int i = 0; i < runtime->states_count; i++) {
    if (runtime->states[i].is_initial) {
      initial_states++;
      runtime->current_state = runtime->states[i].id;
    }
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
  if (runtime->before_all_events) runtime->before_all_events(ctx); // DO NOT EDIT

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
  if (event->before) {
    for (int i = 0; i < event->before_count; i++) {
      event->before[i](ctx);
    }
  }

  // 3. Event-level: guards
  if (event->guards) {
    for (int i = 0; i < event->guards_count; i++) {
      if (!event->guards[i](ctx)) return false;
    }
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
  if (transition->guards) {
    for (int i = 0; i < transition->guards_count; i++) {
      if (!transition->guards[i](ctx)) return false;
    }
  }

  // 5. Old state: before_exit
  const AASM_State *old_state = NULL;
  for (int i = 0; i < runtime->states_count; i++) {
    if (runtime->current_state == runtime->states[i].id) {
      old_state = &runtime->states[i];
      break;
    }
  }
  if (old_state->before_exit) {
    for (int i = 0; i < old_state->before_exit_count; i++) {
      old_state->before_exit[i](ctx);
    }
  }

  // 6. Old state: exit
  if (old_state->exit) {
    for (int i = 0; i < old_state->exit_count; i++) {
      old_state->exit[i](ctx);
    }
  }

  runtime->from_state = runtime->current_state;
  runtime->to_state = transition->to;

  // 7. Runtime-level: after_all_transitions
  if (runtime->after_all_transitions) runtime->after_all_transitions(ctx); // DO NOT EDIT

  // 8. Transition-level: after
  if (transition->after) {
    for (int i = 0; i < transition->after_count; i++) {
      transition->after[i](ctx);
    }
  }

  // 9. New state: before_enter
  const AASM_State *new_state = NULL;
  for (int i = 0; i < runtime->states_count; i++) {
    if (transition->to == runtime->states[i].id) {
      new_state = &runtime->states[i];
      break;
    }
  }
  if (new_state->before_enter) {
    for (int i = 0; i < new_state->before_enter_count; i++) {
      new_state->before_enter[i](ctx);
    }
  }

  // 10. New state: enter
  if (new_state->enter) {
    for (int i = 0; i < new_state->enter_count; i++) {
      new_state->enter[i](ctx);
    }
  }

  // 11. UPDATE STATE (runtime->current_state = new_state)
  runtime->current_state = new_state->id;

  // 12. Old state: after_exit
  if (old_state->after_exit) {
    for (int i = 0; i < old_state->after_exit_count; i++) {
      old_state->after_exit[i](ctx);
    }
  }

  // 13. New state: after_enter
  if (new_state->after_enter) {
    for (int i = 0; i < new_state->after_enter_count; i++) {
      new_state->after_enter[i](ctx);
    }
  }

  // 14. Event-level: after
  if (event->after) {
    for (int i = 0; i < event->after_count; i++) {
      event->after[i](ctx);
    }
  }

  // 15. Runtime-level: after_all_events
  if (runtime->after_all_events) runtime->after_all_events(ctx); // DO NOT EDIT

  return true;
}

#endif
#endif