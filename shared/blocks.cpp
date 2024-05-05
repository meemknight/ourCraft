#include "blocks.h"
#include <algorithm>

bool isOpaque(BlockType type) 
{
    return (blockProperties[type] & Mask::IS_OPAQUE_MASK) != 0;
}

bool isBlockMesh(BlockType type)
{
    return !isCrossMesh(type); // dirty hack for now?
}

bool isTransparentGeometry(BlockType type) 
{
    // I think 'air' shouldn't even be a thing to be honest, you're just trolling yourself
    // Because then you introduce these 'air' blocks that are actually just empty voids that shouldn't even be considered as real.
    // And now you're wasting CPU time calculating nothing.
    if (type == 0) {
        static int count = 0;
        count += 1;
        if (count % 100000 == 0)
            printf("useless calculation count: %i\n", count);

        return false;
    }

    return (blockProperties[type] & Mask::IS_TRANSPARENT_MASK) != 0;
}

bool isLightEmitter(BlockType type) 
{
    return (blockProperties[type] & Mask::IS_LIGHT_EMITTER_MASK) != 0;
}

bool isAnimatedBlock(BlockType type) 
{
    return (blockProperties[type] & Mask::IS_ANIMATED_MASK) != 0;
}

bool isGrassMesh(BlockType type) 
{
    return (blockProperties[type] & Mask::IS_GRASS_MESH_MASK) != 0;
}

bool isCrossMesh(BlockType type) 
{
    return (blockProperties[type] & Mask::IS_CROSS_MESH_MASK) != 0;
}

bool isCollidable(BlockType type) 
{
    return (blockProperties[type] & Mask::IS_COLLIDABLE_MASK) != 0;
}

bool isControlBlock(BlockType type) 
{
    return (blockProperties[type] & Mask::IS_CONTROL_BLOCK_MASK) != 0;
}
