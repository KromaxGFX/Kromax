/*
 * SoftwareDescriptorSet.cpp
 *
 * Software descriptor set: implementation.
 */

#include "SoftwareDescriptorSet.h"
#include "SoftwareDevice.h"

namespace kx {

KxResult SoftwareDescriptorSet::update(const KxWriteDescriptorSet& write) {
    if (!layout_) return KX_ERROR_INITIALIZATION_FAILED;
    if (!device_) return KX_ERROR_DEVICE_LOST;

    // Find binding index by binding number
    const auto* binding = layout_->find_binding(write.dstBinding);
    if (!binding) return KX_ERROR_FEATURE_NOT_PRESENT; // Binding not in layout

    if (write.dstArrayElement + write.descriptorCount > binding->descriptorCount) {
        return KX_ERROR_FEATURE_NOT_PRESENT; // Out of bounds
    }

    if (write.descriptorType != binding->descriptorType) {
        return KX_ERROR_FEATURE_NOT_PRESENT; // Type mismatch
    }

    auto& binding_vec = binding_data_[write.dstBinding];

    if (write.descriptorType == KX_DESCRIPTOR_TYPE_SAMPLER ||
        write.descriptorType == KX_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
        write.descriptorType == KX_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
        write.descriptorType == KX_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
        write.descriptorType == KX_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {

        if (!write.pImageInfo) return KX_ERROR_FEATURE_NOT_PRESENT;

        for (uint32_t i = 0; i < write.descriptorCount; ++i) {
            auto& desc = binding_vec[write.dstArrayElement + i];
            desc.data = DescriptorData::ImageDescriptor{
                device_->find_image_view(write.pImageInfo[i].imageView),
                device_->find_sampler(write.pImageInfo[i].sampler),
                write.pImageInfo[i].imageLayout
            };
            desc.valid = true;
        }
    }
    else if (write.descriptorType == KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
             write.descriptorType == KX_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
             write.descriptorType == KX_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
             write.descriptorType == KX_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {

        if (!write.pBufferInfo) return KX_ERROR_FEATURE_NOT_PRESENT;

        for (uint32_t i = 0; i < write.descriptorCount; ++i) {
            auto& desc = binding_vec[write.dstArrayElement + i];
            desc.data = DescriptorData::BufferDescriptor{
                device_->find_buffer(write.pBufferInfo[i].buffer),
                write.pBufferInfo[i].offset,
                write.pBufferInfo[i].range
            };
            desc.valid = true;
        }
    }

    return KX_SUCCESS;
}

} /* namespace kx */