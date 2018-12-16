#include <stdlib.h>
#include "animation.h"
#include "main.h"

static void pulse_init(uint8_t led, void *anim_data)
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

static void breathe_init(uint8_t led, void *anim_data)
{
  struct breathe_data *breathe = anim_data;
  breathe->period = 3000;
  breathe->phase = rand() % 3000;
}

static uint16_t breathe_at(uint32_t time_ms, void *anim_data)
{
  struct breathe_data *breathe = anim_data;
  uint16_t semi_per = breathe->period >> 1;
  int rem = semi_per - abs(((time_ms + breathe->phase) % breathe->period) - semi_per);

  return MAX_BRIGHTNESS * (rem * 100 / semi_per) / 100;
}

static void blink_init(uint8_t led, void *anim_data)
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

static void snowfall_init(uint8_t led, void *anim_data)
{
  struct snowfall_data *snowfall = anim_data;

  switch (led) {
  case 3: case 4: case 5: case 12:
    snowfall->layer = 0;
    break;

  case 2: case 18: case 6: case 11:
    snowfall->layer = 1;
    break;

  case 1: case 17: case 7: case 10:
    snowfall->layer = 2;
    break;

  case 0: case 15: case 19: case 9:
    snowfall->layer = 3;
    break;

  default:
    snowfall->layer = 4;
  }
}

static uint16_t snowfall_at(uint32_t time_ms, void *anim_data)
{
  struct snowfall_data *snowfall = anim_data;
  uint16_t idx = time_ms % 2000 / 200;
  return MAX_BRIGHTNESS * (idx == snowfall->layer);
}

static const anim_init_fn anim_init_fns[] = {
  [ANIM_PULSE] = pulse_init,
  [ANIM_BREATHE] = breathe_init,
  [ANIM_BLINK] = blink_init,
  [ANIM_SNOWFALL] = snowfall_init,
};

static const anim_fn anim_fns[] = {
  [ANIM_PULSE] = pulse_at,
  [ANIM_BREATHE] = breathe_at,
  [ANIM_BLINK] = blink_at,
  [ANIM_SNOWFALL] = snowfall_at,
};

void animation_switch_to(uint8_t led, struct animation *anim, enum animations id) {
  anim_init_fns[id](led, &anim->data);
  anim->at = anim_fns[id];
}
