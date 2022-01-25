#ifndef _HERO_GFX_INTERNAL_VULKAN_H_
#define _HERO_GFX_INTERNAL_VULKAN_H_

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>

// ===========================================
//
//
// Forward Declarations
//
//
// ===========================================

typedef struct HeroPhysicalDeviceVulkan HeroPhysicalDeviceVulkan;
typedef struct HeroLogicalDeviceVulkan HeroLogicalDeviceVulkan;
typedef struct HeroVertexLayoutVulkan HeroVertexLayoutVulkan;
typedef struct HeroBufferVulkan HeroBufferVulkan;
typedef struct HeroImageVulkan HeroImageVulkan;
typedef struct HeroSamplerVulkan HeroSamplerVulkan;
typedef struct HeroShaderModuleVulkan HeroShaderModuleVulkan;
typedef struct HeroShaderVulkan HeroShaderVulkan;
typedef struct HeroDescriptorPoolVulkan HeroDescriptorPoolVulkan;
typedef struct HeroShaderGlobalsVulkan HeroShaderGlobalsVulkan;
typedef struct HeroRenderPassLayoutVulkan HeroRenderPassLayoutVulkan;
typedef struct HeroRenderPassVulkan HeroRenderPassVulkan;
typedef struct HeroFrameBufferVulkan HeroFrameBufferVulkan;
typedef struct HeroPipelineCacheVulkan HeroPipelineCacheVulkan;
typedef struct HeroPipelineVulkan HeroPipelineVulkan;
typedef struct HeroMaterialVulkan HeroMaterialVulkan;
typedef struct HeroSwapchainVulkan HeroSwapchainVulkan;
/*
typedef struct HeroCommandPoolVulkan HeroCommandPoolVulkan;
typedef struct HeroCommandPoolBufferVulkan HeroCommandPoolBufferVulkan;
*/
typedef struct HeroRenderGraphVulkan HeroRenderGraphVulkan;
typedef struct HeroFrameGraphVulkan HeroFrameGraphVulkan;

#define HERO_OBJECT_TYPE HeroVertexLayoutVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroBufferVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroImageVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroSamplerVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroShaderModuleVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroShaderVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroDescriptorPoolVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroShaderGlobalsVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroRenderPassLayoutVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroRenderPassVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroFrameBufferVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroPipelineCacheVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroPipelineVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroMaterialVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroSwapchainVulkan
#include "object_pool_gen_def.inl"

/*

#define HERO_OBJECT_TYPE HeroCommandPoolVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroCommandPoolBufferVulkan
#include "object_pool_gen_def.inl"

*/

#define HERO_OBJECT_TYPE HeroRenderGraphVulkan
#include "object_pool_gen_def.inl"

#define HERO_OBJECT_TYPE HeroFrameGraphVulkan
#include "object_pool_gen_def.inl"

#if HERO_X11_ENABLE
#define HERO_VULKAN_X11_FN_LIST \
	HERO_VULKAN_FN(vkCreateXlibSurfaceKHR) \
	HERO_VULKAN_FN(vkGetPhysicalDeviceXlibPresentationSupportKHR) \
	/* end */
#else
#define HERO_VULKAN_X11_FN_LIST
#endif // HERO_X11_ENABLE

