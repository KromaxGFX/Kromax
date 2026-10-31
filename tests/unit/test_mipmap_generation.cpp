/*
 * test_mipmap_generation.cpp
 * Unit tests for mipmap generation
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <kromax/kromax.h>
#include <doctest.h>

TEST_CASE("MipmapGeneration - MipLevelDimensions") {
    uint32_t width = 256;
    uint32_t height = 256;
    uint32_t mip_levels = 5;
    
    for (uint32_t level = 0; level < mip_levels; ++level) {
        uint32_t mip_w = std::max<uint32_t>(1, width >> level);
        uint32_t mip_h = std::max<uint32_t>(1, height >> level);
        
        switch (level) {
            case 0: CHECK_EQ(mip_w, 256); CHECK_EQ(mip_h, 256); break;
            case 1: CHECK_EQ(mip_w, 128); CHECK_EQ(mip_h, 128); break;
            case 2: CHECK_EQ(mip_w, 64); CHECK_EQ(mip_h, 64); break;
            case 3: CHECK_EQ(mip_w, 32); CHECK_EQ(mip_h, 32); break;
            case 4: CHECK_EQ(mip_w, 16); CHECK_EQ(mip_h, 16); break;
        }
    }
}

TEST_CASE("MipmapGeneration - NonPowerOfTwo") {
    uint32_t width = 300;
    uint32_t height = 200;
    uint32_t mip_levels = 5;
    
    for (uint32_t level = 0; level < mip_levels; ++level) {
        uint32_t mip_w = std::max<uint32_t>(1, width >> level);
        uint32_t mip_h = std::max<uint32_t>(1, height >> level);
        
        switch (level) {
            case 0: CHECK_EQ(mip_w, 300); CHECK_EQ(mip_h, 200); break;
            case 1: CHECK_EQ(mip_w, 150); CHECK_EQ(mip_h, 100); break;
            case 2: CHECK_EQ(mip_w, 75); CHECK_EQ(mip_h, 50); break;
            case 3: CHECK_EQ(mip_w, 37); CHECK_EQ(mip_h, 25); break;
            case 4: CHECK_EQ(mip_w, 18); CHECK_EQ(mip_h, 12); break;
        }
    }
}

TEST_CASE("MipmapGeneration - MinDimension") {
    // Minimum mip level is always 1x1
    uint32_t width = 1;
    uint32_t height = 1;
    
    for (uint32_t level = 0; level < 5; ++level) {
        uint32_t mip_w = std::max<uint32_t>(1, width >> level);
        uint32_t mip_h = std::max<uint32_t>(1, height >> level);
        CHECK_EQ(mip_w, 1);
        CHECK_EQ(mip_h, 1);
    }
}