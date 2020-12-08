/*
 * Generic hashmap manipulation functions
 * SEE: http://elliottback.com/wp/hashmap-implementation-in-c/
 */

#ifndef __HASHMAP_H__

#include "Kashmap.h"

/*
* hashmap need to handle all mem access
*/

#define __HASHMAP_H__

#define MAP_MISSING -3  /* No such element */
#define MAP_FULL -2     /* Hashmap is full */
#define MAP_OMEM -1     /* Out of Memory */
#define MAP_OK 0    /* OK */

 /*
  * any_t is a pointer.  This allows you to put arbitrary structures in
  * the hashmap.
  */
typedef PVOID any_t;

/*
 * PFany is a pointer to a function that can take two any_t arguments
 * and return an integer. Returns status code..
 */
typedef INT (*PFany)(any_t, any_t);

/*
 * map_t is a pointer to an internally maintained data structure.
 * Clients of this package do not need to know how hashmaps are
 * represented.  They see and manipulate only map_t's.
 */
typedef any_t map_t;

#define INITIAL_SIZE 1024

// We need to keep keys and values
typedef struct _hashmap_element {
    INT key;
    INT in_use;
    any_t data;
} hashmap_element;

// A hashmap has some maximum size and current size,
// as well as the data to hold.
typedef struct _hashmap_map {
    INT table_size;
    INT size;
    hashmap_element* data;
} hashmap_map;

#if defined(__cplusplus)
extern "C" {
#endif

    /*
    * Return an empty hashmap, or NULL on failure.
    */
    static map_t ihashmap_new();

    /*
     * Hashing function for an integer
     */
    static UINT32 ihashmap_hash_int(hashmap_map* map, UINT32 key);

    /*
     * Return the integer of the location in data
     * to store the point to the item, or MAP_FULL.
     */
    static INT ihashmap_hash(map_t in, INT key);

    /*
     * Doubles the size of the hashmap, and rehashes all the elements
     */
    static INT ihashmap_rehash(map_t in);

    /*
     * Add a pointer to the hashmap with some key
     */
    static INT ihashmap_put(map_t in, INT key, any_t value);

    /*
     * Get your pointer out of the hashmap with a key
     */
    static INT ihashmap_get(map_t in, INT key, any_t* arg);

    /*
     * Get a random element from the hashmap
     */
    static INT ihashmap_get_one(map_t in, any_t* arg, INT remove);

    /*
     * Iterate the function parameter over each element in the hashmap.  The
     * additional any_t argument is passed to the function as its first
     * argument and the hashmap element is the second.
     */
    static INT ihashmap_iterate(map_t in, PFany f, any_t item);

    /*
     * Remove an element with that key from the map
     */
    static INT ihashmap_remove(map_t in, INT key);

    /* Deallocate the hashmap */
    static VOID ihashmap_free(map_t in);

    /* Return the length of the hashmap */
    static INT ihashmap_length(map_t in);

#if defined(__cplusplus)
}
#endif

/*
 * Return an empty hashmap, or NULL on failure.
 */
map_t ihashmap_new() {
    hashmap_map* map = (hashmap_map*)malloc(sizeof(hashmap_map));
    if (!map) goto err;

    map->data = (hashmap_element*)calloc(INITIAL_SIZE, sizeof(hashmap_element));
    if (!map->data) goto err;

    map->table_size = INITIAL_SIZE;
    map->size = 0;

    return map;
err:
    if (map)
        ihashmap_free(map);
    return NULL;
}

/*
 * Hashing function for an integer
 */
UINT32 ihashmap_hash_int(hashmap_map* map, UINT32 key) {
    /* Robert Jenkins' 32 bit Mix Function */
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    /* Knuth's Multiplicative Method */
    key = (key >> 3) * 2654435761;

    return key % map->table_size;
}

/*
 * Return the integer of the location in data
 * to store the point to the item, or MAP_FULL.
 */
INT ihashmap_hash(map_t in, INT key) {
    INT curr;
    INT i;

    /* Cast the hashmap */
    hashmap_map* map = (hashmap_map*)in;

    /* If full, return immediately */
    if (map->size == map->table_size) return MAP_FULL;

    /* Find the best index */
    curr = ihashmap_hash_int(map, key);

    /* Linear probling */
    for (i = 0; i < map->table_size; i++) {
        if (map->data[curr].in_use == 0)
            return curr;

        if (map->data[curr].key == key && map->data[curr].in_use == 1)
            return curr;

        curr = (curr + 1) % map->table_size;
    }

    return MAP_FULL;
}

/*
 * Doubles the size of the hashmap, and rehashes all the elements
 */
