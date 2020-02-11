#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <string.h>

#include "chars.h"

/* Func protos */
unsigned int __bit_size(char32_t);
uint32_t __create_mask(uint8_t, uint8_t);


char32_t *char32_alloc(unsigned len) {
    char32_t *str = (char32_t *)malloc(sizeof(char32_t) * (len + 1));
    memset(str, '\0', sizeof(char32_t) * (len + 1));
    return str;
}

int strlen32(const char32_t* strarg)
{
  if (!strarg) {
     return -1; //strarg is NULL pointer
  }
  const char32_t* str = strarg;
  for(;*str;++str) {
    ; // empty body
  }
  return str - strarg;
}

void *strncpy32(char32_t *destptr, const char32_t *srcptr, size_t symbols)
{
  return memcpy((void *)destptr, (void *)srcptr, symbols * sizeof(char32_t));
}

inline int __bytes_encoded(char byte)
{
  if (!(((uint8_t)byte & 0xf0) - 0xf0)) {
    return 4;
  }
  if (!(((uint8_t)byte & 0xe0) - 0xe0)) {
    return 3;
  }
  if (!(((uint8_t)byte & 0xc0) - 0xc0)) {
    return 2;
  }
  if (!((uint8_t)byte & 0x80)) {
    return 1;
  }
  printf("\n>>>>> %u\n", (uint8_t)byte);
  return -1;
}

int __utf8_decode_leading(char byte) {
  uint8_t ubyte = (uint8_t)byte;

  if (!(ubyte & 0x80)) {
    return ubyte;
  }

  uint8_t mask = (uint8_t)__create_mask(6, 8); // for first 3 bits
  //printf("\n mask = %u\n, ubyte = %u, res = %u\n", mask, ubyte, ubyte & (~mask));

  if ((ubyte & mask) == 0xc0) {
    return ubyte & (~mask);
  }

  mask |= 1 << 4; // first 4 bits
  if ((ubyte & mask) == 0xe0) {
    return byte & (~mask);
  }

  mask |= 1 << 3; // first 5 bits
  if ((ubyte & mask) == 0xf0) {
    return byte & (~mask);
  }

  return -1;
}

int __utf8_decode_tail(char byte) {
  if ( (((uint8_t)byte) & 0x80) && !(((uint8_t)byte) & 0x40) ) {
    return byte ^ 0x80;
  }

  return -1;
}

char32_t __utf8_bytes_to_symbol_code(const char *bytes)
{
  uint32_t result = 0x000;
  int buf = 0;

  int bytes_to_decode = __bytes_encoded(bytes[0]);
  if (bytes_to_decode == -1) {
    return (char32_t)0;
  }

  buf = __utf8_decode_leading(bytes[0]);
  if (buf == -1) {
    return (char32_t)0;
  }
  result = (uint8_t)buf;

  if (bytes_to_decode > 1) {
    for (int i = 1; i < bytes_to_decode; i++) {
      buf = __utf8_decode_tail(bytes[i]);
      if (buf == -1) {
        return (char32_t)0;
      }
      buf = (uint8_t)buf;

      result = (result << 6) | buf;
    }
  }
  return result;
}

int utf8_multibyte_char_to_char32(char32_t **output, const char *input, size_t len)
{
  if (!input) {
    return -1;
  }

  char32_t *result, *result_buffer = char32_alloc(len);

  int out_len = 0, processed = 0;

  const char *ptr = input;

  while (processed < len) {
    if (!ptr) {
      break;
    }

    int encoded = __bytes_encoded(ptr[0]);

    if (encoded == -1) {
      //printf("\nEncoded == -1\n");
      return -1;
    }

    char32_t buf = __utf8_bytes_to_symbol_code(ptr);
    result_buffer[out_len++] = buf;

    //printf(" > %u; size: %u\n", (uint32_t)buf, __bit_size(buf));

    ptr += encoded;
    processed += encoded;
  }

  result = char32_alloc(out_len);
  strncpy32(result, result_buffer, out_len);
  *output = result;

  free(result_buffer);
  return out_len;
}

