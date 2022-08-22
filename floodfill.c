#include "floodfill.h"

#ifndef NULL
#define NULL 0
#endif

// #define FLOODFILL_SILENCE
#ifndef FLOODFILL_SILENCE
#include "stdio.h"
#endif

// #define FLOODFILL_IGNORE_ASSERTS
#ifndef FLOODFILL_IGNORE_ASSERTS
#include "assert.h"
#endif

typedef struct NodeToExplore {
    int32_t x;
    int32_t y;
} NodeToExplore;

/*
Get the first RGBA location (so the location of the R) given x and y,
where [0,0] is the top left pixel
*/
static uint32_t xy_to_pixelstart(
    const uint32_t x,
    const uint32_t y,
    const uint32_t img_width)
{
    return (y * 4 * img_width) + (x * 4);
}

static uint32_t node_to_pixelstart(
    const NodeToExplore node,
    const uint32_t img_width)
{
    return xy_to_pixelstart(node.x, node.y, img_width);
}

/*
The 'Cantor pairing function' converts pairs (our coordinates x and y) into
consecutive integers, so it's perfect as a hashing function for this use
*/
static uint32_t cantor_hash(
    const int32_t x,
    const int32_t y)
{
   return ((x + y)*(x + y + 1)/2) + y;
}

/*
Check if there's a 'true' node at [x,y] in a boolean hashset
*/
static uint32_t hashset_find(
    uint8_t * hashset,
    int32_t x,
    int32_t y)
{
    #ifndef FLOODFILL_IGNORE_ASSERTS
    assert(x >= 0);
    assert(y >= 0);
    #endif
    
    uint32_t location_bits = cantor_hash(x,y);
    
    return (hashset[location_bits / 8] >> (location_bits % 8)) & 1;
}

/*
Register a 'true' node at [x,y] in a boolean hashset
*/
static void hashset_register(
    uint8_t * hashset,
    int32_t x,
    int32_t y)
{
    #ifndef FLOODFILL_IGNORE_ASSERTS
    assert(x >= 0);
    assert(y >= 0);
    #endif

    uint32_t location_bits = cantor_hash(x, y);
    
    hashset[location_bits / 8] |= 1 << (location_bits % 8);

    #ifndef FLOODFILL_IGNORE_ASSERTS
    assert(hashset_find(hashset, x, y));
    #endif
}