#define HERO_VULKAN_FN_LIST \
	HERO_VULKAN_FN(vkCreateInstance) \
	HERO_VULKAN_FN(vkGetInstanceProcAddr) \
	HERO_VULKAN_FN(vkGetDeviceProcAddr) \
	HERO_VULKAN_INSTANCE_FN(vkEnumeratePhysicalDevices) \
	HERO_VULKAN_INSTANCE_FN(vkGetPhysicalDeviceProperties) \
	HERO_VULKAN_INSTANCE_FN(vkGetPhysicalDeviceQueueFamilyProperties) \
	HERO_VULKAN_INSTANCE_FN(vkGetPhysicalDeviceFeatures) \
	HERO_VULKAN_INSTANCE_FN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
	HERO_VULKAN_INSTANCE_FN(vkGetPhysicalDeviceSurfacePresentModesKHR) \
	HERO_VULKAN_INSTANCE_FN(vkGetPhysicalDeviceSurfaceFormatsKHR) \
	HERO_VULKAN_INSTANCE_FN(vkGetPhysicalDeviceMemoryProperties) \
	HERO_VULKAN_INSTANCE_FN(vkGetPhysicalDeviceSurfaceSupportKHR) \
	HERO_VULKAN_INSTANCE_FN(vkCreateDevice) \
	HERO_VULKAN_DEVICE_FN(vkGetDeviceQueue) \
	HERO_VULKAN_DEVICE_FN(vkCreateSwapchainKHR) \
	HERO_VULKAN_DEVICE_FN(vkDestroySwapchainKHR) \
	HERO_VULKAN_DEVICE_FN(vkGetSwapchainImagesKHR) \
	HERO_VULKAN_DEVICE_FN(vkAcquireNextImageKHR) \
	HERO_VULKAN_DEVICE_FN(vkCreateRenderPass) \
	HERO_VULKAN_DEVICE_FN(vkDestroyRenderPass) \
	HERO_VULKAN_DEVICE_FN(vkCreateFramebuffer) \
	HERO_VULKAN_DEVICE_FN(vkDestroyFramebuffer) \
	HERO_VULKAN_DEVICE_FN(vkCreateImage) \
	HERO_VULKAN_DEVICE_FN(vkDestroyImage) \
	HERO_VULKAN_DEVICE_FN(vkCreateImageView) \
	HERO_VULKAN_DEVICE_FN(vkDestroyImageView) \
	HERO_VULKAN_DEVICE_FN(vkCreateSampler) \
	HERO_VULKAN_DEVICE_FN(vkDestroySampler) \
	HERO_VULKAN_DEVICE_FN(vkCreateBuffer) \
	HERO_VULKAN_DEVICE_FN(vkDestroyBuffer) \
	HERO_VULKAN_DEVICE_FN(vkGetBufferMemoryRequirements) \
	HERO_VULKAN_DEVICE_FN(vkGetImageMemoryRequirements) \
	HERO_VULKAN_DEVICE_FN(vkAllocateMemory) \
	HERO_VULKAN_DEVICE_FN(vkFreeMemory) \
	HERO_VULKAN_DEVICE_FN(vkBindBufferMemory) \
	HERO_VULKAN_DEVICE_FN(vkBindImageMemory) \
	HERO_VULKAN_DEVICE_FN(vkMapMemory) \
	HERO_VULKAN_DEVICE_FN(vkCreateShaderModule) \
	HERO_VULKAN_DEVICE_FN(vkDestroyShaderModule) \
	HERO_VULKAN_DEVICE_FN(vkCreateDescriptorSetLayout) \
	HERO_VULKAN_DEVICE_FN(vkDestroyDescriptorSetLayout) \
	HERO_VULKAN_DEVICE_FN(vkCreatePipelineLayout) \
	HERO_VULKAN_DEVICE_FN(vkDestroyPipelineLayout) \
	HERO_VULKAN_DEVICE_FN(vkCreatePipelineCache) \
	HERO_VULKAN_DEVICE_FN(vkDestroyPipelineCache) \
	HERO_VULKAN_DEVICE_FN(vkCreateGraphicsPipelines) \
	HERO_VULKAN_DEVICE_FN(vkCreateComputePipelines) \
	HERO_VULKAN_DEVICE_FN(vkDestroyPipeline) \
	HERO_VULKAN_DEVICE_FN(vkCreateDescriptorPool) \
	HERO_VULKAN_DEVICE_FN(vkDestroyDescriptorPool) \
	HERO_VULKAN_DEVICE_FN(vkAllocateDescriptorSets) \
	HERO_VULKAN_DEVICE_FN(vkCreateDescriptorUpdateTemplate) \
	HERO_VULKAN_DEVICE_FN(vkDestroyDescriptorUpdateTemplate) \
	HERO_VULKAN_DEVICE_FN(vkUpdateDescriptorSetWithTemplate) \
	HERO_VULKAN_DEVICE_FN(vkCreateCommandPool) \
	HERO_VULKAN_DEVICE_FN(vkDestroyCommandPool) \
	HERO_VULKAN_DEVICE_FN(vkAllocateCommandBuffers) \
	HERO_VULKAN_DEVICE_FN(vkCreateFence) \
	HERO_VULKAN_DEVICE_FN(vkWaitForFences) \
	HERO_VULKAN_DEVICE_FN(vkResetFences) \
	HERO_VULKAN_DEVICE_FN(vkGetFenceStatus) \
	HERO_VULKAN_DEVICE_FN(vkCreateSemaphore) \
	HERO_VULKAN_DEVICE_FN(vkResetCommandBuffer) \
	HERO_VULKAN_DEVICE_FN(vkBeginCommandBuffer) \
	HERO_VULKAN_DEVICE_FN(vkEndCommandBuffer) \
	HERO_VULKAN_DEVICE_FN(vkCmdSetViewport) \
	HERO_VULKAN_DEVICE_FN(vkCmdSetScissor) \
	HERO_VULKAN_DEVICE_FN(vkCmdBeginRenderPass) \
	HERO_VULKAN_DEVICE_FN(vkCmdBindPipeline) \
	HERO_VULKAN_DEVICE_FN(vkCmdDispatch) \
	HERO_VULKAN_DEVICE_FN(vkCmdBindDescriptorSets) \
	HERO_VULKAN_DEVICE_FN(vkCmdBindVertexBuffers) \
	HERO_VULKAN_DEVICE_FN(vkCmdBindIndexBuffer) \
	HERO_VULKAN_DEVICE_FN(vkCmdDraw) \
	HERO_VULKAN_DEVICE_FN(vkCmdDrawIndexed) \
	HERO_VULKAN_DEVICE_FN(vkCmdEndRenderPass) \
	HERO_VULKAN_DEVICE_FN(vkCmdPipelineBarrier) \
	HERO_VULKAN_DEVICE_FN(vkCmdCopyBuffer) \
	HERO_VULKAN_DEVICE_FN(vkCmdCopyBufferToImage) \
	HERO_VULKAN_DEVICE_FN(vkCmdClearAttachments) \
	HERO_VULKAN_DEVICE_FN(vkCmdPushConstants) \
	HERO_VULKAN_DEVICE_FN(vkCmdClearColorImage) \
	HERO_VULKAN_DEVICE_FN(vkCmdClearDepthStencilImage) \
	HERO_VULKAN_DEVICE_FN(vkQueueSubmit) \
	HERO_VULKAN_DEVICE_FN(vkQueuePresentKHR) \
	HERO_VULKAN_DEVICE_FN(vkDeviceWaitIdle) \
	HERO_VULKAN_DEVICE_FN(vkUpdateDescriptorSets) \
	HERO_VULKAN_X11_FN_LIST \
	/* end */

// ===========================================
//
//
// Types
//
//
// ===========================================

#define HERO_PHYSICAL_DEVICE_VULKAN_QUEUES_CAP 4
struct HeroPhysicalDeviceVulkan {
	HeroPhysicalDevice public_;

	VkPhysicalDevice handle;
	VkPhysicalDeviceMemoryProperties* memory_properties;
	VkQueueFamilyProperties* queue_family_properties;
	VkPresentModeKHR* surface_present_modes;
	U32 queue_families_count;
	U32 surface_formats_count;
	U32 surface_present_modes_count;

