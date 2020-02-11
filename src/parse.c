#include <stdlib.h>
#include <stdio.h>

#include "items.h"
#include "parse.h"
#include "chars.h"


xml_chain_t *parse(const char32_t *input, unsigned int len) {
    int res = -1;

    xml_chain_t *chain = create_empty_chain();
    char32_t *buffer_str = char32_alloc(len);
    strncpy32(buffer_str, input, len);

    unsigned int buffer_len = len;

    while (res != XML_END && res != XML_ERROR)
    {
        xml_item_t *item;

        res = get_next(&buffer_str, &buffer_len, &item);
        switch(res)
        {
            case XML_END:
                break;
            case XML_EMPTY:
                break;
            case XML_DATA:
                if (item) {
                    inspect_xml_item(item);
                    add_item_to_chain(&chain, item);
                }
                break;
            case XML_OTAG:
                if (item) {
                    inspect_xml_item(item);
                    add_item_to_chain(&chain, item);
                }
                break;
            case XML_CTAG:
                if (item) {
                    inspect_xml_item(item);
                    add_item_to_chain(&chain, item);
                }
                break;
            default:
              res = XML_ERROR;
              break;
        }
        //break;
    }

    return chain;
}

int get_next(char32_t **input, unsigned int *len, xml_item_t **output)
{
    #define STATE_UNDEFINED         0
    #define STATE_XML_TAG           1
    #define STATE_OTAG_OPEN         2
    #define STATE_DATA              3
    #define STATE_CTAG_OPEN         4
    #define STATE_OTAG_PROPERTIES   5

    if (!(*len)) {
        return XML_END;
    }
    if (!input || !(*input) || !len) {
        printf("Inbound pointer error.\n");
        return -1;
    }

    char32_t *sym, *data, *tag_name;
    unsigned int i, state, prop_start_l;

    data = *input;
    state = STATE_UNDEFINED;

    for (i = 0, sym = data; i < *len; sym++, i++) {
        char32_t current = *sym;
        char32_t next;
        unsigned remains = *len - i - 1;

        if (remains) {
            next = data[i + 1];
        }

        switch (state) {
            case STATE_UNDEFINED:

                if (current == '<' && next == '?') {
                    state = STATE_XML_TAG;
                    continue;
                }
                if (current == '<' && next == '/') {
                    state = STATE_CTAG_OPEN;
                    continue;
                }
                if (current == '<') {
                    state = STATE_OTAG_OPEN;
                    continue;
                }
                xml_item_t *data_item = get_next_data(input, len);
                if (!data_item) {
                    return -1;
                }
                *output = data_item;
                return XML_DATA;

            case STATE_XML_TAG:
                if (!remains) {
                  return -1;
                }
                if (get_next_xml_tag(input, len) == -1) {
                  return -1;
                }
                return XML_EMPTY;

            case STATE_OTAG_OPEN:
                if (!remains) { return -1; }

                xml_item_t *otag = get_next_otag(input, len);
                if (!otag) {
                  return -1;
                }
                *output = otag;
                return XML_OTAG;

            case STATE_CTAG_OPEN:
                if (!remains) { return -1; }

                xml_item_t *ctag = get_next_ctag(input, len);
                if (!ctag) {
                  return -1;
                }
                *output = ctag;
                return XML_CTAG;
        }
    }
    return -1;
}

xml_item_t *get_next_otag(char32_t **input, unsigned int *len) {
    xml_item_t *result;
    char32_t *data, *sym, current;
    unsigned int in_len, i, remains;

    data = *input + 1;
    in_len = *len - 1;

    for (i = 0, sym = data; i < in_len; sym++, i++) {
        current = *sym;
        remains = in_len - i - 1;

        if (current == '>') {
            if (!i) { return NULL; }

            char32_t *tag_name = char32_alloc(i);
            strncpy32(tag_name, data, i);

            char32_t *out = NULL;
            if (remains) {
                out = char32_alloc(remains);
                strncpy32(out, sym + 1, remains);
            }

            free(*input);

            *input = out;
            *len = remains;
            xml_item_t *result = new_xml_item(XML_OTAG, tag_name, NULL, 0);
            free(tag_name);

            return result;
        }
    }
    return NULL;
}

xml_item_t *get_next_ctag(char32_t **input, unsigned int *len) {
    xml_item_t *result;
    char32_t *data, *sym, current;
    unsigned int in_len, i, remains;

    data = *input + 2;
    in_len = *len - 2;

    for (i = 0, sym = data; i < in_len; sym++, i++) {
        current = *sym;
        remains = in_len - i - 1;

        if (current == '>') {
            if (!i) { return NULL; }

            char32_t *tag_name = char32_alloc(i);
            strncpy32(tag_name, data, i);

            char32_t *out = NULL;
            if (remains) {
                out = char32_alloc(remains);
                strncpy32(out, sym + 1, remains);
            }

            free(*input);

            *input = out;
            *len = remains;

            xml_item_t *result = new_xml_item(XML_CTAG, tag_name, NULL, 0);
            free(tag_name);

            return result;
        }
    }
    return NULL;
}

int get_next_xml_tag(char32_t **input, unsigned int *len) {
    xml_item_t *result;
    char32_t *data, *sym, current;
    unsigned int in_len, i, remains;

    data = *input;
    in_len = *len;

    for (i = 0, sym = data; i < in_len; sym++, i++) {
        current = *sym;
        remains = *len - i - 1;

        if (current == '>') {
            if (!i) { return -1; }

            char32_t *out = NULL;

            if (remains) {
                out = char32_alloc(remains);
                strncpy32(out, sym + 1, remains);
            }

            free(data);

            *input = out;
            *len = remains;

            return 0;
        }
    }
    return -1;
}

xml_item_t *get_next_data(char32_t **input, unsigned int *len) {
    xml_item_t *result;
    char32_t *data, *sym, current;
    unsigned int in_len, i, remains;

    data = *input;
    in_len = *len;

    for (i = 0, sym = data; i < in_len; sym++, i++) {
        current = *sym;
        remains = *len - i - 1;

        if (current == '<') {
            if (!i) {
                // Processing case <a></a> (?)
            }

            char32_t *out = NULL;
            char32_t *processed_data = char32_alloc(i);
            strncpy32(processed_data, data, i);

            if (remains) {
                out = char32_alloc(remains + 1);
                strncpy32(out, sym, remains + 1);
            }

            free(data);

            *input = out;
            *len = remains + 1;

            xml_item_t *result = new_xml_item(XML_DATA, processed_data, NULL, 0);
            free(processed_data);
            return result;
        }
    }
    return NULL;
}
