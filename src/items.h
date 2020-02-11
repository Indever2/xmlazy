#ifdef __APPLE__
    #include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #include "osx_uchar_fixes.h"
    #endif
#else
    #include <uchar.h>
#endif


typedef struct xml_item_s {
    unsigned        empty;
    unsigned int    type;
    char32_t         *data;
    char32_t         *property;
} xml_item_t;

void inspect_xml_item(xml_item_t*);

int free_xml_item(xml_item_t *item);

xml_item_t *new_xml_item(unsigned int type, char32_t *data, char32_t *property, unsigned empty);


/* XML Chain - list of xml items */
typedef struct xml_chain_s {
    xml_item_t *data;
    struct xml_chain_s *next;
} xml_chain_t;

void free_chain(xml_chain_t*);
void inspect_xml_chain(xml_chain_t*);

int xml_chain_len(const xml_chain_t*);
int add_item_to_chain(xml_chain_t**, xml_item_t*);

xml_chain_t *create_empty_chain(void);
xml_chain_t *create_chain_entry(xml_item_t*);