	U32 api_version;
	// has compute & transfer. graphics is supported when HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS
	U32 queue_family_idx_uber;
	U32 queue_family_idx_present;        // if HERO_QUEUE_SUPPORT_FLAGS_PRESENT
	U32 queue_family_idx_async_compute;  // if HERO_QUEUE_SUPPORT_FLAGS_ASYNC_COMPUTE
	U32 queue_family_idx_async_transfer; // if HERO_QUEUE_SUPPORT_FLAGS_ASYNC_TRANSFER
};

#define HERO_STACK_ELMT_TYPE VkCommandBuffer
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE VkSemaphore
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE VkSwapchainKHR
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE VkFence
#include "stack_gen.inl"

typedef struct _HeroGfxSubmitVulkan _HeroGfxSubmitVulkan;
struct _HeroGfxSubmitVulkan {
	VkFence fence_render;
};

#define HERO_STACK_ELMT_TYPE _HeroGfxSubmitVulkan
#include "stack_gen.inl"

typedef union _HeroGfxStagedUpdateData _HeroGfxStagedUpdateData;
union _HeroGfxStagedUpdateData {
	struct {
		VkBuffer dst_buffer;
		U64 dst_offset;
		U64 size;
	} buffer;
	struct {
		VkImage dst_image;
		HeroImageFormat format;
		HeroImageArea area;
		VkImageLayout layout;
	} image;
};

typedef struct _HeroGfxStagedUpdate _HeroGfxStagedUpdate;
struct _HeroGfxStagedUpdate {
	bool is_image;
	VkBuffer src_buffer;
	U64 src_offset;
	_HeroGfxStagedUpdateData data;
};

#define HERO_STACK_ELMT_TYPE _HeroGfxStagedUpdate
#include "stack_gen.inl"

typedef struct _HeroGfxStagingBufferVulkan _HeroGfxStagingBufferVulkan;
struct _HeroGfxStagingBufferVulkan {
	VkBuffer buffer;
	VkDeviceMemory device_memory;
	U64 size;
	U64 cap;
	void* mapped_memory;
};

#define HERO_STACK_ELMT_TYPE _HeroGfxStagingBufferVulkan
#include "stack_gen.inl"

typedef struct _HeroGfxStagingBufferSysVulkan _HeroGfxStagingBufferSysVulkan;
struct _HeroGfxStagingBufferSysVulkan {
	HeroStack(_HeroGfxStagingBufferVulkan) free_buffers;
	HeroStack(_HeroGfxStagingBufferVulkan) free_buffers_in_use; // a VK_NULL_HANDLE is used to seperate submits
	HeroStack(_HeroGfxStagedUpdate)        staged_updates;
	HeroStack(VkCommandBuffer)             free_command_buffers;
	HeroStack(VkCommandBuffer)             free_command_buffers_in_use; // a VK_NULL_HANDLE is used to seperate submits
	VkCommandPool                          command_pool;
	U32                                    used_buffers_count;
};

typedef union HeroDescriptorUpdateDataVulkan HeroDescriptorUpdateDataVulkan;
union HeroDescriptorUpdateDataVulkan {
	VkDescriptorImageInfo image;
	VkDescriptorBufferInfo buffer;
};

#define HERO_VULKAN_OBJECT_DEALLOC_IS_HEADER 0x8000000000000000 // we use the high bit to say it's a header since pointers never use the high bit.
#define HERO_VULKAN_OBJECT_DEALLOC_HEADER(object_type, objects_count) \
	HERO_VULKAN_OBJECT_DEALLOC_IS_HEADER | \
	((U64)((objects_count) & 0xffff) << 32) | \
	((U64)((object_type) & 0xffffffff) << 0) \
	/* end */

#define HERO_VULKAN_OBJECT_DEALLOC_HEADER_OBJECT_TYPE(dealloc) \
	(((dealloc).header >> 0) & 0xffffffff)

#define HERO_VULKAN_OBJECT_DEALLOC_HEADER_ENTRIES_COUNT(dealloc) \
	(((dealloc).header >> 32) & 0xffff)

typedef union HeroVulkanObjectDealloc HeroVulkanObjectDealloc;
union HeroVulkanObjectDealloc {
	U64 header;
	VkImage image;
	VkImageView image_view;
	VkBuffer buffer;
	VkDescriptorPool descriptor_pool;
	VkCommandPool command_pool;
	VkRenderPass render_pass;
	VkFramebuffer frame_buffer;
};

#define HERO_STACK_ELMT_TYPE HeroVulkanObjectDealloc
#include "stack_gen.inl"

struct HeroLogicalDeviceVulkan {
	HeroLogicalDevice public_;
	HeroObjectPool(HeroBufferVulkan)              buffer_pool;
	HeroObjectPool(HeroImageVulkan)               image_pool;
	HeroObjectPool(HeroSamplerVulkan)             sampler_pool;
	HeroObjectPool(HeroShaderModuleVulkan)        shader_module_pool;
	HeroObjectPool(HeroShaderVulkan)              shader_pool;
	HeroObjectPool(HeroDescriptorPoolVulkan)      descriptor_pool_pool;
	HeroObjectPool(HeroShaderGlobalsVulkan)       shader_globals_pool;
	HeroObjectPool(HeroRenderPassLayoutVulkan)    render_pass_layout_pool;
	HeroObjectPool(HeroRenderPassVulkan)          render_pass_pool;
	HeroObjectPool(HeroFrameBufferVulkan)         frame_buffer_pool;
	HeroObjectPool(HeroPipelineCacheVulkan)       pipeline_cache_pool;
	HeroObjectPool(HeroPipelineVulkan)            pipeline_pool;
	HeroObjectPool(HeroMaterialVulkan)            material_pool;
	HeroObjectPool(HeroSwapchainVulkan)           swapchain_pool;
	/*
	HeroObjectPool(HeroCommandPoolVulkan)         command_pool_pool;
	*/
	HeroObjectPool(HeroRenderGraphVulkan)         render_graph_pool;
	HeroObjectPool(HeroFrameGraphVulkan)          frame_graph_pool;

