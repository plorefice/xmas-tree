#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

#include "jingles.h"
#include "pitch.h"

extern TIM_HandleTypeDef htim2;

struct Jingle {
  const uint16_t *melody;
  const uint8_t *durations;
  const uint16_t length;
};

static const struct Jingle jingles[] = {
  [JINGLE_BELLS] = {
    .melody = (const uint16_t []){
      D3, B3, A3, G3, D3, D3, D3, D3, B3, A3, G3, E3, 0,
      E3, C4, B3, A3, FS3, 0, D4, D4, C4, A3, B3, 0, D3,
      B3, A3, G3, D3, D3, D3, D3, B3, A3, G3, E3, E3, E3,
      C4, B3, A3, D4, D4, D4, D4, E4, D4, C4, A3, G3, D4,

      B3, B3, B3, B3, B3, B3, B3, D4, G3, A3, B3, 0,
      C4, C4, C4, C4, C4, B3, B3, B3, B3, B3, A3, A3, B3, A3, D4,
      B3, B3, B3, B3, B3, B3, B3, D4, G3, A3, B3, 0,
      C4, C4, C4, C4, C4, B3, B3, B3, B3, D4, D4, C4, A3, G3, 0
    },
    .durations = (const uint8_t []){
      8, 8, 8, 8, 3, 16, 16, 8, 8, 8, 8, 3, 8,
      8, 8, 8, 8, 3, 8, 8, 8, 8, 8, 3, 8, 8,
      8, 8, 8, 3, 16, 16, 8, 8, 8, 8, 3, 8, 8,
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4,

      8, 8, 4, 8, 8, 4, 8, 8, 6, 16, 3, 8,
      8, 8, 6, 16, 8, 8, 8, 16, 16, 8, 8, 8, 8, 4, 4,
      8, 8, 4, 8, 8, 4, 8, 8, 6, 16, 3, 8,
      8, 8, 6, 16, 8, 8, 8, 16, 16, 8, 8, 8, 8, 2, 1
    },
    .length = 106,
  },
};

static const struct Jingle *curr_jingle;
static uint16_t current_note = 0;
static uint32_t next_note_time = 0;
static bool next_is_silence = false;
static bool playing = false;

void jingle_start(enum JingleId jingle)
{
  curr_jingle = &jingles[jingle];
  playing = true;
}

void jingle_stop(void)
{
  playing = false;
  current_note = 0;
  next_note_time = 0;
  next_is_silence = false;
}

void jingle_update(void)
{
  uint32_t now = HAL_GetTick();

  if (!playing || next_note_time > now)
    return;

  uint16_t dur = 2000 / curr_jingle->durations[current_note];
  uint32_t freq = 500000 / curr_jingle->melody[current_note];

  if (next_is_silence) {
    __HAL_TIM_SetAutoreload(&htim2, 0);
    next_note_time = now + (dur / 10);
  } else {
    __HAL_TIM_SetAutoreload(&htim2, freq);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, freq >> 1);
    current_note = (current_note + 1) % curr_jingle->length;
    next_note_time = now + dur;
  }

  next_is_silence = !next_is_silence;
}
