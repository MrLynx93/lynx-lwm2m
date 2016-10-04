#ifndef LYNX_LWM2M_MAP_H
#define LYNX_LWM2M_MAP_H


typedef struct element {
    int key;
    char* key_string;
    int in_use;
    void *data;
} element;

typedef struct lwm2m_map {
    int table_size;
    int size;
    element *data;
} lwm2m_map;


///////////// PUT ////////////////////

/* Puts node into map */
void lwm2m_map_put(lwm2m_map* map, int key, void* value);

/* Puts node into map. String is the key */
void lwm2m_map_put_string(lwm2m_map* map, char* key, void* value);


///////////// GET ////////////////////

/* Gets node from map */
void* lwm2m_map_get(lwm2m_map* map, int key);

/* Gets node from map. String is the key */
void* lwm2m_map_get_string(lwm2m_map* map, char* key);


///////////// REMOVE ////////////////////

/* Removes node from map */
void lwm2m_map_remove(lwm2m_map* map, int key);

/* Removes node from map. String is the key */
void lwm2m_map_remove_string(lwm2m_map* map, char* key);


///////////// GET KEYS ////////////////////

void lwm2m_map_get_keys(lwm2m_map* map, int* keys);

void lwm2m_map_get_keys_string(lwm2m_map* map, char** keys);


///////////// UTILS ////////////////////

/* Creates a new map */
lwm2m_map* lwm2m_map_new();

/* Frees a map */
void lwm2m_map_free(lwm2m_map* map);

/* Returns a size of map */
int lwm2m_map_length(lwm2m_map* map);

#endif //LYNX_LWM2M_MAP_H