	VkDevice                handle;
	U32                     queue_family_idx_uber;
	U32                     queue_family_idx_present;
	U32                     queue_family_idx_async_compute;
	U32                     queue_family_idx_async_transfer;
	VkQueue                 queue_uber;
	VkQueue                 queue_present;
	VkQueue                 queue_async_compute;
	VkQueue                 queue_async_transfer;
	VkSemaphore             semaphore_present;

	VkSampler   null_sampler;
	VkImage     null_image_1d;
	VkImage     null_image_2d;
	VkImage     null_image_3d;
	VkImageView null_image_view_1d;
	VkImageView null_image_view_2d;
	VkImageView null_image_view_3d;
	VkBuffer    null_buffer;
	VkDescriptorSetLayout null_descriptor_set_layout; // a descriptor set layout with no descriptors


	U32                                objects_to_deallocate_prev_idx;
	VkObjectType                       objects_to_deallocate_prev_object_type;
	HeroStack(HeroVulkanObjectDealloc) objects_to_deallocate; // a VK_NULL_HANDLE is used to seperate submits
	HeroStack(VkCommandBuffer)         submit_command_buffers;
	HeroStack(VkSemaphore)             submit_render_semaphores;
	HeroStack(VkSwapchainKHR)          submit_swapchains;
	HeroStack(U32)                     submit_swapchain_image_indices;
	HeroStack(VkFence)                 submit_free_fences;
	HeroStack(_HeroGfxSubmitVulkan)    submits;

	U32 descriptor_caps[HERO_VULKAN_DESCRIPTOR_BINDING_COUNT];

	_HeroGfxStagingBufferSysVulkan staging_buffer_sys;

	//
	// make fields for all of the logical device function pointers
#define HERO_VULKAN_FN(NAME)
#define HERO_VULKAN_INSTANCE_FN(NAME)
#define HERO_VULKAN_DEVICE_FN(NAME) PFN_##NAME NAME;
		HERO_VULKAN_FN_LIST
#undef HERO_VULKAN_FN
#undef HERO_VULKAN_INSTANCE_FN
#undef HERO_VULKAN_DEVICE_FN
};

struct HeroVertexLayoutVulkan {
	HeroObjectHeader                   header;
	HeroVertexLayout                   public_;

	VkVertexInputBindingDescription*   bindings;
	VkVertexInputAttributeDescription* attribs;
	U16                                attribs_count;
};

struct HeroBufferVulkan {
	HeroObjectHeader header;
	HeroBuffer public_;

	VkBuffer handle;
	VkDeviceMemory device_memory;
};

struct HeroImageVulkan {
	HeroObjectHeader header;
	HeroImage public_;

	VkImage handle;
	VkImageView view_handle;
	VkDeviceMemory device_memory;
};

struct HeroSamplerVulkan {
	HeroObjectHeader header;
	HeroSampler public_;

	VkSampler handle;
};

struct HeroShaderModuleVulkan {
	HeroObjectHeader header;
	HeroShaderModule public_;

	VkShaderModule handle;
};

struct HeroShaderVulkan {
	HeroObjectHeader header;
	HeroShader public_;

	VkPipelineLayout pipeline_layout;
	VkPipeline compute_pipeline;
	VkDescriptorSetLayout descriptor_set_layouts[HERO_GFX_DESCRIPTOR_SET_COUNT];
	VkDescriptorUpdateTemplate descriptor_update_templates[HERO_GFX_DESCRIPTOR_SET_COUNT];
	U32* descriptor_binding_update_data_indices;
	U32 descriptors_counts[HERO_GFX_DESCRIPTOR_SET_COUNT];
	U32 descriptors_count;
};

typedef struct _HeroDescriptorPoolVulkan _HeroDescriptorPoolVulkan;
struct _HeroDescriptorPoolVulkan {
	VkDescriptorPool handle;
	U32* layout_free_counts; // count = HeroDescriptorPoolVulkan.layouts_count
};

#define HERO_STACK_ELMT_TYPE _HeroDescriptorPoolVulkan
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE VkDescriptorSet
#include "stack_gen.inl"

typedef struct _HeroDescriptorSetLayoutAuxVulkan _HeroDescriptorSetLayoutAuxVulkan;
struct _HeroDescriptorSetLayoutAuxVulkan {
	HeroStack(VkDescriptorSet) free_sets;
	HeroStack(VkDescriptorSet) free_sets_in_use; // for descriptor sets that are still being used, a VK_NULL_HANDLE is used to seperate submits
	HeroGfxFrameIdx last_frame_idx_for_free_sets_in_use;
	U32 free_pool_idx;
	U32 cap_per_pool;
};

struct HeroDescriptorPoolVulkan {
	HeroObjectHeader header;

	HeroStack(_HeroDescriptorPoolVulkan) pools;
	_HeroDescriptorSetLayoutAuxVulkan*   layout_auxs;
	VkDescriptorSetLayout*               layouts;
	VkDescriptorPoolSize*                pool_sizes;
	U32                                  layouts_count;
	U32                                  pool_sizes_count;
	U32                                  pool_max_sets;
	U32                                  reset_counter;
};

typedef struct HeroDescriptorSetVulkan HeroDescriptorSetVulkan;
struct HeroDescriptorSetVulkan {
	VkDescriptorSet handle;
	VkDescriptorSetLayout layout;
	VkDescriptorUpdateTemplate update_template;
	HeroDescriptorUpdateDataVulkan* update_data;
	U32* binding_update_data_indices;
	U32 descriptors_count;
	HeroGfxFrameIdx last_submitted_frame_idx;
};

struct HeroShaderGlobalsVulkan {
	HeroObjectHeader header;
	HeroShaderGlobals public_;

	HeroDescriptorSetVulkan descriptor_set;
};

struct HeroRenderPassLayoutVulkan {
	HeroObjectHeader header;
	HeroRenderPassLayout public_;

	VkRenderPass handle;
};

