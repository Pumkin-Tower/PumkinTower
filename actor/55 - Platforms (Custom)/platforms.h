#ifndef Z_PLATFORMS_H
#define Z_PLATFORMS_H

#include "ultra64.h"
#include "global.h"

struct Platforms;

typedef void (*PlatformsActionFunc)(struct Platforms*, PlayState*);

typedef struct Platforms {
    DynaPolyActor dyna;
    PlatformsActionFunc actionFunc;
    f32 dipOffset;
    u16 timer;
    s16 alphaTarget;
    s16 alpha;
    u8 playerOn;
} Platforms;

#endif