INT ihashmap_rehash(map_t in) {
    INT i;
    INT old_size;
    hashmap_element* curr;

    /* Setup the new elements */
    hashmap_map* map = (hashmap_map*)in;
    hashmap_element* temp = (hashmap_element*)
        calloc(2 * map->table_size, sizeof(hashmap_element));
    if (!temp) return MAP_OMEM;

    /* Update the array */
    curr = map->data;
    map->data = temp;

    /* Update the size */
    old_size = map->table_size;
    map->table_size = 2 * map->table_size;
    map->size = 0;

    /* Rehash the elements */
    for (i = 0; i < old_size; i++) {
        INT status = ihashmap_put(map, curr[i].key, curr[i].data);
        if (status != MAP_OK)
            return status;
    }

    free(curr);

    return MAP_OK;
}

/*
 * Add a pointer to the hashmap with some key
 */
INT ihashmap_put(map_t in, INT key, any_t value) {
    INT index;
    hashmap_map* map;

    /* Cast the hashmap */
    map = (hashmap_map*)in;

    /* Find a place to put our value */
    index = ihashmap_hash(in, key);
    while (index == MAP_FULL) {
        if (ihashmap_rehash(in) == MAP_OMEM) {
            return MAP_OMEM;
        }
        index = ihashmap_hash(in, key);
    }

    /* Set the data */
    map->data[index].data = value;
    map->data[index].key = key;
    map->data[index].in_use = 1;
    map->size++;

    return MAP_OK;
}

/*
 * Get your pointer out of the hashmap with a key
 */
INT ihashmap_get(map_t in, INT key, any_t* arg) {
    INT curr;
    INT i;
    hashmap_map* map;

    /* Cast the hashmap */
    map = (hashmap_map*)in;

    /* Find data location */
    curr = ihashmap_hash_int(map, key);

    /* Linear probing, if necessary */
    for (i = 0; i < map->table_size; i++) {

        if (map->data[curr].key == key && map->data[curr].in_use == 1) {
            *arg = (PINT32)(map->data[curr].data);
            return MAP_OK;
        }

        curr = (curr + 1) % map->table_size;
    }

    *arg = NULL;

    /* Not found */
    return MAP_MISSING;
}

/*
 * Get a random element from the hashmap
 */
INT ihashmap_get_one(map_t in, any_t* arg, INT remove) {
    INT i;
    hashmap_map* map;

    /* Cast the hashmap */
    map = (hashmap_map*)in;

    /* On empty hashmap return immediately */
    if (ihashmap_length(map) <= 0)
        return MAP_MISSING;

    /* Linear probing */
    for (i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0) {
            *arg = (any_t)(map->data[i].data);
            if (remove) {
                map->data[i].in_use = 0;
                map->size--;
            }
            return MAP_OK;
        }

    return MAP_OK;
}

/*
 * Iterate the function parameter over each element in the hashmap.  The
 * additional any_t argument is passed to the function as its first
 * argument and the hashmap element is the second.
 */
INT ihashmap_iterate(map_t in, PFany f, any_t item) {
    INT i;

    /* Cast the hashmap */
    hashmap_map* map = (hashmap_map*)in;

    /* On empty hashmap, return immediately */
    if (ihashmap_length(map) <= 0)
        return MAP_MISSING;

    /* Linear probing */
    for (i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0) {
            any_t data = (any_t)(map->data[i].data);
            INT status = f(item, data);
            if (status != MAP_OK) {
                return status;
            }
        }

    return MAP_OK;
}

/*
 * Remove an element with that key from the map
 */
INT ihashmap_remove(map_t in, INT key) {
    INT i;
    INT curr;
    hashmap_map* map;

    /* Cast the hashmap */
    map = (hashmap_map*)in;

    /* Find key */
    curr = ihashmap_hash_int(map, key);

    /* Linear probing, if necessary */
    for (i = 0; i < map->table_size; i++) {
        if (map->data[curr].key == key && map->data[curr].in_use == 1) {
            /* Blank out the fields */
            map->data[curr].in_use = 0;
            map->data[curr].data = NULL;
            map->data[curr].key = 0;

            /* Reduce the size */
            map->size--;
            return MAP_OK;
        }
        curr = (curr + 1) % map->table_size;
    }

    /* Data not found */
    return MAP_MISSING;
}

/* Deallocate the hashmap */
VOID ihashmap_free(map_t in) {
    hashmap_map* map = (hashmap_map*)in;
    free(map->data);
    free(map);
}

/* Return the length of the hashmap */
INT ihashmap_length(map_t in) {
    hashmap_map* map = (hashmap_map*)in;
    if (map != NULL) return map->size;
    else return 0;
}

#endif __HASHMAP_H__