struct HeroRenderPassVulkan {
	HeroObjectHeader header;
	HeroRenderPass public_;

	VkRenderPass handle;
};

struct HeroFrameBufferVulkan {
	HeroObjectHeader header;
	HeroFrameBuffer public_;

	VkFramebuffer handle;
};

struct HeroPipelineCacheVulkan {
	HeroObjectHeader header;

	VkPipelineCache handle;
};

struct HeroPipelineVulkan {
	HeroObjectHeader header;
	HeroPipeline public_;

	VkPipeline handle;
};

struct HeroMaterialVulkan {
	HeroObjectHeader header;
	HeroMaterial public_;

	HeroDescriptorSetVulkan descriptor_set;
};

struct HeroSwapchainVulkan {
	HeroObjectHeader header;
	HeroSwapchain public_;

	VkSwapchainKHR handle;
	VkSemaphore semaphore_render;
	VkSurfaceKHR surface;
	U32 image_idx;
	VkPresentModeKHR present_mode;
	VkImageView* image_views;
};

#define HERO_VULKAN_PUSH_CONSTANTS_WORDS_CAP 32

typedef struct HeroCommandRecorderVulkan HeroCommandRecorderVulkan;
struct HeroCommandRecorderVulkan {
	VkCommandBuffer command_buffer;
	HeroPipelineId bound_pipeline_graphics;
	HeroShaderId bound_pipeline_compute;
	VkDescriptorSet bound_graphics_descriptor_sets[HERO_GFX_DESCRIPTOR_SET_COUNT];
	VkDescriptorSet bound_compute_descriptor_set;
	VkBuffer bound_vertex_buffers[HERO_BUFFER_BINDINGS_CAP];
	U32 push_constants[HERO_VULKAN_PUSH_CONSTANTS_WORDS_CAP];
	U64 bound_vertex_buffer_offsets[HERO_BUFFER_BINDINGS_CAP];
	U32 vertex_buffer_binding_start;
	U32 vertex_buffer_binding_end;
	U32 vertices_start_idx;
	U32 indices_start_idx;
	U32 instances_start_idx;
	U32 instances_count;
	U32 group_count_x;
	U32 group_count_y;
	U32 group_count_z;
};

/*

struct HeroCommandPoolBufferVulkan {
	HeroObjectHeader  header;
	VkCommandBuffer   command_buffer;
	bool              is_static;
};

struct HeroCommandPoolVulkan {
	HeroObjectHeader           header;
	VkCommandPool              command_pool;
	VkCommandPool              command_pool_static;
	HeroCommandRecorderVulkan  command_recorder; // the command buffer that is currently being built

	HeroStack(VkCommandBuffer)                  free_buffers;
	HeroStack(VkCommandBuffer)                  free_buffers_in_use;
	HeroObjectPool(HeroCommandPoolBufferVulkan) command_buffer_pool;
};
*/

struct HeroRenderGraphVulkan {
	HeroObjectHeader header;
	HeroRenderGraph  public_;
};

typedef struct HeroExecutionUnitVulkan HeroExecutionUnitVulkan;
struct HeroExecutionUnitVulkan {
	VkSemaphore          cross_frame_semaphore;
	VkPipelineStageFlags src_stage_flags;
	VkPipelineStageFlags dst_stage_flags;
	U16                  resource_barriers_start_idx; // for HeroFrameGraphVulkan.{execution_units_image_barriers, execution_units_buffer_barriers}
	U16                  image_barriers_count;
	U16                  buffer_barriers_count;
};

typedef struct HeroPassVulkan HeroPassVulkan;
struct HeroPassVulkan {
	VkRenderPass    render_pass; // VK_NULL_HANDLE when it's a compute pass
	HeroSwapchainId swapchain_id;
	U16             clear_count;
	U32             frame_buffer_image_width;
	U32             frame_buffer_image_height;
};

typedef struct HeroFrameGraphVulkanActiveFrame HeroFrameGraphVulkanActiveFrame;
struct HeroFrameGraphVulkanActiveFrame {
	VkDeviceMemory  images_device_memory;
	VkDeviceMemory  buffers_device_memory;
	VkDescriptorSet descriptor_set;
};

typedef union HeroVulkanResource HeroVulkanResource;
union HeroVulkanResource {
	struct {
		VkImage      image;
		VkImageView  image_view;
	};
	VkBuffer buffer;
};

struct HeroFrameGraphVulkan {
	HeroObjectHeader                   header;
	HeroFrameGraph                     public_;
	HeroPassVulkan*                    passes;
	VkDeviceMemory                     persistent_images_device_memory;
	VkDeviceMemory                     persistent_buffers_device_memory;
	HeroVulkanResource*                persistent_vulkan_resources;
	HeroVulkanResource*                active_frames_vulkan_resources; // array[active_frame_idx][physical_resource_idx]
	VkFramebuffer*                     active_frames_frame_buffers;    // array[active_frame_idx][pass_enum]
	VkCommandBuffer*                   active_frames_command_buffers;  // array[active_frame_idx][pass_enum]
	HeroFrameGraphVulkanActiveFrame*   active_frames;
	U16*                               execution_units_image_barrier_physical_resource_indices;  // cap = public_.resources_cap
	VkImageMemoryBarrier*              execution_units_image_barriers;                           // cap = public_.resources_cap
	U16*                               execution_units_buffer_barrier_physical_resource_indices; // cap = public_.resources_cap
	VkBufferMemoryBarrier*             execution_units_buffer_barriers;                          // cap = public_.resources_cap
	HeroExecutionUnitVulkan*           execution_units;                                          // cap = public_.execution_units_count
	VkCommandPool                      command_pool_transient_reset;
	VkDescriptorSetLayout              descriptor_set_layout;
	VkDescriptorPool                   descriptor_pool;
	U16                                num_frames_to_descriptor_sets;
};

