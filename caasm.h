#ifndef AASM_H
#define AASM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define AASM_EVENT(event_id, ...) \
  { .id = (event_id), \
    .transitions = (AASM_Transition[]){ __VA_ARGS__ }, \
    .transitions_count = (sizeof((AASM_Transition[]){ __VA_ARGS__ }) / sizeof(AASM_Transition)) }


// Act As State Machine in C

// TODO: replacable uint8_t type for state and event identifiers
typedef uint8_t AASM_State_ID;
typedef uint8_t AASM_Event_ID;

// TODO: replacable bool type
typedef bool (*AASM_Guard_Callback)(void *ctx);
typedef void (*AASM_Action_Callback)(void *ctx);

typedef struct {
  AASM_State_ID from;
  AASM_State_ID to;
  AASM_Guard_Callback guard; // optional. NULL = always allow transition
  AASM_Action_Callback action; // optional. NULL = no action triggered after transition
} AASM_Transition;

typedef struct {
  AASM_Event_ID id;
  const AASM_Transition *transitions;
  uint8_t transitions_count;
} AASM_Event;

typedef struct {
  AASM_State_ID initial_state;
  const AASM_Event *events;
  uint8_t events_count;
} AASM_Description;

typedef struct {
  const AASM_Description *description;
  AASM_State_ID current_state;
  void *ctx;
} AASM_Runtime;

void aasm_init(AASM_Runtime *runtime, const AASM_Description *description, void *ctx);
bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id);


#ifdef AASM_IMPLEMENTATION

void aasm_init(AASM_Runtime *runtime, const AASM_Description *description, void *ctx) {
  runtime->description = description;
  runtime->current_state = description->initial_state;
  runtime->ctx = ctx;
}

bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id) {
  const AASM_Description *description = runtime->description;

  // Let's find the event to fire!
  const AASM_Event *event = NULL;
  for (uint8_t i = 0; i < description->events_count; i++) {
    if (description->events[i].id == event_id) {
      event = &description->events[i];
      break;
    }
  }
  
  if (event == NULL) return false;

  // Let's try to trigger the transitions of this event
  for (uint8_t i = 0; i < event->transitions_count; i++) {
    const AASM_Transition *transition = &event->transitions[i];
    
    if (transition->from != runtime->current_state) continue;

    bool unguarded = transition->guard == NULL;
    bool transition_allowed = unguarded ? true : transition->guard(runtime->ctx);
    if (!transition_allowed) continue;

    runtime->current_state = transition->to;
    if (transition->action != NULL) transition->action(runtime->ctx);
    return true;
  }
  
  return false;
}

#endif
#endif