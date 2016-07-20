#ifndef LYNX_LWM2M_MAP_H
#define LYNX_LWM2M_MAP_H

#include "../include/lwm2m_object.h"

typedef struct lwm2m_map lwm2m_map;

typedef struct lwm2m_map {
    int table_size;
    int size;
    element *data;
};

// TODO need a way to iterate on keys

/* Creates a new map */
lwm2m_map* lwm2m_map_new();

/* Puts node into map */
void lwm2m_map_put(lwm2m_map* map, int key, lwm2m_node* value);

/* Gets node from map */
lwm2m_node* lwm2m_map_get(lwm2m_map* map, int key);

/* Removes node from map */
void lwm2m_map_remove(lwm2m_map* map, int key);

void lwm2m_map_get_keys(int* keys);

/* Frees a map */
void lwm2m_map_free(lwm2m_map* map);

/* Returns a size of map */
int lwm2m_map_length(lwm2m_map* map);

#endif //LYNX_LWM2M_MAP_H
