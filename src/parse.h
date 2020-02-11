#define XML_EMPTY   0
#define XML_END     1
#define XML_OTAG    2
#define XML_CTAG    3
#define XML_DATA    4

#define XML_ERROR   255

/* Parse functions */
xml_chain_t *parse(const char32_t*, unsigned int);

int get_next(char32_t**, unsigned int*, xml_item_t**);

xml_item_t *get_next_otag(char32_t**, unsigned int*);
xml_item_t *get_next_ctag(char32_t**, unsigned int*);
xml_item_t *get_next_data(char32_t**, unsigned int*);
int get_next_xml_tag(char32_t**, unsigned int*);
