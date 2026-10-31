/*
 * test_sampler_filtering.cpp
 * Unit tests for sampler filtering (nearest, bilinear, trilinear, anisotropic)
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <kromax/kromax.h>
#include <doctest.h>
#include "kx_core.h"

extern "C" {
    KxResult kxSampleTexture(const KxBoundTexture* pTexture, float u, float v, float outColor[4]);
    KxResult kxSampleTextureGrad(const KxBoundTexture* pTexture, float u, float v,
                                 float dudx, float dvdx, float dudy, float dvdy,
                                 float outColor[4]);
    KxResult kxSampleTextureLod(const KxBoundTexture* pTexture, float u, float v, float lod, float outColor[4]);
    KxSamplerState kxDefaultSamplerState(void);
}

TEST_CASE("SamplerFiltering - DefaultState") {
    KxSamplerState state = kxDefaultSamplerState();
    
    CHECK_EQ(state.magFilter, KX_FILTER_NEAREST);
    CHECK_EQ(state.minFilter, KX_FILTER_NEAREST);
    CHECK_EQ(state.addressModeU, KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    CHECK_EQ(state.addressModeV, KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    CHECK_EQ(state.addressModeW, KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    CHECK_EQ(state.mipLodBias, 0.0f);
    CHECK_EQ(state.anisotropyEnable, KX_FALSE);
    CHECK_EQ(state.maxAnisotropy, 1.0f);
}

TEST_CASE("SamplerFiltering - FilterModes") {
    CHECK_EQ(KX_FILTER_NEAREST, 0);
    CHECK_EQ(KX_FILTER_LINEAR, 1);
}

TEST_CASE("SamplerFiltering - AddressModes") {
    CHECK_EQ(KX_SAMPLER_ADDRESS_MODE_REPEAT, 0);
    CHECK_EQ(KX_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, 1);
    CHECK_EQ(KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 2);
    CHECK_EQ(KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 3);
}

TEST_CASE("SamplerFiltering - BorderColors") {
    CHECK_EQ(KX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, 0);
    CHECK_EQ(KX_BORDER_COLOR_INT_TRANSPARENT_BLACK, 1);
    CHECK_EQ(KX_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 2);
    CHECK_EQ(KX_BORDER_COLOR_INT_OPAQUE_BLACK, 3);
    CHECK_EQ(KX_BORDER_COLOR_FLOAT_OPAQUE_WHITE, 4);
    CHECK_EQ(KX_BORDER_COLOR_INT_OPAQUE_WHITE, 5);
}