#ifndef AASM_H
#define AASM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define AASM_EVENT(event_id, ...) \
  { .id = (event_id), \
    .transitions = (AASM_Transition[]){ __VA_ARGS__ }, \
    .transitions_count = (sizeof((AASM_Transition[]){ __VA_ARGS__ }) / sizeof(AASM_Transition)) }


typedef uint8_t AASM_State_ID;
typedef uint8_t AASM_Event_ID;

typedef bool (*AASM_Guard_Callback)(void *ctx);
typedef void (*AASM_Callback)(void *ctx);

typedef struct {
  const AASM_State_ID *from;
  uint8_t             from_count;
  AASM_State_ID       to;

  AASM_Guard_Callback guards;
  AASM_Callback       after;
  AASM_Callback       success;
} AASM_Transition;

typedef struct {
  AASM_Event_ID         id;
  const AASM_Transition *transitions;
  uint8_t               transitions_count;

  AASM_Guard_Callback guards;
  AASM_Callback       before;
  AASM_Callback       before_success;
  AASM_Callback       success;
  AASM_Callback       after;
} AASM_Event;

typedef struct {
  AASM_State_ID id;
  bool          is_initial;

  AASM_Callback before_enter;
  AASM_Callback enter;
  AASM_Callback after_enter;
  AASM_Callback before_exit;
  AASM_Callback exit;
  AASM_Callback after_exit;
} AASM_State;

typedef struct {
  const AASM_State   *states;
  uint8_t            states_count;
  const AASM_Event   *events;
  uint8_t            events_count;

  AASM_Callback before_all_events;
  AASM_Callback after_all_transitions;
  AASM_Callback after_all_events;

  AASM_State_ID current_state;
  void          *ctx;
} AASM_Runtime;

bool aasm_init(AASM_Runtime *runtime, void *ctx);
bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id);


#ifdef AASM_IMPLEMENTATION

void aasm_init(AASM_Runtime *runtime, const AASM_Description *description, void *ctx) {
  runtime->ctx = ctx;
  // TODO: Maybe some magic for a more optimized event dispatching
}

bool aasm_fire_event(AASM_Runtime *runtime, AASM_Event_ID event_id) {
  const AASM_Description *description = runtime->description;

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