uint32_t __create_mask(uint8_t from, uint8_t to)
{
  from--;
  uint32_t register r = 0;
  for (uint8_t i = from; i < to; i++) {
    r |= 1 << i;
  }
  return r;
}

int __utf8_encode_symbol(char **output, char32_t symbol)
{
  uint32_t usym = (uint32_t)symbol;
  unsigned int sym_bitsize = __bit_size(symbol);

  if (sym_bitsize < 8) {
    *output[0] = (char)symbol;
    return 1;
  }

  uint32_t mask_1 = __create_mask(1, 6);
  uint32_t mask_2 = __create_mask(7, 11);

  if (sym_bitsize < 12) {
    uint8_t first = (uint8_t)((usym & mask_2) >> 6);
    uint8_t sec = (uint8_t)(usym & mask_1);

    char buf[2];

    buf[0] = first | 0xc0;
    buf[1] = sec | 0x80;

    memcpy(*output, buf, sizeof(char) * 2);
    return 2;
  }

  mask_2 = __create_mask(7, 12);
  uint32_t mask_3 = __create_mask(13, 16);

  if (sym_bitsize < 17) {
    uint8_t first = (uint8_t)((usym & mask_3) >> 12);
    uint8_t sec = (uint8_t)((usym & mask_2) >> 6);
    uint8_t third = (uint8_t)(usym & mask_1);

    char buf[3];

    buf[0] = first | 0xe0;
    buf[1] = sec | 0x80;
    buf[2] = third | 0x80;

    memcpy(*output, buf, sizeof(char) * 3);
    return 3;
  }

  mask_3 = __create_mask(13, 18);
  uint32_t mask_4 = __create_mask(19, 21);

  if (sym_bitsize < 22) {
    uint8_t first = (uint8_t)((usym & mask_4) >> 18);
    uint8_t sec = (uint8_t)((usym & mask_3) >> 12);
    uint8_t third = (uint8_t)((usym & mask_2) >> 6);
    uint8_t forth = (uint8_t)(usym & mask_1);

    char buf[4];

    buf[0] = first | 0xf0;
    buf[1] = sec | 0x80;
    buf[2] = third | 0x80;
    buf[3] = forth | 0x80;

    memcpy(*output, buf, sizeof(char) * 4);
    return 4;
  }
  return -1;
}

unsigned int __bit_size(char32_t symbol)
{
  uint32_t v = (uint32_t)symbol;

  unsigned int r = 0; // r will be lg(v)

  while (v >>= 1) // unroll for more speed...
  {
    r++;
  }

  return (!r) ? r : ++r;
}

int utf8_char32_string_to_multibyte(char **output, const char32_t *input, size_t len)
{
  if (!input) {
    return -1;
  }

  const char32_t *ptr = input;
  unsigned register i = 0;
  unsigned out_len = 0;
  char *out_buffer = (char *)malloc(sizeof(char) * (len * 4 + 1)), *out_buffer_ptr;

  out_buffer_ptr = out_buffer;

  for (i = 0; i < len; i++) {
    int encoded_with = __utf8_encode_symbol(&out_buffer_ptr, ptr[0]); //bytes

    if (encoded_with == -1) {
      out_buffer_ptr[0] = '0';
      encoded_with = 1;
    }

    //printf("step: %d; encoded_with: %d\n", i, encoded_with);
    out_len += encoded_with;
    out_buffer_ptr += encoded_with;
    ptr++;
  }

  //printf("\nEncoded! in len = %zu || out len = %u\n\n", len, out_len);

  uint8_t *print_ptr = (uint8_t *)out_buffer;
  //for (i = 0; i < out_len; i++) {
  //    printf("%u ", print_ptr[i]);
  //}
  //printf("\n==================\n\n");

  if (output) {
    *output = (char *)malloc(sizeof(char) * out_len);
    memcpy(*output, out_buffer, sizeof(char) * out_len);
  }
  free(out_buffer);
  return out_len;
}
