#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

#include "jingles.h"
#include "pitch.h"

#define S   4*Q
#define EE  E*4/3
#define E   2*Q
#define Q   8
#define H   Q/2
#define W   Q/4

extern TIM_HandleTypeDef htim2;

struct jingle {
  const uint16_t *melody;
  const uint8_t *durations;
  const uint16_t length;
};

static const struct jingle jingles[] = {
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

  [IMPERIAL_MARCH] = {
    .melody = (const uint16_t []){
      G3, G3, G3, E3, AS3, G3, E3, AS3, G3, D4, D4, D4, DS4,
      AS3, G3, E3, AS3, G3, G4, G3, G3, G4, FS4, F4, E4, DS4,
      E4, 0, GS3, CS4, C4, B3, B3, A3, B3, 0, E3, FS3, E3,
      G3, AS3, G3, AS3, D4, G4, G3, G3, G4, FS4, F4, E4, DS4,
      E4, 0, GS3, CS4, C4, B3, B3, A3, B3, 0, E3, FS3, E3,
      AS3, G3, E3, AS3, G3, 0,
    },
    .durations = (const uint8_t []){
      4, 4, 4, 6, 12, 4, 6, 12, 2, 4, 4, 4, 6, 12, 4, 6, 12,
      2, 4, 6, 12, 4, 6, 12, 16, 16, 8, 8, 8, 4, 6, 12, 16,
      16, 8, 8, 8, 4, 6, 12, 4, 6, 12, 2, 4, 6, 12, 4, 6, 12,
      16, 16, 8, 8, 8, 4, 6, 12, 16, 16, 8, 8, 8, 4, 6, 12,
      4, 6, 12, 2, 1,
    },
    .length = 71,
  },

  [SUPER_MARIO] = {
    .melody = (const uint16_t []){
      E4, E4, E4, C4, E4, G4, G3, C4, G3, E3, A3, B3, AS3, A3, G3,
      E4, G4, A4, F4, G4, E4, C4, D4, B3, C4, G3, E3, A3, B3, AS3,
      A3, G3, E4, G4, A4, F4, G4, E4, C4, D4, B3, G4, FS4, F4, DS4,
      E4, GS3, A3, C4, A3, C4, D4, G4, FS4, F4, DS4, E4, A4, A4,
      A4, G4, FS4, F4, DS4, E4, GS3, A3, C4, A3, C4, D4, DS4, D4, C4, 0
    },
    .durations = (const uint8_t []){
      E, Q, Q, E, Q, H, H, Q+E, Q+E, Q+E, Q, Q, E, Q, EE, EE, EE, Q, E,
      Q, Q, E, E, Q+E, Q+E, Q+E, Q+E, Q, Q, E, Q, EE, EE, EE, Q, E, Q, Q,
      E, E, Q+E, E, E, E, Q, Q, E, E, Q, E, E, Q+E, E, E, E, Q, Q, Q, E,
      H, E, E, E, Q, Q, E, E, Q, E, E, Q+E, Q+S, Q+S, H, 1
    },
    .length = 75,
  }
};

static const struct jingle *curr_jingle;
static uint16_t current_note = 0;
static uint32_t next_note_time = 0;
static bool next_is_silence = false;
static bool playing = false;

void jingle_start(enum jingle_id jingle)
{
  curr_jingle = &jingles[jingle];
  playing = true;
}

void jingle_stop(void)
{
  __HAL_TIM_SetAutoreload(&htim2, 0);

  playing = false;
  current_note = 0;
  next_note_time = 0;
  next_is_silence = false;
}

bool jingle_is_playing(void)
{
  return playing;
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
