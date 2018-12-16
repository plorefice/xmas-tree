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

static const uint8_t blink_initial_on[] = {
  3, 18, 1, 15, 14, 5, 11, 7, 9, 13
};

static void blink_init(uint8_t led, void *anim_data)
{
  struct blink_data *blink = anim_data;

  for (uint8_t i = 0; i < sizeof(blink_initial_on); i++)
    if (led == blink_initial_on[i]) {
      blink->initial_state = 1;
      return;
    }

  blink->initial_state = 0;
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

static uint16_t snowfall_pers_at(uint32_t time_ms, void *anim_data)
{
  struct snowfall_data *snowfall = anim_data;
  uint16_t idx = time_ms % 2000 / 200;
  return MAX_BRIGHTNESS * (snowfall->layer <= idx);
}

static const uint8_t slide_led_order[] = {
  3, 5, 4, 12, 2, 6, 18, 11, 1, 7, 17, 10, 0, 19, 15, 9, 14, 13, 16, 8
};

static void slide_init(uint8_t led, void *anim_data)
{
  struct slide_data *slide = anim_data;

  for (uint8_t i = 0; i < sizeof(slide_led_order); i++)
    if (led == slide_led_order[i]) {
      slide->position = i;
      return;
    }
}

static uint16_t slide_at(uint32_t time_ms, void *anim_data)
{
  struct slide_data *slide = anim_data;
  uint16_t idx = time_ms % 5000 / 200;
  return MAX_BRIGHTNESS * (slide->position <= idx);
}

static const anim_init_fn anim_init_fns[] = {
  [ANIM_PULSE]         = pulse_init,
  [ANIM_BREATHE]       = breathe_init,
  [ANIM_BLINK]         = blink_init,
  [ANIM_SNOWFALL]      = snowfall_init,
  [ANIM_SNOWFALL_PERS] = snowfall_init,
  [ANIM_SLIDE]         = slide_init,
};

static const anim_fn anim_fns[] = {
  [ANIM_PULSE]         = pulse_at,
  [ANIM_BREATHE]       = breathe_at,
  [ANIM_BLINK]         = blink_at,
  [ANIM_SNOWFALL]      = snowfall_at,
  [ANIM_SNOWFALL_PERS] = snowfall_pers_at,
  [ANIM_SLIDE]         = slide_at,
};

void animation_switch_to(uint8_t led, struct animation *anim, enum animations id) {
  anim_init_fns[id](led, &anim->data);
  anim->at = anim_fns[id];
}
