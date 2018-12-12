#include <stdlib.h>
#include "animation.h"
#include "main.h"

static void blink_init(void *anim_data)
{
  struct blink_data *blink = anim_data;
  blink->initial_state = rand() % 2;
}

static uint16_t blink_at(uint32_t time_ms, void *anim_data)
{
  struct blink_data *blink = anim_data;
  uint32_t on = ((time_ms & 0x400) >> 10) ^ blink->initial_state;

  return MAX_BRIGHTNESS * on;
}

static void pulse_init(void *anim_data)
{
  struct pulse_data *pulse = anim_data;
  pulse->period = 3000;
}

static uint16_t pulse_at(uint32_t time_ms, void *anim_data)
{
  struct pulse_data *pulse = anim_data;
  uint16_t semi_per = pulse->period >> 1;
  int rem = semi_per - abs((time_ms % pulse->period) - semi_per);

  return MAX_BRIGHTNESS * (rem * 100 / semi_per) / 100;
}

static const anim_init_fn anim_init_fns[] = {
  [ANIM_PULSE] = pulse_init,
  [ANIM_BLINK] = blink_init,
};

static const anim_fn anim_fns[] = {
  [ANIM_PULSE] = pulse_at,
  [ANIM_BLINK] = blink_at,
};

void animation_switch_to(struct animation *anim, enum animations id) {
  anim_init_fns[id](&anim->data);
  anim->at = anim_fns[id];
}
