#pragma once

#include <typedefs.h>
#include <util/Buffer.h>
#include <files/world_regions_fwd.h>

namespace compatibility {
    util::Buffer<ubyte> convert_region_2to3(
        const util::Buffer<ubyte>& src, RegionLayerIndex layer
    );
}
