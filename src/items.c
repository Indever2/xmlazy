#include <stdlib.h>
#include <stdio.h>

#include "items.h"
#include "chars.h"

int xml_chain_len(const xml_chain_t *first) {
    if (!first) {
        return -1;
    }
    if (first->data == NULL) {
        return 0;
    }

    int len = 1;
    const xml_chain_t *ptr = first;

    while (ptr->next != NULL) {
        len++;
        ptr = ptr->next;
    }
    return len;
}

xml_chain_t *create_empty_chain() {
    xml_chain_t *empty = (xml_chain_t*)malloc(sizeof(xml_chain_t));
    empty->data = NULL;
    empty->next = NULL;
    return empty;
}

xml_chain_t *create_chain_entry(xml_item_t *item) {
    xml_chain_t *entry = (xml_chain_t*)malloc(sizeof(xml_chain_t));
    entry->data = item;
    entry->next =NULL;
    return entry;
}

int add_item_to_chain(xml_chain_t **chain, xml_item_t *item) {
    xml_chain_t *first = *chain, *ptr = first;

    if (xml_chain_len(first) == 0) {
        first->data = item;
        return 0;
    }

    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    if (ptr->next == NULL) {
        ptr->next = create_chain_entry(item);
        return 0;
    }
    return -1;
}

void free_chain(xml_chain_t *first) {
    if (!first) {
        return;
    }
    if (first->next == NULL) {
        if (first->data != NULL) {
            free_xml_item(first->data);
        }
        free(first);
        return;
    }
    free_chain(first->next);
    if (first->data) {
        free_xml_item(first->data);
    }
    free(first);
}

void inspect_xml_chain(xml_chain_t *first)
{
    if (!first) {
        return;
    }

    inspect_xml_item(first->data);
    if (first->next) {
        inspect_xml_chain(first->next);
    }

    return;
}

int free_xml_item(xml_item_t *item)
{
    if (!item) {
        return -1;
    }

    if (item->data) {
        free(item->data);
    }
    if(item->property) {
        free(item->property);
    }
    free(item);
    return 0;
}

xml_item_t *new_xml_item(unsigned int type, char32_t *data, char32_t *property, unsigned empty) {
    xml_item_t *res = (xml_item_t *)malloc(sizeof(xml_item_t));

    res->empty = empty;
    res->type = type;
    res->data = NULL;
    res->property = NULL;

    if (data) {
        char32_t *data_p = char32_alloc(strlen32(data));
        strncpy32(data_p, data, strlen32(data));
        res->data = data_p;
    }

    if (property) {
        char32_t *property_p = char32_alloc(strlen32(property));
        strncpy32(property_p, property, strlen32(property));
        res->property = property_p;
    }

    return res;
}

void inspect_xml_item(xml_item_t *item) {
#if 0
#define DEBUG 0
    if (item && DEBUG) {
        printf("XML: xml_item: e: %u;\n", item->empty);
        printf("XML: type: %u\n", item->type);
        if (item->data) {
            printf("XML: data: %S\n", item->data);
        }
        if (item->property)
        {
            printf("XML: property: %S\n", item->property);
        }
    }
#endif
}
