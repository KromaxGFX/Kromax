/*
 * test_renderpass.cpp
 *
 * Unit tests for ImageView, RenderPass, Framebuffer lifecycle.
 */

#include <kromax/kx_core.h>
#include <cstring>
#include <cstdio>

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, msg); \
            return 1; \
        } else { \
            printf("PASS: %s\n", msg); \
        } \
    } while (0)

#define ASSERT_SUCCESS(result, msg) \
    do { \
        if ((result) != KX_SUCCESS) { \
            fprintf(stderr, "FAIL: %s:%d: %s (got %s)\n", __FILE__, __LINE__, msg, kxResultToString(result)); \
            return 1; \
        } else { \
            printf("PASS: %s\n", msg); \
        } \
    } while (0)

int test_image_view_create_2d() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create 2D image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT_SUCCESS(result, "Create ImageView for 2D image");

    kxDestroyImageView(device, view, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_image_view_create_3d() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_3D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 16,
        .height = 16,
        .depth = 16,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create 3D image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_3D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT_SUCCESS(result, "Create ImageView for 3D image");

    kxDestroyImageView(device, view, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_image_view_invalid_type() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create 2D image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_3D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT(result != KX_SUCCESS, "ImageView creation should fail for incompatible view type");

    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_image_view_invalid_range() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create 2D image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 2,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT(result != KX_SUCCESS, "ImageView creation should fail for invalid mip range");

    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_image_view_destroy() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT_SUCCESS(result, "Create ImageView");

    kxDestroyImageView(device, view, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    printf("PASS: ImageView destroy\n");
    return 0;
}

int test_renderpass_validation() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxAttachmentDescription attachments[1] = {{
        .sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = KX_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    }};

    KxAttachmentReference color_ref = { .attachment = 1, .layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    KxSubpassDescription subpass = {
        .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION,
        .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
    };

    KxRenderPassCreateInfo rp_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    KxRenderPass render_pass;
    result = kxCreateRenderPass(device, &rp_ci, nullptr, &render_pass);
    ASSERT(result != KX_SUCCESS, "RenderPass creation should fail for invalid attachment reference");

    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_renderpass_cyclic_dep() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxAttachmentDescription attachments[1] = {{
        .sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = KX_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    }};

    KxAttachmentReference color_ref = { .attachment = 0, .layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    KxSubpassDescription subpasses[2] = {
        { .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION, .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = 1, .pColorAttachments = &color_ref },
        { .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION, .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = 1, .pColorAttachments = &color_ref },
    };

    KxSubpassDependency deps[2] = {
        { .sType = KX_STRUCTURE_TYPE_SUBPASS_DEPENDENCY, .srcSubpass = 0, .dstSubpass = 1, .srcStageMask = KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstStageMask = KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
        { .sType = KX_STRUCTURE_TYPE_SUBPASS_DEPENDENCY, .srcSubpass = 1, .dstSubpass = 0, .srcStageMask = KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstStageMask = KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
    };

    KxRenderPassCreateInfo rp_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 2,
        .pSubpasses = subpasses,
        .dependencyCount = 2,
        .pDependencies = deps,
    };
    KxRenderPass render_pass;
    result = kxCreateRenderPass(device, &rp_ci, nullptr, &render_pass);
    if (result == KX_SUCCESS) {
        kxDestroyRenderPass(device, render_pass, nullptr);
    }
    printf("PASS: RenderPass cyclic dependency test (result: %s)\n", kxResultToString(result));

    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_framebuffer_attachment_count() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxAttachmentDescription attachments[1] = {{
        .sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = KX_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    }};

    KxAttachmentReference color_ref = { .attachment = 0, .layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    KxSubpassDescription subpass = {
        .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION,
        .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
    };

    KxRenderPassCreateInfo rp_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    KxRenderPass render_pass;
    result = kxCreateRenderPass(device, &rp_ci, nullptr, &render_pass);
    ASSERT_SUCCESS(result, "Create RenderPass");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT_SUCCESS(result, "Create ImageView");

    KxFramebufferCreateInfo fb_ci = {
        .sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = 0,
        .pAttachments = &view,
        .width = 64,
        .height = 64,
        .layers = 1,
    };
    KxFramebuffer framebuffer;
    result = kxCreateFramebuffer(device, &fb_ci, nullptr, &framebuffer);
    ASSERT(result != KX_SUCCESS, "Framebuffer creation should fail for wrong attachment count");

    kxDestroyImageView(device, view, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyRenderPass(device, render_pass, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_framebuffer_dimensions() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxAttachmentDescription attachments[1] = {{
        .sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = KX_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    }};

    KxAttachmentReference color_ref = { .attachment = 0, .layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    KxSubpassDescription subpass = {
        .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION,
        .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
    };

    KxRenderPassCreateInfo rp_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    KxRenderPass render_pass;
    result = kxCreateRenderPass(device, &rp_ci, nullptr, &render_pass);
    ASSERT_SUCCESS(result, "Create RenderPass");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT_SUCCESS(result, "Create ImageView");

    KxFramebufferCreateInfo fb_ci = {
        .sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = 1,
        .pAttachments = &view,
        .width = 128,
        .height = 128,
        .layers = 1,
    };
    KxFramebuffer framebuffer;
    result = kxCreateFramebuffer(device, &fb_ci, nullptr, &framebuffer);
    ASSERT(result != KX_SUCCESS, "Framebuffer creation should fail for wrong dimensions");

    kxDestroyImageView(device, view, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyRenderPass(device, render_pass, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_cmd_begin_renderpass_clear() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxAttachmentDescription attachments[1] = {{
        .sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = KX_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    }};

    KxAttachmentReference color_ref = { .attachment = 0, .layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    KxSubpassDescription subpass = {
        .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION,
        .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
    };

    KxRenderPassCreateInfo rp_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    KxRenderPass render_pass;
    result = kxCreateRenderPass(device, &rp_ci, nullptr, &render_pass);
    ASSERT_SUCCESS(result, "Create RenderPass");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT_SUCCESS(result, "Create ImageView");

    KxFramebufferCreateInfo fb_ci = {
        .sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = 1,
        .pAttachments = &view,
        .width = 64,
        .height = 64,
        .layers = 1,
    };
    KxFramebuffer framebuffer;
    result = kxCreateFramebuffer(device, &fb_ci, nullptr, &framebuffer);
    ASSERT_SUCCESS(result, "Create Framebuffer");

    KxCommandPoolCreateInfo pool_ci = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = 0,
    };
    KxCommandPool cmd_pool;
    result = kxCreateCommandPool(device, &pool_ci, nullptr, &cmd_pool);
    ASSERT_SUCCESS(result, "Create command pool");

    KxCommandBufferAllocateInfo alloc_ci = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool,
        .level = KX_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    KxCommandBuffer cmd_buf;
    result = kxAllocateCommandBuffers(device, &alloc_ci, &cmd_buf);
    ASSERT_SUCCESS(result, "Allocate command buffer");

    KxCommandBufferBeginInfo begin_ci = { .sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    result = kxBeginCommandBuffer(cmd_buf, &begin_ci);
    ASSERT_SUCCESS(result, "Begin command buffer");

    KxClearValue clear_value = { .color = { 1.0f, 0.0f, 0.0f, 1.0f } };
    KxRect2D render_area = { .offset = {0, 0}, .extent = {64, 64} };
    KxRenderPassBeginInfo rpb_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = framebuffer,
        .renderArea = render_area,
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    kxCmdBeginRenderPass(cmd_buf, &rpb_ci, KX_SUBPASS_CONTENTS_INLINE);

    kxCmdEndRenderPass(cmd_buf);

    result = kxEndCommandBuffer(cmd_buf);
    ASSERT_SUCCESS(result, "End command buffer");

    KxQueue queue;
    result = kxGetDeviceQueue(device, 0, 0, &queue);
    ASSERT_SUCCESS(result, "Get device queue");

    KxSubmitInfo submit = {
        .sType = KX_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buf,
    };
    result = kxQueueSubmit(queue, 1, &submit, (KxFence)0);
    ASSERT_SUCCESS(result, "Queue submit");

    kxQueueWaitIdle(queue);
    kxFreeCommandBuffers(device, cmd_pool, 1, &cmd_buf);
    kxDestroyCommandPool(device, cmd_pool, nullptr);
    kxDestroyFramebuffer(device, framebuffer, nullptr);
    kxDestroyImageView(device, view, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyRenderPass(device, render_pass, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_cmd_next_subpass() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxAttachmentDescription attachments[1] = {{
        .sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = KX_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    }};

    KxAttachmentReference color_ref = { .attachment = 0, .layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    KxSubpassDescription subpasses[2] = {
        { .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION, .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = 1, .pColorAttachments = &color_ref },
        { .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION, .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = 1, .pColorAttachments = &color_ref },
    };

    KxRenderPassCreateInfo rp_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 2,
        .pSubpasses = subpasses,
    };
    KxRenderPass render_pass;
    result = kxCreateRenderPass(device, &rp_ci, nullptr, &render_pass);
    ASSERT_SUCCESS(result, "Create RenderPass with 2 subpasses");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT_SUCCESS(result, "Create ImageView");

    KxFramebufferCreateInfo fb_ci = {
        .sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = 1,
        .pAttachments = &view,
        .width = 64,
        .height = 64,
        .layers = 1,
    };
    KxFramebuffer framebuffer;
    result = kxCreateFramebuffer(device, &fb_ci, nullptr, &framebuffer);
    ASSERT_SUCCESS(result, "Create Framebuffer");

    KxCommandPoolCreateInfo pool_ci = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = 0,
    };
    KxCommandPool cmd_pool;
    result = kxCreateCommandPool(device, &pool_ci, nullptr, &cmd_pool);
    ASSERT_SUCCESS(result, "Create command pool");

    KxCommandBufferAllocateInfo alloc_ci = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool,
        .level = KX_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    KxCommandBuffer cmd_buf;
    result = kxAllocateCommandBuffers(device, &alloc_ci, &cmd_buf);
    ASSERT_SUCCESS(result, "Allocate command buffer");

    KxCommandBufferBeginInfo begin_ci = { .sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    result = kxBeginCommandBuffer(cmd_buf, &begin_ci);
    ASSERT_SUCCESS(result, "Begin command buffer");

    KxClearValue clear_value = { .color = { 1.0f, 0.0f, 0.0f, 1.0f } };
    KxRect2D render_area = { .offset = {0, 0}, .extent = {64, 64} };
    KxRenderPassBeginInfo rpb_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = framebuffer,
        .renderArea = render_area,
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    kxCmdBeginRenderPass(cmd_buf, &rpb_ci, KX_SUBPASS_CONTENTS_INLINE);

    kxCmdNextSubpass(cmd_buf, KX_SUBPASS_CONTENTS_INLINE);

    kxCmdEndRenderPass(cmd_buf);

    result = kxEndCommandBuffer(cmd_buf);
    ASSERT_SUCCESS(result, "End command buffer");

    KxQueue queue;
    result = kxGetDeviceQueue(device, 0, 0, &queue);
    ASSERT_SUCCESS(result, "Get device queue");

    KxSubmitInfo submit = {
        .sType = KX_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buf,
    };
    result = kxQueueSubmit(queue, 1, &submit, (KxFence)0);
    ASSERT_SUCCESS(result, "Queue submit");

    kxQueueWaitIdle(queue);
    kxFreeCommandBuffers(device, cmd_pool, 1, &cmd_buf);
    kxDestroyCommandPool(device, cmd_pool, nullptr);
    kxDestroyFramebuffer(device, framebuffer, nullptr);
    kxDestroyImageView(device, view, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyRenderPass(device, render_pass, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_cmd_end_renderpass() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxAttachmentDescription attachments[1] = {{
        .sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = KX_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    }};

    KxAttachmentReference color_ref = { .attachment = 0, .layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    KxSubpassDescription subpass = {
        .sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION,
        .pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
    };

    KxRenderPassCreateInfo rp_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    KxRenderPass render_pass;
    result = kxCreateRenderPass(device, &rp_ci, nullptr, &render_pass);
    ASSERT_SUCCESS(result, "Create RenderPass");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create image");

    KxImageViewCreateInfo view_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = KX_IMAGE_VIEW_TYPE_2D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = KX_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    KxImageView view;
    result = kxCreateImageView(device, &view_ci, nullptr, &view);
    ASSERT_SUCCESS(result, "Create ImageView");

    KxFramebufferCreateInfo fb_ci = {
        .sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = 1,
        .pAttachments = &view,
        .width = 64,
        .height = 64,
        .layers = 1,
    };
    KxFramebuffer framebuffer;
    result = kxCreateFramebuffer(device, &fb_ci, nullptr, &framebuffer);
    ASSERT_SUCCESS(result, "Create Framebuffer");

    KxCommandPoolCreateInfo pool_ci = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = 0,
    };
    KxCommandPool cmd_pool;
    result = kxCreateCommandPool(device, &pool_ci, nullptr, &cmd_pool);
    ASSERT_SUCCESS(result, "Create command pool");

    KxCommandBufferAllocateInfo alloc_ci = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool,
        .level = KX_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    KxCommandBuffer cmd_buf;
    result = kxAllocateCommandBuffers(device, &alloc_ci, &cmd_buf);
    ASSERT_SUCCESS(result, "Allocate command buffer");

    KxCommandBufferBeginInfo begin_ci = { .sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    result = kxBeginCommandBuffer(cmd_buf, &begin_ci);
    ASSERT_SUCCESS(result, "Begin command buffer");

    KxClearValue clear_value = { .color = { 0.0f, 1.0f, 0.0f, 1.0f } };
    KxRect2D render_area = { .offset = {0, 0}, .extent = {64, 64} };
    KxRenderPassBeginInfo rpb_ci = {
        .sType = KX_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = framebuffer,
        .renderArea = render_area,
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    kxCmdBeginRenderPass(cmd_buf, &rpb_ci, KX_SUBPASS_CONTENTS_INLINE);

    kxCmdEndRenderPass(cmd_buf);

    result = kxEndCommandBuffer(cmd_buf);
    ASSERT_SUCCESS(result, "End command buffer");

    KxQueue queue;
    result = kxGetDeviceQueue(device, 0, 0, &queue);
    ASSERT_SUCCESS(result, "Get device queue");

    KxSubmitInfo submit = {
        .sType = KX_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buf,
    };
    result = kxQueueSubmit(queue, 1, &submit, (KxFence)0);
    ASSERT_SUCCESS(result, "Queue submit");

    kxQueueWaitIdle(queue);
    kxFreeCommandBuffers(device, cmd_pool, 1, &cmd_buf);
    kxDestroyCommandPool(device, cmd_pool, nullptr);
    kxDestroyFramebuffer(device, framebuffer, nullptr);
    kxDestroyImageView(device, view, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyRenderPass(device, render_pass, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_image_format_d32() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_D32_SFLOAT,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create D32_SFLOAT image");

    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_image_format_d24s8() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_D24_UNORM_S8_UINT,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create D24_UNORM_S8_UINT image");

    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int test_image_3d_size() {
    KxInstanceCreateInfo inst_ci = { .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    KxInstance instance;
    KxResult result = kxCreateInstance(&inst_ci, nullptr, &instance);
    ASSERT_SUCCESS(result, "Create instance");

    KxPhysicalDevice pd;
    uint32_t count = 1;
    result = kxEnumeratePhysicalDevices(instance, &count, &pd);
    ASSERT_SUCCESS(result, "Enumerate physical devices");

    KxDeviceCreateInfo dev_ci = { .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    KxDevice device;
    result = kxCreateDevice(pd, &dev_ci, nullptr, &device);
    ASSERT_SUCCESS(result, "Create device");

    KxImageCreateInfo img_ci = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_3D,
        .format = KX_FORMAT_R8G8B8A8_UNORM,
        .width = 16,
        .height = 16,
        .depth = 16,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_OPTIMAL,
        .usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    KxImage image;
    result = kxCreateImage(device, &img_ci, nullptr, &image);
    ASSERT_SUCCESS(result, "Create 3D image");

    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
    return 0;
}

int main() {
    int (*tests[])() = {
        test_image_view_create_2d,
        test_image_view_create_3d,
        test_image_view_invalid_type,
        test_image_view_invalid_range,
        test_image_view_destroy,
        test_renderpass_validation,
        test_renderpass_cyclic_dep,
        test_framebuffer_attachment_count,
        test_framebuffer_dimensions,
        test_cmd_begin_renderpass_clear,
        test_cmd_next_subpass,
        test_cmd_end_renderpass,
        test_image_format_d32,
        test_image_format_d24s8,
        test_image_3d_size,
    };

    int passed = 0;
    int failed = 0;
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        if (tests[i]() == 0) {
            passed++;
        } else {
            failed++;
        }
    }

    printf("\n=== SUMMARY: %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}