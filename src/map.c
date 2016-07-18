#include "../include/map.h"

#define INITIAL_SIZE 1024
#define MAP_FULL 1

static typedef struct element {
    int key;
    int in_use;
    lwm2m_node *data;
} element;

lwm2m_map lwm2m_map_new() {
    lwm2m_map *map = (lwm2m_map *) malloc(sizeof(lwm2m_map));
    map->data = (element *) calloc(INITIAL_SIZE, sizeof(element));
    map->table_size = INITIAL_SIZE;
    map->size = 0;
    return map;
}

void lwm2m_map_put(lwm2m_map *map, int key, lwm2m_node *value) {
    int index = lwm2m_map_hash(map, key);
    if (index == MAP_FULL) {
        lwm2m_map_rehash(map);
        index = lwm2m_map_hash(map, key);
    }
    map->data[index].data = value;
    map->data[index].key = key;
    map->data[index].in_use = 1;
    map->size++;
}

lwm2m_node *lwm2m_map_get(lwm2m_map *map, int key) {
    int curr = hash_int(m, key);
    for (int i = 0; i < map->table_size; i++) {
        if (map->data[curr].key == key && map->data[curr].in_use) {
            return map->data[curr].data;
        }
        curr = (curr + 1) % map->table_size;
    }
    return NULL;
}

void lwm2m_map_remove(lwm2m_map map, int key) {
    int curr = hash_int(map, key);
    for (int i = 0; i < map->table_size; i++) {
        if (map->data[curr].key == key && map->data[curr].in_use) {
            map->data[curr].in_use = 0;
            map->data[curr].data = NULL;
            map->data[curr].key = 0;
        }
        curr = (curr + 1) % map->table_size;
    }
}

void lwm2m_map_free(lwm2m_map *map) {
    free(map->data);
    free(map);
}

static void lwm2m_map_rehash(lwm2m_map *map) {
    int old_size;
    element *curr;

    element *temp = (element *) calloc(2 * map->table_size, sizeof(element));
    curr = map->data;
    map->data = temp;

    old_size = map->table_size;
    map->table_size = 2 * map->table_size;
    map->size = 0;

    for (int i = 0; i < old_size; i++) {
        lwm2m_map_put(map, curr[i].key, curr[i].data);
    }
    free(curr);
}

static int lwm2m_map_hash(lwm2m_map *map, int key) {
    if (map->size == map->table_size) {
        return MAP_FULL;
    }

    int curr = hash_int(map, key);
    for (int i = 0; i < map->table_size; i++) {
        if (map->data[curr].in_use || (map->data[curr].key == key && m->data[curr].in_use)) {
            return curr;
        }
        curr = (curr + 1) % map->table_size;
    }
    return MAP_FULL;
}

static unsigned int hash_int(lwm2m_map *map, unsigned int key) {
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);
    key = (key >> 3) * 2654435761;
    return key % map->table_size;
}