#define HERO_OBJECT_ID_TYPE HeroVertexLayoutId
#define HERO_OBJECT_TYPE HeroVertexLayoutVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroBufferId
#define HERO_OBJECT_TYPE HeroBufferVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroImageId
#define HERO_OBJECT_TYPE HeroImageVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroSamplerId
#define HERO_OBJECT_TYPE HeroSamplerVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroShaderModuleId
#define HERO_OBJECT_TYPE HeroShaderModuleVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroShaderId
#define HERO_OBJECT_TYPE HeroShaderVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroDescriptorPoolId
#define HERO_OBJECT_TYPE HeroDescriptorPoolVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroShaderGlobalsId
#define HERO_OBJECT_TYPE HeroShaderGlobalsVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroRenderPassLayoutId
#define HERO_OBJECT_TYPE HeroRenderPassLayoutVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroRenderPassId
#define HERO_OBJECT_TYPE HeroRenderPassVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroFrameBufferId
#define HERO_OBJECT_TYPE HeroFrameBufferVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroPipelineCacheId
#define HERO_OBJECT_TYPE HeroPipelineCacheVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroPipelineId
#define HERO_OBJECT_TYPE HeroPipelineVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroMaterialId
#define HERO_OBJECT_TYPE HeroMaterialVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroSwapchainId
#define HERO_OBJECT_TYPE HeroSwapchainVulkan
#include "object_pool_gen_impl.inl"

/*

#define HERO_OBJECT_ID_TYPE HeroCommandPoolId
#define HERO_OBJECT_TYPE HeroCommandPoolVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroCommandPoolBufferId
#define HERO_OBJECT_TYPE HeroCommandPoolBufferVulkan
#include "object_pool_gen_impl.inl"

*/

#define HERO_OBJECT_ID_TYPE HeroRenderGraphId
#define HERO_OBJECT_TYPE HeroRenderGraphVulkan
#include "object_pool_gen_impl.inl"

#define HERO_OBJECT_ID_TYPE HeroFrameGraphId
#define HERO_OBJECT_TYPE HeroFrameGraphVulkan
#include "object_pool_gen_impl.inl"

// ===========================================
//
//
// Backend
//
//
// ===========================================

HeroResult _hero_vulkan_init(HeroGfxSysSetup* setup);

HeroResult _hero_vulkan_physical_device_surface_image_formats_supported(HeroPhysicalDevice* physical_device, HeroSurface surface, HeroImageFormat* formats, U32 formats_count);

HeroResult _hero_vulkan_logical_device_init(HeroPhysicalDevice* physical_device, HeroLogicalDeviceSetup* setup, HeroLogicalDevice** out);
HeroResult _hero_vulkan_logical_device_deinit(HeroLogicalDevice* ldev);

HeroResult _hero_vulkan_logical_device_frame_start(HeroLogicalDevice* ldev);

HeroResult _hero_vulkan_logical_device_queue_transfer(HeroLogicalDevice* ldev);
/*
HeroResult _hero_vulkan_logical_device_queue_command_buffers(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandPoolBufferId* command_pool_buffer_ids, U32 command_pool_buffers_count);
*/
HeroResult _hero_vulkan_logical_device_submit(HeroLogicalDevice* ldev, HeroSwapchainId* swapchain_ids, U32 swapchains_count);

HeroResult _hero_vulkan_stage_buffer_update(HeroLogicalDeviceVulkan* ldev_vulkan, bool is_image, _HeroGfxStagedUpdateData* data, void** destination_out);

HeroResult _hero_vulkan_vertex_layout_register(HeroVertexLayout* vl, HeroVertexLayoutId* id_out, HeroVertexLayout** out);
HeroResult _hero_vulkan_vertex_layout_deregister(HeroVertexLayoutId id, HeroVertexLayout* vertex_layout);
HeroResult _hero_vulkan_vertex_layout_get(HeroVertexLayoutId id, HeroVertexLayout** out);

HeroResult _hero_vulkan_buffer_init(HeroLogicalDevice* ldev, HeroBufferSetup* setup, HeroBufferId* id_out, HeroBuffer** out);
HeroResult _hero_vulkan_buffer_deinit(HeroLogicalDevice* ldev, HeroBufferId id, HeroBuffer* buffer);
HeroResult _hero_vulkan_buffer_get(HeroLogicalDevice* ldev, HeroBufferId id, HeroBuffer** out);
HeroResult _hero_vulkan_buffer_resize(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 elmts_count);
HeroResult _hero_vulkan_buffer_map(HeroLogicalDevice* ldev, HeroBuffer* buffer, void** addr_out);
HeroResult _hero_vulkan_buffer_read(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 start_idx, Uptr elmts_count, void* destination);
HeroResult _hero_vulkan_buffer_write(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 start_idx, Uptr elmts_count, void** destination_out);

HeroResult _hero_vulkan_image_init(HeroLogicalDevice* ldev, HeroImageSetup* setup, HeroImageId* id_out, HeroImage** out);
HeroResult _hero_vulkan_image_deinit(HeroLogicalDevice* ldev, HeroImageId id, HeroImage* image);
HeroResult _hero_vulkan_image_get(HeroLogicalDevice* ldev, HeroImageId id, HeroImage** out);
HeroResult _hero_vulkan_image_resize(HeroLogicalDevice* ldev, HeroImage* image, U32 width, U32 height, U32 depth, U32 mip_levels, U32 array_layers_count);
HeroResult _hero_vulkan_image_map(HeroLogicalDevice* ldev, HeroImage* image, void** addr_out);
HeroResult _hero_vulkan_image_read(HeroLogicalDevice* ldev, HeroImage* image, HeroImageArea* area, void* destination);

