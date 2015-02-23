/*
 * Copyright (c) 2014, TU Braunschweig
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/* Work around GCC 4.8.1/4.7.2 optimizer bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58545 */
#if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 8) && (__GNUC_PATCHLEVEL__ <= 3)) || ((__GNUC__ == 4) && (__GNUC_MINOR__ == 7) && (__GNUC_PATCHLEVEL__ == 2))
#define FIX_OPTIMZERBUG __attribute__((optimize(2)))
#else
#define FIX_OPTIMZERBUG
#endif

/**
 * \file Additional functions for the extended features of RF231
 * \author Sebastian Willenborg <s.willenborg@tu-bs.de>
 */
#include "rf231.h"

uint8_t aes_buffer[18];

typedef struct {
  uint8_t last_iv[AES_128_BLOCK_SIZE];
  uint8_t mode;
  uint8_t first;
} aes_setting;

static aes_setting aes;

static inline FIX_OPTIMZERBUG void
xor16(uint8_t *inout, uint8_t *xor_key)
{
  uint8_t i;
  for(i = 0; i < 16; i++) {
    inout[i] ^= xor_key[i];
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Waits until the aes-component completed its aes-operation
 *        Will terminate itself, if the status dosn't change after 5 attempts
 */
void
rf231_aes_128_wait_until_is_ready()
{
  uint8_t status = 0;
  uint8_t timeout = 0;
  while(!status && timeout < 5) {
    _delay_us(20);
    timeout++;
    hal_sram_read(ADDR_AES_STATUS, 1, &status);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief              Sets the aes-key for encyption or decryption
 * \param key          Pointer to the aes-key
 * \param decryption   Is 0 to setup encryption, 1 to setup decryption
 * \param aes_mode     AES Mode to select ECB or CBC
 * \param iv           ECB needs an IV, this is the pointer to the IV
 */
void
rf231_aes_128_set_key(uint8_t *key, uint8_t decryption, uint8_t aes_mode, uint8_t *iv)
{
  aes.mode = aes_mode;
  aes.first = 1;
  switch(aes.mode) {
  case AES_128_MODE_CBC:
    if(iv) {
      memcpy(aes.last_iv, iv, AES_128_BLOCK_SIZE);
    }
    break;
  case AES_128_MODE_ECB:
  default:
    break;
  }

  /* Set aes-key on RF231 */
  aes_buffer[0] = RF231_AES_MODE_KEY;
  memcpy(aes_buffer + 1, key, AES_128_KEY_SIZE);
  hal_sram_write(ADDR_AES_CTRL, 1 + AES_128_KEY_SIZE, aes_buffer);

  if(decryption) {
    /* Perform dummy encryption to generate the "decryption" key */
    aes_buffer[0] = RF231_AES_MODE_ECB;
    memset(aes_buffer + 1, 0, 16);
    aes_buffer[17] = RF231_AES_MODE_ECB | AES_REQUEST_WRITE;
    hal_sram_write(ADDR_AES_CTRL, 1 + AES_128_BLOCK_SIZE + 1, aes_buffer);

    rf231_aes_128_wait_until_is_ready();

    /* Request the decryption key */
    aes_buffer[0] = RF231_AES_MODE_KEY;
    hal_sram_write(ADDR_AES_CTRL, 1, aes_buffer);

    /* Read the decryption key */
    hal_sram_read(ADDR_AES_KEY, AES_128_KEY_SIZE, aes_buffer + 1);

    /* Set the decryption key on RF231 */
    aes_buffer[0] = RF231_AES_MODE_KEY;
    hal_sram_write(ADDR_AES_CTRL, 1 + AES_128_KEY_SIZE, aes_buffer);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief                        Performs the encryption on the given block
 * \param plaintext_and_result   Pointer to an buffer, which will be replaced
 *                               with its encrypted content (16 Bytes)
 */
void
rf231_aes_128_encrypt(uint8_t *plaintext_and_result)
{
  uint8_t aes_mode = RF231_AES_MODE_ECB;
  if(aes.first) {
    aes.first = 0;
    if(aes.mode == AES_128_MODE_CBC) {
      xor16(plaintext_and_result, aes.last_iv);
    }
  } else {
    switch(aes.mode) {
    case AES_128_MODE_ECB:
      aes_mode = RF231_AES_MODE_ECB;
      break;
    case AES_128_MODE_CBC:
      aes_mode = RF231_AES_MODE_CBC;
      break;
    }
  }

  aes_buffer[0] = AES_DIRECTION_ENCRYPTION | aes_mode;
  memcpy(aes_buffer + 1, plaintext_and_result, AES_128_BLOCK_SIZE);
  aes_buffer[1 + AES_128_BLOCK_SIZE] = AES_DIRECTION_ENCRYPTION | aes_mode | AES_REQUEST_WRITE;
  hal_sram_write(ADDR_AES_CTRL, 1 + AES_128_BLOCK_SIZE + 1, aes_buffer);

  rf231_aes_128_wait_until_is_ready();

  hal_sram_read(ADDR_AES_CTRL, 1 + AES_128_BLOCK_SIZE, aes_buffer);
  memcpy(plaintext_and_result, aes_buffer + 1, AES_128_BLOCK_SIZE);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief                        Performs the decryption on the given block
 * \param plaintext_and_result   Pointer to an buffer, which will be replaced
 *                               with its decrypted content (16 Bytes)
 */
void
rf231_aes_128_decrypt(uint8_t *ciphertext_and_result)
{
  aes_buffer[0] = AES_DIRECTION_DECRYPTION | RF231_AES_MODE_ECB;
  memcpy(aes_buffer + 1, ciphertext_and_result, AES_128_BLOCK_SIZE);
  aes_buffer[1 + AES_128_BLOCK_SIZE] = AES_DIRECTION_DECRYPTION | AES_128_MODE_ECB | AES_REQUEST_WRITE;
  hal_sram_write(ADDR_AES_CTRL, 1 + AES_128_BLOCK_SIZE + 1, aes_buffer);

  rf231_aes_128_wait_until_is_ready();

  hal_sram_read(ADDR_AES_CTRL, AES_128_BLOCK_SIZE + 1, aes_buffer);
  if(aes.mode == AES_128_MODE_CBC) {
    xor16(aes_buffer + 1, aes.last_iv);
    memcpy(aes.last_iv, ciphertext_and_result, AES_128_BLOCK_SIZE);
  }
  memcpy(ciphertext_and_result, aes_buffer + 1, AES_128_BLOCK_SIZE);
}
/*---------------------------------------------------------------------------*/
static inline uint8_t
get_rand_bits()
{
  return hal_subregister_read(SR_RND_VALUE);
}
/*---------------------------------------------------------------------------*/
uint8_t
rf231_random()
{
  uint8_t result = 0;
  result |= get_rand_bits() << 6;
  result |= get_rand_bits() << 4;
  result |= get_rand_bits() << 2;
  result |= get_rand_bits();

  return result;
}
