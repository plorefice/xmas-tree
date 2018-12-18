#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

#include "jingles.h"
#include "pitch.h"

// Note duration: N / bpm
#define W    240
#define H    120
#define Q    60
#define E    30
#define S    15
#define DH   180
#define DQ   90
#define DE   45
#define DS   22
#define TQ   40
#define TE   15
#define TS   10

extern TIM_HandleTypeDef htim2;

struct jingle {
  const uint16_t *melody;
  const uint8_t *durations;
  const uint16_t length;
  const uint16_t bpm;
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
      C4, C4, C4, C4, C4, B3, B3, B3, B3, D4, D4, C4, A3, G3, 0, 0,
    },
    .durations = (const uint8_t []){
      Q, Q, Q, Q, DH, E, E, Q, Q, Q, Q, DH, Q,
      Q, Q, Q, Q, DH, Q, Q, Q, Q, Q, DH, Q, Q,
      Q, Q, Q, DH, E, E, Q, Q, Q, Q, DH, Q, Q,
      Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H, H,

      Q, Q, H, Q, Q, H, Q, Q, DQ, E, DH, Q,
      Q, Q, DQ, E, Q, Q, Q, E, E, Q, Q, Q, Q, H, H,
      Q, Q, H, Q, Q, H, Q, Q, DQ, E, DH, Q,
      Q, Q, DQ, E, Q, Q, Q, E, E, Q, Q, Q, Q, W, W, W,
    },
    .length = 107,
    .bpm = 240,
  },

  [IMPERIAL_MARCH] = {
    .melody = (const uint16_t []){
      G3, G3, G3, E3, AS3, G3, E3, AS3, G3, D4, D4, D4, DS4, AS3,
      FS3, DS3, AS3, G3, G4, G3, G3, G4, FS4, F4, E4, DS4, E4, 0,
      GS3, CS4, C4, B3, B3, A3, B3, 0, E3, FS3, E3, G3, AS3, G3,
      AS3, D4, G4, G3, G3, G4, FS4, F4, E4, DS4, E4, 0, GS3, CS4,
      C4, B3, B3, A3, B3, 0, E3, FS3, E3, AS3, G3, E3, AS3, G3, 0, 0,
    },
    .durations = (const uint8_t []){
      Q, Q, Q, DE, TE, Q, DE, TE, H, Q, Q, Q, DE, TE, Q, DE, TE, H,
      Q, DE, TE, Q, DE, TE, S, S, E, E, E, Q, DE, TE, S, S, E, E, E,
      Q, DE, TE, Q, DE, TE, H, Q, DE, TE, Q, DE, TE, S, S, E, E, E,
      Q, DE, TE, S, S, E, E, E, Q, DE, TE, Q, DE, TE, H, W, W,
    },
    .length = 72,
    .bpm = 105,
  },

  [SUPER_MARIO] = {
    .melody = (const uint16_t []){
      E4, E4, 0, E4, 0, C4, E4, 0, G4, 0, 0, F3, 0, 0,

      C4, 0, 0, F3, 0, E3, 0, 0, A3, 0, B3, 0, AS3, A3, 0, G3, E4,
      G4, A4, 0, F4, G4, 0, E4, 0, C4, D4, B3, 0,

      C4, 0, 0, F3, 0, E3, 0, 0, A3, 0, B3, 0, AS3, A3, 0, G3, E4,
      G4, A4, 0, F4, G4, 0, E4, 0, C4, D4, B3, 0,

      0, G4, FS4, F4, DS4, 0, E4, 0, GS3, A3, C4, 0, A3, C4, D4,
      0, G4, FS4, F4, DS4, 0, E4, 0, C5, 0, C5, C5, 0, 0,

      0, G4, FS4, F4, DS4, 0, E4, 0, GS3, A3, C4, 0, A3, C4, D4,
      0, DS4, 0, 0, D4, 0, C4, 0, 0, 0,

      0, G4, FS4, F4, DS4, 0, E4, 0, GS3, A3, C4, 0, A3, C4, D4,
      0, G4, FS4, F4, DS4, 0, E4, 0, C5, 0, C5, C5, 0, 0,

      0, G4, FS4, F4, DS4, 0, E4, 0, GS3, A3, C4, 0, A3, C4, D4,
      0, DS4, 0, 0, D4, 0, C4, 0, 0, 0,

      C4, C4, 0, C4, 0, C4, D4, 0, E4, C4, 0, A3, G3, 0, 0,
      C4, C4, 0, C4, 0, C4, D4, E4, 0, 0,

      C4, C4, 0, C4, 0, C4, D4, 0, E4, C4, 0, A3, G3, 0, 0,
      E4, E4, 0, E4, 0, C4, E4, 0, G4, 0, 0, F3, 0, 0,
    },
    .durations = (const uint8_t []){
      Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H, Q, Q, H,

      Q, Q, Q, Q, H, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q,
      Q + 20, Q + 20, Q + 20, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H,

      Q, Q, Q, Q, H, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q,
      Q + 20, Q + 20, Q + 20, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H,

      H, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q,
      H, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H,

      H, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q,
      H, Q, Q, Q, Q, H, Q, Q, H, W,

      H, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q,
      H, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H,

      H, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q,
      H, Q, Q, Q, Q, H, Q, Q, H, W,

      Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H,
      Q, Q, Q, Q, Q, Q, Q, Q, W, W,

      Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H,
      Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H, Q, Q, H,
    },
    .length = 234,
    .bpm = 390,
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

  uint16_t dur = 1000 * curr_jingle->durations[current_note] / curr_jingle->bpm;
  uint32_t freq = 500000 / curr_jingle->melody[current_note];

  if (next_is_silence) {
    __HAL_TIM_SetAutoreload(&htim2, 0);
    next_note_time = now + (dur / 20);
  } else {
    __HAL_TIM_SetAutoreload(&htim2, freq);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, freq >> 1);
    current_note = (current_note + 1) % curr_jingle->length;
    next_note_time = now + dur;
  }

  next_is_silence = !next_is_silence;
}
