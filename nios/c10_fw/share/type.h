#pragma once

typedef float          float32_t;
typedef double         float64_t;

typedef uint8_t        BOOLEAN;

typedef struct _wav_t {
   uint32_t      chunk_iD;
   uint32_t      chunk_size;
   uint32_t      format;
   uint32_t      chunk1_id;
   uint32_t      chunk1_size;
   uint16_t      audio_fmt;
   uint16_t      num_chan;
   uint32_t      samp_rate;
   uint32_t      byte_rate;
   uint16_t      block_align;
   uint16_t      bits_per_samp;
   uint32_t      chunk2_id;
   uint32_t      chunk2_size;
} wav_t, *pwav_t;

