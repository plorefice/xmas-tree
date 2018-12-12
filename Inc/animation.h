#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include <stdint.h>

enum animations {
  ANIM_PULSE = 0,
  ANIM_BLINK,
  NUM_ANIMATIONS,
};

typedef uint16_t (*anim_fn)(uint32_t, void *);
typedef void (*anim_init_fn)(void *);

struct animation {
  union {
    /* Pulse animation */
    struct pulse_data {
      uint16_t period;
    } pulse;

    /* Blink animation */
    struct blink_data {
      uint8_t initial_state;
    } blink;
  } data;

  anim_fn at;
};

void animation_switch_to(struct animation *anim, enum animations id);

#endif
