#ifndef AASM_MACROS_H
#define AASM_MACROS_H

#define AASM_STATE(state_id, ...)   { .id = (state_id), __VA_ARGS__ }
#define AASM_EVENT(event_id, ...)   { .id = (event_id), __VA_ARGS__ }

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

#ifndef AASM_SINGLE_CALLBACK
  #define AASM_GUARDS(...) \
    .guards = (AASM_Guard_Callback[]){ __VA_ARGS__ }, \
    .guards_count = (sizeof((AASM_Guard_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Guard_Callback))

  #define AASM_AFTER(...) \
    .after = (AASM_Callback[]){ __VA_ARGS__ }, \
    .after_count = (sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback))

  #define AASM_BEFORE(...) \
    .before = (AASM_Callback[]){ __VA_ARGS__ }, \
    .before_count = (sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback))

  #define AASM_BEFORE_ENTER(...) \
    .before_enter = (AASM_Callback[]){ __VA_ARGS__ }, \
    .before_enter_count = (sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback))

  #define AASM_ENTER(...) \
    .enter = (AASM_Callback[]){ __VA_ARGS__ }, \
    .enter_count = (sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback))

  #define AASM_AFTER_ENTER(...) \
    .after_enter = (AASM_Callback[]){ __VA_ARGS__ }, \
    .after_enter_count = (sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback))

  #define AASM_BEFORE_EXIT(...) \
    .before_exit = (AASM_Callback[]){ __VA_ARGS__ }, \
    .before_exit_count = (sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback))

  #define AASM_EXIT(...) \
    .exit = (AASM_Callback[]){ __VA_ARGS__ }, \
    .exit_count = (sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback))

  #define AASM_AFTER_EXIT(...) \
    .after_exit = (AASM_Callback[]){ __VA_ARGS__ }, \
    .after_exit_count = (sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback))
#endif

#endif