void floodfill(
    uint8_t * rgba,
    const uint32_t rgba_size,
    const uint32_t width,
    const uint32_t height,
    const uint32_t at_x,
    const uint32_t at_y,
    const uint8_t replacement_RGBA[4],
    const uint8_t * working_memory,
    const uint64_t working_memory_size)
{
    uint8_t * working_memory_at = (uint8_t *)working_memory;
    #ifndef FLOODFILL_IGNORE_ASSERTS
    uint64_t working_memory_left = (uint64_t)working_memory_size;
    #endif
    
    uint32_t explored_hashset_cap = (cantor_hash(height, width) / 8) + 1;
    #ifndef FLOODFILL_IGNORE_ASSERTS
    assert(working_memory_size >= explored_hashset_cap);
    #endif
    uint8_t * explored_hashset = working_memory_at;
    working_memory_at += explored_hashset_cap;
    
    #ifndef FLOODFILL_IGNORE_ASSERTS
    assert(working_memory_left > explored_hashset_cap);
    working_memory_left -= explored_hashset_cap;
    #endif
    for (uint32_t i = 0; i < explored_hashset_cap; i++) {
        explored_hashset[i] = 0;
    }
    
    uint32_t queued_for_exploring_hashset_cap =
        (cantor_hash(
            height,
            width) / 8) + 1;
    #ifndef FLOODFILL_IGNORE_ASSERTS
    assert(working_memory_size >= queued_for_exploring_hashset_cap);
    working_memory_left -= queued_for_exploring_hashset_cap;
    #endif
    uint8_t * queued_for_exploring_hashset = working_memory_at;
    working_memory_at += queued_for_exploring_hashset_cap;
    for (uint32_t i = 0; i < queued_for_exploring_hashset_cap; i++) {
        queued_for_exploring_hashset[i] = 0;
    }
    
    uint32_t to_explore_cap = width * height;
    // align to 4 bytes
    while (((uintptr_t)(const void *)working_memory_at & 0x3)) {
        #ifndef FLOODFILL_IGNORE_ASSERTS
        assert(working_memory_left > 0);
        working_memory_left -= 1;
        #endif
        working_memory_at++;
    }

    #ifndef FLOODFILL_IGNORE_ASSERTS
    assert(working_memory_left > to_explore_cap * sizeof(NodeToExplore));
    working_memory_left -= to_explore_cap;
    #endif
    NodeToExplore * to_explore = (NodeToExplore *)working_memory_at;
    for (uint32_t i = 0; i < to_explore_cap; i++) {
        to_explore[i].x = 0;
        to_explore[i].y = 0;
    }
    working_memory_at += to_explore_cap;
    
    to_explore[0].x = at_x;
    to_explore[0].y = at_y;
    uint32_t to_explore_size = 1;
    
    uint8_t target_RGBA[4];
    uint32_t initial_pixelstart = node_to_pixelstart(
        to_explore[0],
        width);
    for (uint32_t _ = 0; _ < 4; _++) {
        target_RGBA[_] = rgba[initial_pixelstart + _];
    }
    
    #ifndef FLOODFILL_SILENCE
    printf(
        "target_RGBA (set from pixelstart %u): [%u,%u,%u,%u]\n",
        initial_pixelstart,
        target_RGBA[0],
        target_RGBA[1],
        target_RGBA[2],
        target_RGBA[3]);
    #endif
    
    hashset_register(queued_for_exploring_hashset, at_x, at_y);
    uint32_t pixelstart = xy_to_pixelstart(
        /* const uint32_t x: */
            at_x,
        /* const uint32_t y: */
            at_y,
        /* const uint32_t img_width: */
            width);
    rgba[pixelstart + 0] = replacement_RGBA[0];
    rgba[pixelstart + 1] = replacement_RGBA[1];
    rgba[pixelstart + 2] = replacement_RGBA[2];
    rgba[pixelstart + 3] = replacement_RGBA[3];
    
    // pop off a node to explore
    while (to_explore_size > 0) {
        NodeToExplore exploring = to_explore[to_explore_size-1];
        to_explore_size -= 1;
        
        #ifndef FLOODFILL_IGNORE_ASSERTS
        assert(!hashset_find(explored_hashset, exploring.x, exploring.y));
        #endif
        hashset_register(explored_hashset, exploring.x, exploring.y);
        
        for (
            int32_t try_y = exploring.y - 1;
            try_y <= exploring.y + 1;
            try_y++)
        {
            for (
                int32_t try_x = exploring.x - 1;
                try_x <= (exploring.x + 1);
                try_x++)
            {
                if (
                    try_x >= 0
                    && try_x < width
                    && try_y >= 0
                    && try_y < height
                    && !hashset_find(
                        queued_for_exploring_hashset,
                        try_x,
                        try_y))
                {
                    uint32_t pixelstart = xy_to_pixelstart(
                        /* const uint32_t x: */
                            try_x,
                        /* const uint32_t y: */
                            try_y,
                        /* const uint32_t img_width: */
                            width);
                    
                    if (
                        rgba[pixelstart + 0] == target_RGBA[0] &&
                        rgba[pixelstart + 1] == target_RGBA[1] &&
                        rgba[pixelstart + 2] == target_RGBA[2] &&
                        rgba[pixelstart + 3] == target_RGBA[3])
                    {
                        rgba[pixelstart + 0] = replacement_RGBA[0];
                        rgba[pixelstart + 1] = replacement_RGBA[1];
                        rgba[pixelstart + 2] = replacement_RGBA[2];
                        rgba[pixelstart + 3] = replacement_RGBA[3];
                        
                        #ifndef FLOODFILL_IGNORE_ASSERTS
                        assert(to_explore_size + 1 < to_explore_cap);
                        
                        for (uint32_t i = 0; i < to_explore_size; i++) {
                            if (
                                (to_explore[i].x == try_x) &&
                                (to_explore[i].y == try_y))
                            {
                                assert(0);
                            }
                        }
                        #endif
                        
                        to_explore[to_explore_size].x = try_x;
                        to_explore[to_explore_size].y = try_y;
                        to_explore_size++;
                        hashset_register(queued_for_exploring_hashset, try_x, try_y);
                    }
                }
            }
        }
    }
    
    #ifndef FLOODFILL_SILENCE 
    printf("bytes of memory used: %llu\n", working_memory_size - working_memory_left);
    #endif
    #ifndef FLOODFILL_IGNORE_ASSERTS
    assert(working_memory_at - working_memory < working_memory_size);
    #endif
}

