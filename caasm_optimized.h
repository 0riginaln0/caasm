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

  
  const AASM_State * const * state_table; // [state_id] -> state*
  const AASM_Event * const * event_table; // [event_id] -> event*
  const AASM_Transition * const * transition_table; // [state_id * events_count + event_id]
                                                    // -> transition*
} AASM_Runtime;

bool aasm_init(AASM_Runtime *runtime, void *ctx,
               const AASM_State **state_table, int state_table_size,
               const AASM_Event **event_table, int event_table_size,
               const AASM_Transition **transition_table, int transition_table_size,
               char **err);
bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id);

#ifdef AASM_PRETTIER_MACROS
#  include "caasm_prettier_macros.h"
#else
#  include "caasm_macros.h"
#endif

#ifdef AASM_IMPLEMENTATION

bool aasm_init(AASM_Runtime *runtime, void *ctx,
               const AASM_State **state_table, int state_table_size,
               const AASM_Event **event_table, int event_table_size,
               const AASM_Transition **transition_table, int transition_table_size,
               char **err) {
  runtime->ctx = ctx;
  runtime->state_table = state_table;
  runtime->event_table = event_table;
  runtime->transition_table = transition_table;

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

  // Some magic for a more optimized event dispatching
  for (int i = 0; i < state_table_size; i++) state_table[i] = NULL;
  for (int i = 0; i < event_table_size; i++) event_table[i] = NULL;
  for (int i = 0; i < transition_table_size; i++) transition_table[i] = NULL;
  
  if (runtime->states_count != state_table_size) {
    *err = "State table size mismatch";
    return false;
  }
  if (runtime->events_count != event_table_size) {
    *err = "Event table size mismatch";
    return false;
  }
  
  for (int i = 0; i < runtime->states_count; i++) {
    const AASM_State *s = &runtime->states[i];
    if (s->id >= state_table_size) {
      *err = "State ID exceeds state table size";
      return false;
    }
    state_table[s->id] = s;
  }
  for (int e = 0; e < runtime->events_count; e++) {
    const AASM_Event *ev = &runtime->events[e];
    if (ev->id >= event_table_size) {
      *err = "Event ID exceeds event table size";
      return false;
    }
    event_table[ev->id] = ev;

    for (int t = 0; t < ev->transitions_count; t++) {
      const AASM_Transition *tr = &ev->transitions[t];
      for (int f = 0; f < tr->from_count; f++) {
        int from_state = tr->from[f];
        int idx = from_state * event_table_size + ev->id;
        if (idx >= transition_table_size) {
          *err = "Transition index exceeds transition table size";
          return false;
        }
        transition_table[idx] = tr;
      }
    }
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
  const AASM_Event *event = runtime->event_table[event_id];
  if (!event) return false;

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
  int transition_idx = runtime->current_state * runtime->events_count + event_id;
  const AASM_Transition *transition = runtime->transition_table[transition_idx];
  if (!transition) return false;

  // 4. Transition-level: guards (for the matching transition)
  if (transition->guards) {
    for (int i = 0; i < transition->guards_count; i++) {
      if (!transition->guards[i](ctx)) return false;
    }
  }

  // 5. Old state: before_exit
  const AASM_State *old_state = runtime->state_table[runtime->current_state];

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
  const AASM_State *new_state = runtime->state_table[transition->to];

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