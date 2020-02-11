/* niftest.c */
#include <erl_nif.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __APPLE__
    #include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #include "osx_uchar_fixes.h"
    #endif
#else
    #include <uchar.h>
#endif

#include "items.h"
#include "parse.h"
#include "byteswap.h"

#define LOG_DEBUG   1
#define LOG_INFO    2
#define LOG_WARN    3
#define LOG_ERR     4

#define XML_EMPTY   0
#define XML_END     1
#define XML_OTAG    2
#define XML_CTAG    3
#define XML_DATA    4

#define XML_ERROR   255


#define LOGFILE_PATH "log.txt"

#include "chars.h"

ERL_NIF_TERM char32_string_to_charlist(ErlNifEnv *env, const char32_t *str, unsigned int len)
{
    ERL_NIF_TERM *termp, result;
    termp = enif_alloc(sizeof(ERL_NIF_TERM) * len);

    for (unsigned int i = 0; i < len; i++) {
        termp[i] = enif_make_uint(env, (unsigned int)(str[i]));
    }

    result = enif_make_list_from_array(env, termp, len);
    enif_free(termp);
    return result;
}

ERL_NIF_TERM char32_to_binary_string(ErlNifEnv *env, const char32_t *str, unsigned int len)
{
  #define MAGIC_ERLANG_TERM   0x83
  #define MAGIC_ERLANG_STRING 0x6d
  #define BINARY_TERM_HEADER_SIZE 0x6

  size_t out_size;
  char *out_data, *out_buffer;
  ERL_NIF_TERM result;

  out_size = (size_t)utf8_char32_string_to_multibyte(&out_data, str, len);

  out_buffer = (char *)malloc(sizeof(char) * (out_size + BINARY_TERM_HEADER_SIZE));
  out_buffer[0] = MAGIC_ERLANG_TERM;
  out_buffer[1] = MAGIC_ERLANG_STRING;

  uint32_t __size_buffer[1];
  __size_buffer[0] = bswap_32((uint32_t)out_size);

  memcpy(&out_buffer[2], __size_buffer, sizeof(char) * 4);
  memcpy(&out_buffer[6], out_data, sizeof(char) * (out_size));

  const unsigned char *immutable_out_buffer = (unsigned char *)out_buffer;

  if (!enif_binary_to_term(env, immutable_out_buffer, out_size + BINARY_TERM_HEADER_SIZE, &result, 0)) {
      return enif_make_badarg(env);
  }

  free(out_buffer);
  return result;
}

ERL_NIF_TERM make_property(ErlNifEnv *env, const char32_t *property, const char32_t *value)
{
        ERL_NIF_TERM property_arr[3];

        property_arr[0] = enif_make_atom(env, "property");
        property_arr[1] = enif_make_string(env, "someprop", ERL_NIF_LATIN1);
        property_arr[2] = enif_make_string(env, "somevalue", ERL_NIF_LATIN1);

        return enif_make_tuple_from_array(env, property_arr, 3);
}

ERL_NIF_TERM xml_item_to_otag(ErlNifEnv *env, const xml_item_t *item)
{
    if (!item || item->type != XML_OTAG) {
        return enif_make_badarg(env);
    }

    ERL_NIF_TERM otag_arr[3];

    otag_arr[0] = enif_make_atom(env, "otag");
    otag_arr[1] = char32_to_binary_string(env, item->data, strlen32(item->data));
    otag_arr[2] = enif_make_list(env, 0);

    return enif_make_tuple_from_array(env, otag_arr, 3);
}

ERL_NIF_TERM xml_item_to_ctag(ErlNifEnv *env, const xml_item_t *item)
{
    if (!item || item->type != XML_CTAG) {
        return enif_make_badarg(env);
    }

    ERL_NIF_TERM ctag_arr[2];

    ctag_arr[0] = enif_make_atom(env, "ctag");
    ctag_arr[1] = char32_to_binary_string(env, item->data, strlen32(item->data));

    return enif_make_tuple_from_array(env, ctag_arr, 2);
}

ERL_NIF_TERM xml_item_to_data(ErlNifEnv *env, const xml_item_t *item)
{
    if (!item || item->type != XML_DATA) {
        return enif_make_badarg(env);
    }

    ERL_NIF_TERM data_arr[2];

    data_arr[0] = enif_make_atom(env, "data");
    data_arr[1] = char32_to_binary_string(env, item->data, strlen32(item->data));

    return enif_make_tuple_from_array(env, data_arr, 2);
}

