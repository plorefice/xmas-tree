#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include <stdint.h>

enum animations {
  ANIM_PULSE = 0,
  ANIM_BREATHE,
  ANIM_BLINK,
  ANIM_SNOWFALL,
  ANIM_SNOWFALL_PERS,
  NUM_ANIMATIONS,
};

typedef void (*anim_init_fn)(uint8_t, void *);
typedef uint16_t (*anim_fn)(uint32_t, void *);

struct animation {
  union {
    /* Pulse animation */
    struct pulse_data {
      uint16_t period;
    } pulse;

    /* Breathe animation */
    struct breathe_data {
      uint16_t period;
      uint16_t phase;
    } breathe;

    /* Blink animation */
    struct blink_data {
      uint8_t initial_state;
    } blink;

    /* Snowfall animation */
    struct snowfall_data {
      uint8_t layer;
    } snowfall;
  } data;

  anim_fn at;
};

void animation_switch_to(uint8_t led, struct animation *anim, enum animations id);

#endif
