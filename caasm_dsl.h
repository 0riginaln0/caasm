//    Use this DSL in the following way:
//
// #include "aasm_scoped.h"   // defines macros
// static AASM_Runtime runtime = { /* use macros here */ };
// #include "aasm_scoped.h"   // undefines macros
//
//    Good to know facts:
//
// - States and events MUST be defined as enums with values
//   starting at 0 and increasing sequentially.
// - Enum identifiers MUST carry the prefixes STATE_ and EVENT_,
//   e.g. STATE_SLEEPING, EVENT_RUN.
// - The FROM() macro supports at most 8 source states.

#ifdef AASM_SCOPED

#undef AASM_STATES
#undef AASM_EVENTS
#undef AASM_NARGS
#undef AASM_NARGS_
#undef STATE_IDS_1
#undef STATE_IDS_2
#undef STATE_IDS_3
#undef STATE_IDS_4
#undef STATE_IDS_5
#undef STATE_IDS_6
#undef STATE_IDS_7
#undef STATE_IDS_8
#undef STATE_IDS
#undef STATE_IDS_EXPAND
#undef STATE_IDS_CAT
#undef INITIAL_STATE
#undef STATE
#undef EVENT
#undef TRANSITIONS
#undef FROM
#undef TO
#undef BEFORE_ENTER
#undef ENTER
#undef AFTER_ENTER
#undef BEFORE_EXIT
#undef EXIT
#undef AFTER_EXIT
#undef BEFORE
#undef AFTER
#undef GUARD

#undef AASM_SCOPED
#else

#define AASM_STATES(...) \
  .states = (AASM_State[]){ __VA_ARGS__ }, \
  .states_count = (sizeof((AASM_State[]){ __VA_ARGS__ }) / sizeof(AASM_State))

#define AASM_EVENTS(...) \
  .events = (AASM_Event[]){ __VA_ARGS__ }, \
  .events_count = (sizeof((AASM_Event[]){ __VA_ARGS__ }) / sizeof(AASM_Event))

#define AASM_NARGS(...)  AASM_NARGS_(__VA_ARGS__, 8,7,6,5,4,3,2,1,0)
#define AASM_NARGS_( _1, _2, _3, _4, _5, _6, _7, _8, N, ...) N

#define STATE_IDS_1(x)                    STATE_##x
#define STATE_IDS_2(x,y)                  STATE_##x, STATE_##y
#define STATE_IDS_3(x,y,z)                STATE_##x, STATE_##y, STATE_##z
#define STATE_IDS_4(x,y,z,w)              STATE_##x, STATE_##y, STATE_##z, STATE_##w
#define STATE_IDS_5(x,y,z,w,v)            STATE_##x, STATE_##y, STATE_##z, STATE_##w, STATE_##v
#define STATE_IDS_6(x,y,z,w,v,u)          STATE_##x, STATE_##y, STATE_##z, STATE_##w, STATE_##v, STATE_##u
#define STATE_IDS_7(x,y,z,w,v,u,t)        STATE_##x, STATE_##y, STATE_##z, STATE_##w, STATE_##v, STATE_##u, STATE_##t
#define STATE_IDS_8(x,y,z,w,v,u,t,s)      STATE_##x, STATE_##y, STATE_##z, STATE_##w, STATE_##v, STATE_##u, STATE_##t, STATE_##s

#define STATE_IDS(...)              STATE_IDS_EXPAND(AASM_NARGS(__VA_ARGS__), __VA_ARGS__)
#define STATE_IDS_EXPAND(N, ...)    STATE_IDS_CAT(N, __VA_ARGS__)
#define STATE_IDS_CAT(N, ...)       STATE_IDS_##N(__VA_ARGS__)

#define INITIAL_STATE(state_id, ...) \
  { .id = STATE_##state_id, .is_initial = true, __VA_ARGS__ }

#define STATE(state_id, ...) \
  { .id = STATE_##state_id, .is_initial = false, __VA_ARGS__ }

#define EVENT(event_id, ...) \
  { .id = EVENT_##event_id, __VA_ARGS__ }

#define TRANSITIONS(...) \
  .transitions = (AASM_Transition[]){ __VA_ARGS__ }, \
  .transitions_count = sizeof((AASM_Transition[]){ __VA_ARGS__ }) / sizeof(AASM_Transition)

#define FROM(...) \
  .from       = (AASM_State_ID[]){ STATE_IDS(__VA_ARGS__) }, \
  .from_count = sizeof((AASM_State_ID[]){ STATE_IDS(__VA_ARGS__) }) / sizeof(AASM_State_ID)
#define TO(state_id)  .to = STATE_##state_id

#define BEFORE_ENTER(...) \
  .before_enter       = (AASM_Callback[]){ __VA_ARGS__ }, \
  .before_enter_count = sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback)
#define ENTER(...) \
  .enter       = (AASM_Callback[]){ __VA_ARGS__ }, \
  .enter_count = sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback)
#define AFTER_ENTER(...) \
  .after_enter       = (AASM_Callback[]){ __VA_ARGS__ }, \
  .after_enter_count = sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback)
#define BEFORE_EXIT(...) \
  .before_exit       = (AASM_Callback[]){ __VA_ARGS__ }, \
  .before_exit_count = sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback)
#define EXIT(...) \
  .exit       = (AASM_Callback[]){ __VA_ARGS__ }, \
  .exit_count = sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback)
#define AFTER_EXIT(...) \
  .after_exit       = (AASM_Callback[]){ __VA_ARGS__ }, \
  .after_exit_count = sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback)
#define BEFORE(...) \
  .before       = (AASM_Callback[]){ __VA_ARGS__ }, \
  .before_count = sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback)
#define AFTER(...) \
  .after       = (AASM_Callback[]){ __VA_ARGS__ }, \
  .after_count = sizeof((AASM_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Callback)
#define GUARD(...) \
  .guards       = (AASM_Guard_Callback[]){ __VA_ARGS__ }, \
  .guards_count = sizeof((AASM_Guard_Callback[]){ __VA_ARGS__ }) / sizeof(AASM_Guard_Callback)


#define AASM_SCOPED
#endif