ERL_NIF_TERM xml_item_to_tuple(ErlNifEnv *env, const xml_item_t *item)
{
    switch (item->type)
    {
        case XML_OTAG:
            return xml_item_to_otag(env, item);
        case XML_CTAG:
            return xml_item_to_ctag(env, item);
        case XML_DATA:
            return xml_item_to_data(env, item);
        default:
            return enif_make_badarg(env);
    }
}

ERL_NIF_TERM xml_chain_to_list(ErlNifEnv *env, const xml_chain_t *chain)
{
    if (!chain || !chain->data) {
        printf("incorrect data or chain\n");
        return enif_make_badarg(env);
    }

    const xml_chain_t *ptr = chain;
    int chain_len = xml_chain_len(ptr);
    ERL_NIF_TERM *termp, result;

    termp = enif_alloc(sizeof(ERL_NIF_TERM) * chain_len);

    for (unsigned int i = 0; i < chain_len; i++) {
        termp[i] = xml_item_to_tuple(env, ptr->data);
        ptr = ptr->next;
    }

    result = enif_make_list_from_array(env, termp, chain_len);
    enif_free(termp);
    return result;
}

int cleanup_string(char32_t **in, unsigned int *len)
{
    if (!in || !*in || !len || *len == 0) {
        return -1;
    }

    int out_len = 0;
    unsigned int i, buffer_pos = 0;
    char32_t *data = *in;
    char32_t *buffer = char32_alloc(*len);
    char32_t *out, *sym;

    for (i = 0, sym = data; i < *len; sym++, i++) {
        char32_t current = *sym;
        char32_t next;
        unsigned remains = *len - i - 1;

        if (remains) {
            next = data[i + 1];
        }

        if (current == '\r' || current == '\t' || current == '\n') {
            continue;
        }

        if (remains && current == ' ' && next == ' ') {
            continue;
        }

        buffer[buffer_pos] = current;
        out_len++;
        buffer_pos++;
    }

    out = char32_alloc(out_len);
    strncpy32(out, buffer, out_len);
    free(buffer);
    free(*in);
    *in = out;
    *len = out_len;

    return out_len;
}

static ERL_NIF_TERM hello(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary binary;
    if (!enif_term_to_binary(env, argv[0], &binary)) {
        return enif_make_badarg(env);
    }

    /* ===== < OUTPUT SECTION > ===== */
    char *out_data, *out_buffer;
    size_t out_size = 0;

    /* ===== < END SECTION > ===== */

    uint8_t *ptr = (uint8_t *)(binary.data);
    uint32_t len = bswap_32(*(uint32_t *)(ptr + 2));
    unsigned int utf8_str_len = 0;

    char32_t *buffer;

    /*
    printf("sizeof(char32_t) = %zu\nbinary.size = %zu\ntype = %u\nterm_type = %u\nlen = %u\n\n",
      sizeof(char32_t),
      binary.size,
      ptr[0],
      ptr[1],
      len
    );
    for (unsigned i = 0; i < binary.size; i++) {
        printf("%u ", ptr[i]);
    }

    printf("\n");
    */

    /* encoded data starts here */
    const char *data_ptr = (char *)(ptr + 6); 
    utf8_str_len = utf8_multibyte_char_to_char32(&buffer, data_ptr, len);
    cleanup_string(&buffer, &utf8_str_len);

    ERL_NIF_TERM result;
    xml_chain_t* chain = parse(buffer, utf8_str_len);
    inspect_xml_chain(chain);

    result = xml_chain_to_list(env, chain);

    free_chain(chain);
    free(buffer);
    enif_release_binary(&binary);

    return result;
}

static ERL_NIF_TERM hello__(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    unsigned int in_len, i;
    char32_t *buffer;
    ERL_NIF_TERM head, tail, result;

    if (!enif_get_list_length(env, argv[0], &in_len)) {
        return enif_make_badarg(env);
    }

    buffer = char32_alloc(in_len);

    if (!enif_get_list_cell(env, argv[0], &head, &tail)) {
        return enif_make_badarg(env);
    }

    for (i = 0; i < in_len; i++) {
        unsigned int u;
        if (!enif_get_uint(env, head, &u)) {
            break;
        }
        buffer[i] = (char32_t)u;

        if (!enif_get_list_cell(env, tail, &head, &tail)) {
            break;
        }
    }

    cleanup_string(&buffer, &in_len);

    xml_chain_t* chain = parse(buffer, in_len);

    //result = xml_chain_to_list(env, chain);
    result = char32_string_to_charlist(env, buffer, in_len);
    free(buffer);

    inspect_xml_chain(chain);
    free_chain(chain);

    return result;
}

static ErlNifFunc nif_funcs[] =
{
    {"hello", 1, hello}
};

ERL_NIF_INIT(Elixir.Xmlazy.Niftest,nif_funcs,NULL,NULL,NULL,NULL)