HeroResult _hero_vulkan_sampler_init(HeroLogicalDevice* ldev, HeroSamplerSetup* setup, HeroSamplerId* id_out, HeroSampler** out);
HeroResult _hero_vulkan_sampler_deinit(HeroLogicalDevice* ldev, HeroSamplerId id, HeroSampler* sampler);
HeroResult _hero_vulkan_sampler_get(HeroLogicalDevice* ldev, HeroSamplerId id, HeroSampler** out);

HeroResult _hero_vulkan_shader_metadata_calculate(HeroLogicalDevice* ldev, HeroShaderMetadataSetup* setup, HeroShaderMetadata** out);

HeroResult _hero_vulkan_shader_module_init(HeroLogicalDevice* ldev, HeroShaderModuleSetup* setup, HeroShaderModuleId* id_out, HeroShaderModule** out);
HeroResult _hero_vulkan_shader_module_deinit(HeroLogicalDevice* ldev, HeroShaderModuleId id, HeroShaderModule* shader_module);
HeroResult _hero_vulkan_shader_module_get(HeroLogicalDevice* ldev, HeroShaderModuleId id, HeroShaderModule** out);

HeroResult _hero_vulkan_shader_init(HeroLogicalDevice* ldev, HeroShaderSetup* setup, HeroShaderId* id_out, HeroShader** out);
HeroResult _hero_vulkan_shader_deinit(HeroLogicalDevice* ldev, HeroShaderId id, HeroShader* shader);
HeroResult _hero_vulkan_shader_get(HeroLogicalDevice* ldev, HeroShaderId id, HeroShader** out);

HeroResult _hero_vulkan_descriptor_pool_init(HeroLogicalDevice* ldev, HeroDescriptorPoolSetup* setup, HeroDescriptorPoolId* id_out);
HeroResult _hero_vulkan_descriptor_pool_deinit(HeroLogicalDevice* ldev, HeroDescriptorPoolId id);
HeroResult _hero_vulkan_descriptor_pool_reset(HeroLogicalDevice* ldev, HeroDescriptorPoolId id);

HeroResult _hero_vulkan_shader_globals_init(HeroLogicalDevice* ldev, HeroShaderGlobalsSetup* setup, HeroShaderGlobalsId* id_out, HeroShaderGlobals** out);
HeroResult _hero_vulkan_shader_globals_deinit(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, HeroShaderGlobals* shader_globals);
HeroResult _hero_vulkan_shader_globals_get(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, HeroShaderGlobals** out);
HeroResult _hero_vulkan_shader_globals_set_descriptor(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data);

HeroResult _hero_vulkan_render_pass_layout_init(HeroLogicalDevice* ldev, HeroRenderPassLayoutSetup* setup, HeroRenderPassLayoutId* id_out, HeroRenderPassLayout** out);
HeroResult _hero_vulkan_render_pass_layout_deinit(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id, HeroRenderPassLayout* render_pass_layout);
HeroResult _hero_vulkan_render_pass_layout_get(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id, HeroRenderPassLayout** out);

HeroResult _hero_vulkan_render_pass_init(HeroLogicalDevice* ldev, HeroRenderPassSetup* setup, HeroRenderPassLayout* render_pass_layout, HeroRenderPassId* id_out, HeroRenderPass** out);
HeroResult _hero_vulkan_render_pass_deinit(HeroLogicalDevice* ldev, HeroRenderPassId id, HeroRenderPass* render_pass);
HeroResult _hero_vulkan_render_pass_get(HeroLogicalDevice* ldev, HeroRenderPassId id, HeroRenderPass** out);

HeroResult _hero_vulkan_frame_buffer_init(HeroLogicalDevice* ldev, HeroFrameBufferSetup* setup, HeroFrameBufferId* id_out, HeroFrameBuffer** out);
HeroResult _hero_vulkan_frame_buffer_deinit(HeroLogicalDevice* ldev, HeroFrameBufferId id, HeroFrameBuffer* frame_buffer);
HeroResult _hero_vulkan_frame_buffer_get(HeroLogicalDevice* ldev, HeroFrameBufferId id, HeroFrameBuffer** out);

HeroResult _hero_vulkan_pipeline_cache_init(HeroLogicalDevice* ldev, HeroPipelineCacheSetup* setup, HeroPipelineCacheId* id_out);
HeroResult _hero_vulkan_pipeline_cache_deinit(HeroLogicalDevice* ldev, HeroPipelineCacheId id);

HeroResult _hero_vulkan_pipeline_graphics_init(HeroLogicalDevice* ldev, HeroPipelineGraphicsSetup* setup, HeroPipelineId* id_out, HeroPipeline** out);
HeroResult _hero_vulkan_pipeline_deinit(HeroLogicalDevice* ldev, HeroPipelineId id, HeroPipeline* pipeline);
HeroResult _hero_vulkan_pipeline_get(HeroLogicalDevice* ldev, HeroPipelineId id, HeroPipeline** out);

HeroResult _hero_vulkan_material_init(HeroLogicalDevice* ldev, HeroMaterialSetup* setup, HeroMaterialId* id_out, HeroMaterial** out);
HeroResult _hero_vulkan_material_deinit(HeroLogicalDevice* ldev, HeroMaterialId id, HeroMaterial* material);
HeroResult _hero_vulkan_material_get(HeroLogicalDevice* ldev, HeroMaterialId id, HeroMaterial** out);
HeroResult _hero_vulkan_material_set_descriptor(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data);

HeroResult _hero_vulkan_swapchain_init(HeroLogicalDevice* ldev, HeroSwapchainSetup* setup, HeroSwapchainId* id_out, HeroSwapchain** out);
HeroResult _hero_vulkan_swapchain_deinit(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain* swapchain);
HeroResult _hero_vulkan_swapchain_get(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain** out);
HeroResult _hero_vulkan_swapchain_next_image(HeroLogicalDevice* ldev, HeroSwapchain* swapchain, U32* next_image_idx_out);

