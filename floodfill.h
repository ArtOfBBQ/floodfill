#include "inttypes.h"

/*
Floodfill or 'color in' a chunk of an image with a new color, just like you
remember doing with MSPaint in 1996.

- rgba:
    the red/green/blue/alpha values of your image.
- rgba_size:
    the number of rgba values for your image.
- width:
    the width of your image.
- height:
    the height of your image.
- at_x:
    the x-coordinate for the pixel where you want to floodfill, starting
    from 0.
- at_y:
    the y-coordinate for the pixel where you want to floodfill, starting
    from 0.
- replacement_RGBA:
    the color you want to floodfill with.
- working_memory:
    a buffer of allocated memory that floodfill may use. You can overwrite
    this memory immediately after floodfill ends.

    If you want to use floodfill() on multiple threads at the same time, each
    instance must have its own unique working memory to avoid overwriting each
    other's data.
*/
void floodfill(
    uint8_t * rgba,
    const uint32_t rgba_size,
    const uint32_t width,
    const uint32_t height,
    const uint32_t at_x,
    const uint32_t at_y,
    const uint8_t replacement_RGBA[4],
    const uint8_t * working_memory,
    const uint64_t working_memory_size);

