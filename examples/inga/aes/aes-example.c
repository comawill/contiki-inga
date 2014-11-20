#include "contiki.h"
#include <stdlib.h>
#include <stdio.h> /* For printf() */
#include "clock.h"
#include <util/delay.h>
#include <string.h>
#include "dev/rf23x/rf231/rf231.h"

uint8_t key[AES_128_KEY_SIZE] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
uint8_t iv[AES_128_KEY_SIZE] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
uint8_t buffer[AES_128_BLOCK_SIZE];
uint8_t buffer2[AES_128_BLOCK_SIZE];

/*---------------------------------------------------------------------------*/
void
show_block(uint8_t *data)
{
  uint8_t i;
  for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
    printf("%02x", data[i]);
  }
  printf("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS(test_process, "Test process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();
  printf("AES ECB");
  printf("Set AES-Key: ");
  show_block(key);
  rf231_aes_128_set_key(key, 0, AES_128_MODE_ECB, 0);

  memset(buffer, 0x12, AES_128_BLOCK_SIZE);
  printf("Encrypt 1: ");
  show_block(buffer);
  rf231_aes_128_encrypt(buffer);
  printf("Result 1: ");
  show_block(buffer);
  memcpy(buffer2, buffer, AES_128_BLOCK_SIZE);

  memset(buffer, 0x12, AES_128_BLOCK_SIZE);
  printf("Encrypt 2: ");
  show_block(buffer);
  rf231_aes_128_encrypt(buffer);
  printf("Result 2: ");
  show_block(buffer);

  printf("Setup Decryption\n");
  rf231_aes_128_set_key(key, 1, AES_128_MODE_ECB, 0);

  printf("Decrypt: ");
  show_block(buffer2);
  rf231_aes_128_decrypt(buffer2);
  printf("Result: ");
  show_block(buffer2);

  printf("Decrypt: ");
  show_block(buffer);
  rf231_aes_128_decrypt(buffer);
  printf("Result: ");
  show_block(buffer);

  printf("AES CBC");
  printf("Set AES-Key: ");
  show_block(key);
  rf231_aes_128_set_key(key, 0, AES_128_MODE_CBC, iv);
  memset(buffer, 0x12, AES_128_BLOCK_SIZE);
  printf("Encrypt: ");
  show_block(buffer);
  rf231_aes_128_encrypt(buffer);
  printf("Result: ");
  show_block(buffer);
  memcpy(buffer2, buffer, AES_128_BLOCK_SIZE);

  memset(buffer, 0x12, AES_128_BLOCK_SIZE);
  printf("Encrypt: ");
  show_block(buffer);
  rf231_aes_128_encrypt(buffer);
  printf("Result: ");
  show_block(buffer);

  printf("Setup Decryption\n");
  rf231_aes_128_set_key(key, 1, AES_128_MODE_CBC, iv);

  printf("Decrypt: ");
  show_block(buffer2);
  rf231_aes_128_decrypt(buffer2);
  printf("Result: ");
  show_block(buffer2);

  printf("Decrypt: ");
  show_block(buffer);
  rf231_aes_128_decrypt(buffer);
  printf("Result: ");
  show_block(buffer);

  printf("Random: \n");
  uint8_t ix = 0;
  uint32_t stats[4];
  for(ix = 0; ix < 4; ix++) {
    stats[ix] = 0;
  }
  for(ix = 0; ix < 100; ix++) {
    uint8_t x = rf231_random();
    stats[x & 0x03]++;
    stats[(x >> 2) & 0x03]++;
    stats[(x >> 4) & 0x03]++;
    stats[(x >> 6) & 0x03]++;
  }
  uint32_t s = stats[0] + stats[1] + stats[2] + stats[3];
  printf("%02x: %lu %lu\n", 0, stats[0], stats[0] * 100 / s);
  printf("%02x: %lu %lu\n", 1, stats[1], stats[1] * 100 / s);
  printf("%02x: %lu %lu\n", 2, stats[2], stats[2] * 100 / s);
  printf("%02x: %lu %lu\n", 3, stats[3], stats[3] * 100 / s);

  PROCESS_END();
}

AUTOSTART_PROCESSES(&test_process);