/*

HeroResult _hero_vulkan_command_pool_init(HeroLogicalDevice* ldev, HeroCommandPoolSetup* setup, HeroCommandPoolId* id_out);
HeroResult _hero_vulkan_command_pool_deinit(HeroLogicalDevice* ldev, HeroCommandPoolId id);
HeroResult _hero_vulkan_command_pool_reset(HeroLogicalDevice* ldev, HeroCommandPoolId id);

HeroResult _hero_vulkan_command_buffer_deinit(HeroLogicalDevice* ldev, HeroCommandPoolId pool_id, HeroCommandPoolBufferId buffer_id);
HeroResult _hero_vulkan_command_recorder_start(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, bool is_static, HeroCommandRecorder** out);
HeroResult _hero_vulkan_command_recorder_end(HeroCommandRecorder* command_recorder, HeroCommandPoolBufferId* id_out);

HeroResult _hero_vulkan_cmd_render_pass_start(HeroCommandRecorder* command_recorder, HeroRenderPassId render_pass_id, HeroFrameBufferId frame_buffer_id, HeroViewport* viewport, HeroUAabb* scissor);
HeroResult _hero_vulkan_cmd_render_pass_end(HeroCommandRecorder* command_recorder);

HeroResult _hero_vulkan_cmd_draw_start(HeroCommandRecorder* command_recorder, HeroMaterialId material_id);
HeroResult _hero_vulkan_cmd_draw_end_vertexed(HeroCommandRecorder* command_recorder, U32 vertices_start_idx, U32 vertices_count);
HeroResult _hero_vulkan_cmd_draw_end_indexed(HeroCommandRecorder* command_recorder, HeroBufferId index_buffer_id, U32 indices_start_idx, U32 indices_count, U32 vertices_start_idx);

HeroResult _hero_vulkan_cmd_draw_set_vertex_buffer(HeroCommandRecorder* command_recorder, HeroBufferId buffer_id, U32 binding, U64 offset);
HeroResult _hero_vulkan_cmd_draw_set_push_constants(HeroCommandRecorder* command_recorder, void* data, U32 offset, U32 size);
HeroResult _hero_vulkan_cmd_draw_set_instances(HeroCommandRecorder* command_recorder, U32 instances_start_idx, U32 instances_count);
HeroResult _hero_vulkan_cmd_compute_dispatch(HeroCommandRecorder* command_recorder, HeroShaderId compute_shader_id, HeroShaderGlobalsId shader_globals_id, U32 group_count_x, U32 group_count_y, U32 group_count_z);
*/

HeroResult _hero_vulkan_render_graph_init(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroRenderGraphId* id_out, HeroRenderGraph** out);
HeroResult _hero_vulkan_render_graph_deinit(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph* render_pass);
HeroResult _hero_vulkan_render_graph_get(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph** out);

HeroResult _hero_vulkan_frame_graph_init(HeroLogicalDevice* ldev, HeroFrameGraphSetup* setup, HeroFrameGraphId* id_out, HeroFrameGraph** out);
HeroResult _hero_vulkan_frame_graph_deinit(HeroLogicalDevice* ldev, HeroFrameGraphId id, HeroFrameGraph* frame_graph);
HeroResult _hero_vulkan_frame_graph_get(HeroLogicalDevice* ldev, HeroFrameGraphId id, HeroFrameGraph** out);
HeroResult _hero_vulkan_frame_graph_update(HeroLogicalDevice* ldev, HeroFrameGraph* frame_graph);
HeroResult _hero_vulkan_frame_graph_record_pass_start(HeroLogicalDevice* ldev, HeroFrameGraph* frame_graph, HeroPass* pass, _HeroCommandRecorder* command_recorder);
HeroResult _hero_vulkan_frame_graph_record_pass_end(_HeroCommandRecorder* command_recorder);
HeroResult _hero_vulkan_frame_graph_submit(HeroLogicalDevice* ldev, HeroFrameGraph* frame_graph);

void _hero_vulkan_cmd_draw_start(HeroCommandRecorder* command_recorder, HeroPipelineId pipeline_id);
void _hero_vulkan_cmd_draw_end_vertexed(HeroCommandRecorder* command_recorder, U32 vertices_count);
void _hero_vulkan_cmd_draw_end_indexed(HeroCommandRecorder* command_recorder, HeroPassResource* index_buffer_resource, HeroIndexType index_type, U32 indices_count);

void _hero_vulkan_cmd_draw_set_vertex_buffer(HeroCommandRecorder* command_recorder, HeroPassResource* vertex_buffer_resource, U32 binding, U64 offset);
void _hero_vulkan_cmd_draw_set_vertices_start_idx(HeroCommandRecorder* command_recorder, U32 vertices_start_idx);
void _hero_vulkan_cmd_draw_set_instances(HeroCommandRecorder* command_recorder, U32 instances_start_idx, U32 instances_count);

void _hero_vulkan_cmd_compute_dispatch_start(HeroCommandRecorder* command_recorder, HeroShaderId compute_shader_id, U32 group_count_x, U32 group_count_y, U32 group_count_z);
void _hero_vulkan_cmd_compute_dispatch_end(HeroCommandRecorder* command_recorder);

void _hero_vulkan_cmd_add_sampler(HeroCommandRecorder* command_recorder, U32 binding, HeroSamplerId sampler_id);
void _hero_vulkan_cmd_add_image(HeroCommandRecorder* command_recorder, U32 binding, HeroPassResource* resource);
void _hero_vulkan_cmd_add_buffer(HeroCommandRecorder* command_recorder, U32 binding, HeroPassResource* resource);

typedef struct HeroGfxSysVulkan HeroGfxSysVulkan;
struct HeroGfxSysVulkan {
	HeroDLL dll;
	VkInstance instance;
	HeroObjectPool(HeroVertexLayoutVulkan) vertex_layout_pool;
};

extern HeroGfxSysVulkan hero_gfx_sys_vulkan;

#endif // _HERO_GFX_INTERNAL_VULKAN_H_
