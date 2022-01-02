#ifndef _HERO_GFX_H_
#define _HERO_GFX_H_

#define HERO_GFX_VULKAN_DEBUG 0
#define HERO_BUFFER_BINDINGS_CAP 3
#define HERO_VULKAN_STAGING_BUFFER_MIN_SIZE (64 * 1024 * 1024)

typedef struct HeroGfxSysSetup HeroGfxSysSetup;

// ===========================================
//
//
// General
//
//
// ===========================================

typedef U8 HeroGfxBackendType;
enum {
#if HERO_VULKAN_ENABLE
	HERO_GFX_BACKEND_TYPE_VULKAN,
#endif // HERO_VULKAN_ENABLE
};

typedef U8 HeroGfxDisplayManagerType;
enum {
	HERO_GFX_DISPLAY_MANAGER_WINDOWS,
	HERO_GFX_DISPLAY_MANAGER_MACOS,
	HERO_GFX_DISPLAY_MANAGER_IOS,
	HERO_GFX_DISPLAY_MANAGER_XLIB,
	HERO_GFX_DISPLAY_MANAGER_XCB,
	HERO_GFX_DISPLAY_MANAGER_WAYLAND,
	HERO_GFX_DISPLAY_MANAGER_ANDROID,
};

enum {
	HERO_ERROR_GFX_OUT_OF_MEMORY_DEVICE = HERO_ERROR_GFX_START,

	HERO_ERROR_GFX_QUEUE_SUPPORT_INCOMPATIBLE,
	HERO_ERROR_GFX_VERTEX_BINDING_DOES_NOT_EXIST,
	HERO_ERROR_GFX_ELMT_SIZE_CANNOT_BE_ZERO,
	HERO_ERROR_GFX_BINDING_SIZE_MISMATCH,
	HERO_ERROR_GFX_BUFFER_ELMTS_COUNT_CANNOT_BE_ZERO,

	HERO_ERROR_GFX_EXPECTED_VERTEX_BUFFER,
	HERO_ERROR_GFX_EXPECTED_INDEX_BUFFER,

	HERO_ERROR_GFX_VERTEX_ARRAY_VERTEX_BUFFER_IDS_CANNOT_BE_ZERO,
	HERO_ERROR_GFX_VERTEX_ARRAY_VERTEX_LAYOUT_ID_CANNOT_BE_NULL,

	HERO_ERROR_GFX_DESCRIPTOR_POOL_ADVISED_POOL_COUNT_CANNOT_BE_ZERO,

	HERO_ERROR_GFX_SHADER_FORMAT_INCOMPATIBLE,
	HERO_ERROR_GFX_SHADER_MISSING_VERTEX,
	HERO_ERROR_GFX_SHADER_MISSING_FRAGMENT,
	HERO_ERROR_GFX_SHADER_MISSING_MESH,
	HERO_ERROR_GFX_SHADER_MISSING_COMPUTE,
	HERO_ERROR_GFX_SHADER_MUST_BE_COMPUTE,

	HERO_ERROR_GFX_RENDER_PASS_LAYOUT_MISMATCH,

	HERO_ERROR_GFX_RENDER_PASS_LAYOUT_ATTACHMENTS_COUNT_CANNOT_BE_ZERO,

	HERO_ERROR_GFX_RENDER_PASS_ATTACHMENTS_MUST_MATCH_LAYOUT,

	HERO_ERROR_GFX_FRAME_BUFFER_ATTACHMENTS_COUNT_CANNOT_BE_ZERO,
	HERO_ERROR_GFX_FRAME_BUFFER_VULKAN_ATTACHMENTS,

	HERO_ERROR_GFX_PIPELINE_SHADER_ID_CANNOT_BE_NULL,
	HERO_ERROR_GFX_PIPELINE_RENDER_PASS_LAYOUT_ID_CANNOT_BE_NULL,
	HERO_ERROR_GFX_PIPELINE_SHADER_MUST_BE_COMPUTE,

	HERO_ERROR_GFX_PIPELINE_MUST_BE_GRAPHICS,
	HERO_ERROR_GFX_PIPELINE_MUST_BE_COMPUTE,

	HERO_ERROR_GFX_MATERIAL_PIPELINE_ID_CANNOT_BE_NULL,
	HERO_ERROR_GFX_MATERIAL_SHADER_GLOBALS_ID_CANNOT_BE_NULL,
	HERO_ERROR_GFX_MATERIAL_DESCRIPTOR_POOL_ID_CANNOT_BE_NULL,
	HERO_ERROR_GFX_MATERIAL_PIPELINE_AND_SHADER_GLOBALS_SHADER_ID_MISMATCH,

	HERO_ERROR_GFX_SWAPCHAIN_ARRAY_LAYERS_CANNOT_BE_ZERO,

	HERO_ERROR_GFX_COMMAND_POOL_STATIC_SUPPORT_NOT_ENABLED,

	HERO_ERROR_GFX_NO_SWAPCHAIN_OR_READBACK_OUTPUT,
	HERO_ERROR_GFX_DUPLICATE_SWAPCHAIN_IN_IMAGE_ATTACHMENTS,
	HERO_ERROR_GFX_GRAPH_CANNOT_BE_EMPTY,

	HERO_ERROR_GFX_SPIR_V_BINARY_INVALID_FORMAT,
	HERO_ERROR_GFX_SPIR_V_BINARY_CORRUPT,
	HERO_ERROR_GFX_SPIR_V_BINARY_UNSUPPORTED_VERSION,
	HERO_ERROR_GFX_SPIR_V_BINARY_UNSUPPORTED_INSTRUCTION_SCHEMA,
	HERO_ERROR_GFX_SPIR_V_BINARY_DESCRIPTOR_TYPE_MISMATCH,
	HERO_ERROR_GFX_SPIR_V_BINARY_DESCRIPTOR_COUNT_MISMATCH,
	HERO_ERROR_GFX_SPIR_V_BINARY_DESCRIPTOR_ELMT_SIZE_MISMATCH,

	HERO_ERROR_GFX_COUNT,
};

enum {
	HERO_GFX_ALLOC_TAG_PHYSICAL_DEVICES = HERO_ALLOC_TAG_GFX_START,
	HERO_GFX_ALLOC_TAG_PHYSICAL_DEVICES_VULKAN,
	HERO_GFX_ALLOC_TAG_PHYSICAL_DEVICES_VULKAN_SURFACE_FORMATS,
	HERO_GFX_ALLOC_TAG_PHYSICAL_DEVICES_VULKAN_QUEUE_FAMILY_PROPERTIES,

	HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_DESCRIPTOR_POOLS_TO_DEALLOCATE,
	HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_COMMAND_POOLS_TO_DEALLOCATE,
	HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_COMMAND_BUFFERS,
	HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_RENDER_SEMAPHORES,
	HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_SWAPCHAINS,
	HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_SWAPCHAIN_IMAGE_INDICES,
	HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMITS,

	HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_BUFFERS,
	HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_BUFFERS_IN_USE,
	HERO_GFX_ALLOC_TAG_VULKAN_STAGED_UPDATES,
	HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_COMMAND_BUFFERS,
	HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_COMMAND_BUFFERS_IN_USE,

	HERO_GFX_ALLOC_TAG_VERTEX_LAYOUT_VULKAN_BINDINGS,
	HERO_GFX_ALLOC_TAG_VERTEX_LAYOUT_VULKAN_ATTRIBUTES,

	HERO_GFX_ALLOC_TAG_VERTEX_ARRAY_VERTEX_BUFFERS,

	HERO_GFX_ALLOC_TAG_RENDER_PASS_LAYOUT_ATTACHMENTS,

	HERO_GFX_ALLOC_TAG_RENDER_PASS_CLEAR_VALUES,

	HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_DESCRIPTION,
	HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_REF,

	HERO_GFX_ALLOC_TAG_PIPELINE_VULKAN_ATTACHMENTS,

	HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_POOLS,
	HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS,
	HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUTS,
	HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_POOL_SIZES,
	HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_POOL_FREE_COUNTS,
	HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS_FREE_SETS,
	HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS_FREE_SETS_IN_USE,

	HERO_GFX_ALLOC_TAG_COMMAND_POOL_COMMAND_POOL_BUFFERS_POOL,
	HERO_GFX_ALLOC_TAG_COMMAND_POOL_VULKAN_FREE_BUFFERS,
	HERO_GFX_ALLOC_TAG_COMMAND_POOL_VULKAN_FREE_BUFFERS_IN_USE,

	HERO_GFX_ALLOC_TAG_SWAPCHAIN_VULKAN_PRESENT_MODES,
	HERO_GFX_ALLOC_TAG_SWAPCHAIN_VULKAN_IMAGES,
	HERO_GFX_ALLOC_TAG_SWAPCHAIN_IMAGE_IDS,
	HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE,
	HERO_GFX_ALLOC_TAG_VERTEX_LAYOUT_POOL,
	HERO_GFX_ALLOC_TAG_BUFFER_POOL,
	HERO_GFX_ALLOC_TAG_IMAGE_POOL,
	HERO_GFX_ALLOC_TAG_SAMPLER_POOL,
	HERO_GFX_ALLOC_TAG_SHADER_MODULE_POOL,
	HERO_GFX_ALLOC_TAG_SHADER_POOL,
	HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_POOL,
	HERO_GFX_ALLOC_TAG_SHADER_GLOBALS_POOL,
	HERO_GFX_ALLOC_TAG_RENDER_PASS_LAYOUT_POOL,
	HERO_GFX_ALLOC_TAG_RENDER_PASS_POOL,
	HERO_GFX_ALLOC_TAG_FRAME_BUFFER_POOL,
	HERO_GFX_ALLOC_TAG_PIPELINE_CACHE_POOL,
	HERO_GFX_ALLOC_TAG_PIPELINE_POOL,
	HERO_GFX_ALLOC_TAG_MATERIAL_POOL,
	HERO_GFX_ALLOC_TAG_SWAPCHAIN_POOL,
	HERO_GFX_ALLOC_TAG_COMMAND_POOL_POOL,
	HERO_GFX_ALLOC_TAG_RENDER_GRAPH_POOL,

	HERO_GFX_ALLOC_TAG_SHADER_METADATA,
	HERO_GFX_ALLOC_TAG_SPIR_V_KEY_TO_DESCRIPTOR_MAP,
	HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_DESCRIPTOR_MAP,
	HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_DESCRIPTOR_TYPE_MAP,
	HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_SIZE_STD140_MAP,
	HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_SIZE_STD430_MAP,
	HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_CONSTANT_MAP,
	HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_LOCATION_MAP,
	HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_VERTEX_ATTRIB_MAP,
	HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_NON_VERTEX_INPUT,
	HERO_GFX_ALLOC_TAG_SPIR_V_VERTEX_ATTRIBS,
};

HERO_TYPEDEF_OBJECT_ID(HeroVertexLayoutId);
HERO_TYPEDEF_OBJECT_ID(HeroBufferId);
HERO_TYPEDEF_OBJECT_ID(HeroVertexArrayId);
HERO_TYPEDEF_OBJECT_ID(HeroSamplerId);
HERO_TYPEDEF_OBJECT_ID(HeroImageId);
HERO_TYPEDEF_OBJECT_ID(HeroShaderModuleId);
HERO_TYPEDEF_OBJECT_ID(HeroShaderId);
HERO_TYPEDEF_OBJECT_ID(HeroDescriptorPoolId);
HERO_TYPEDEF_OBJECT_ID(HeroShaderGlobalsId);
HERO_TYPEDEF_OBJECT_ID(HeroRenderPassLayoutId);
HERO_TYPEDEF_OBJECT_ID(HeroRenderPassId);
HERO_TYPEDEF_OBJECT_ID(HeroFrameBufferId);
HERO_TYPEDEF_OBJECT_ID(HeroPipelineCacheId);
HERO_TYPEDEF_OBJECT_ID(HeroPipelineId);
HERO_TYPEDEF_OBJECT_ID(HeroMaterialId);
HERO_TYPEDEF_OBJECT_ID(HeroSwapchainId);
HERO_TYPEDEF_OBJECT_ID(HeroCommandPoolId);
HERO_TYPEDEF_OBJECT_ID(HeroCommandPoolBufferId);
HERO_TYPEDEF_OBJECT_ID(HeroRenderGraphId);

#define HERO_STACK_ELMT_TYPE HeroImageId
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE HeroBufferId
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE HeroMaterialId
#include "stack_gen.inl"

typedef struct HeroSurface HeroSurface;
struct HeroSurface {
	HeroGfxBackendType type;
		/*
	union {
#ifdef HERO_VULKAN_ENABLE
		VkSurfaceKHR vulkan;
#endif
	} backend;
*/
};

typedef struct HeroViewport HeroViewport;
struct HeroViewport {
	F32 x;
	F32 y;
	F32 width;
	F32 height;
	F32 min_depth;
	F32 max_depth;
};

typedef union HeroClearValue HeroClearValue;
union HeroClearValue {
	union {
		F32 float32[4];
		S32 sint32[4];
		U32 uint32[4];
	} color;
	struct {
		F32 depth;
		U32 stencil;
	} depth_stencil;
};

typedef U8 HeroSampleCount;
enum {
	HERO_SAMPLE_COUNT_1  = 1,
	HERO_SAMPLE_COUNT_2  = 2,
	HERO_SAMPLE_COUNT_4  = 4,
	HERO_SAMPLE_COUNT_8  = 8,
	HERO_SAMPLE_COUNT_16 = 16,
	HERO_SAMPLE_COUNT_32 = 32,
	HERO_SAMPLE_COUNT_64 = 64,
};

typedef U8 HeroCompareOp;
enum {
	HERO_COMPARE_OP_NEVER,
	HERO_COMPARE_OP_LESS,
	HERO_COMPARE_OP_EQUAL,
	HERO_COMPARE_OP_LESS_OR_EQUAL,
	HERO_COMPARE_OP_GREATER,
	HERO_COMPARE_OP_NOT_EQUAL,
	HERO_COMPARE_OP_GREATER_OR_EQUAL,
	HERO_COMPARE_OP_ALWAYS,
};

typedef U8 HeroImageFormat;
enum {
	HERO_IMAGE_FORMAT_NONE,

	HERO_IMAGE_FORMAT_R8_UNORM,
	HERO_IMAGE_FORMAT_R8G8_UNORM,
	HERO_IMAGE_FORMAT_R8G8B8_UNORM,
	HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,

	HERO_IMAGE_FORMAT_B8G8R8_UNORM,
	HERO_IMAGE_FORMAT_B8G8R8A8_UNORM,

    HERO_IMAGE_FORMAT_R32_UINT,
    HERO_IMAGE_FORMAT_R32_SINT,
	HERO_IMAGE_FORMAT_R32_SFLOAT,

	//
	// depth and stencil
	HERO_IMAGE_FORMAT_D16,
	HERO_IMAGE_FORMAT_D32,
	HERO_IMAGE_FORMAT_S8,
	HERO_IMAGE_FORMAT_D16_S8,
	HERO_IMAGE_FORMAT_D24_S8,
	HERO_IMAGE_FORMAT_D32_S8,

	HERO_IMAGE_FORMAT_COUNT,
};

#define HERO_IMAGE_FORMAT_IS_DEPTH(format) ((format) >= HERO_IMAGE_FORMAT_D16 && (format) <= HERO_IMAGE_FORMAT_D32_S8)

extern U8 hero_image_format_bytes_per_pixel[HERO_IMAGE_FORMAT_COUNT];

typedef U8 HeroIndexType;
enum {
	HERO_INDEX_TYPE_U8,
	HERO_INDEX_TYPE_U16,
	HERO_INDEX_TYPE_U32,

	HERO_INDEX_TYPE_COUNT,
};

extern U8 hero_index_type_sizes[HERO_INDEX_TYPE_COUNT];

typedef U8 HeroMemoryLocation;
enum {
	// memory is local to the GPU and the CPU cannot read from this resource.
	// this is the biggest pool of memory on the GPU.
	// writes to this memory should happen infrequently
	// in vulkan terms: DEVICE_LOCAL_BIT and may have (but not required) HOST_VISIBLE_BIT
	// in dx12 terms: D3D12_HEAP_TYPE_DEFAULT
	HERO_MEMORY_LOCATION_GPU,

	// memory is optimized for frequent updates to the GPU from the CPU.
	// this is the smallest pool of memory on the GPU.
	// in vulkan terms: HOST_VISIBLE_BIT and may have (but not required) DEVICE_LOCAL_BIT
	// in dx12 terms: D3D12_HEAP_TYPE_UPLOAD
	HERO_MEMORY_LOCATION_SHARED,

	// memory is optimized for frequent read to the CPU from the GPU.
	// in vulkan terms: DEVICE_LOCAL_BIT, HOST_VISIBLE_BIT and HOST_CACHED_BIT
	// in dx12 terms: D3D12_HEAP_TYPE_READBACK
	HERO_MEMORY_LOCATION_SHARED_CACHED,
};

typedef U32 HeroGfxFrameIdx;
#define HERO_GFX_FRAME_IDX_DIFF(a, b) ((S32)(a) - (S32)(b))
#define HERO_GFX_FRAME_IDX_LESS_THAN(a, b)             (HERO_GFX_FRAME_IDX_DIFF(a, b) < 0)
#define HERO_GFX_FRAME_IDX_LESS_THAN_OR_EQUAL(a, b)    (HERO_GFX_FRAME_IDX_DIFF(a, b) <= 0)
#define HERO_GFX_FRAME_IDX_GREATER_THAN(a, b)          (HERO_GFX_FRAME_IDX_DIFF(a, b) > 0)
#define HERO_GFX_FRAME_IDX_GREATER_THAN_OR_EQUAL(a, b) (HERO_GFX_FRAME_IDX_DIFF(a, b) >= 0)

#define HERO_GFX_INTERNAL_OBJECT(InternalType, public_object) ((InternalType*)HERO_PTR_SUB(public_object, offsetof(InternalType, public_)))

typedef struct HeroVertexArray HeroVertexArray;
typedef struct HeroRenderState HeroRenderState;

#define HERO_OBJECT_TYPE HeroVertexArray
#include "object_pool_gen_def.inl"

// ===========================================
//
//
// Physical Device
//
//
// ===========================================

typedef U8 HeroPhysicalDeviceType;
enum {
	HERO_PHYSICAL_DEVICE_TYPE_OTHER,
	HERO_PHYSICAL_DEVICE_TYPE_GPU_INTEGRATED,
	HERO_PHYSICAL_DEVICE_TYPE_GPU_DISCRETE,
	HERO_PHYSICAL_DEVICE_TYPE_GPU_VIRTUAL,
	HERO_PHYSICAL_DEVICE_TYPE_CPU,
};

typedef U16 HeroPhysicalDeviceFeature;
enum {
	HERO_PHYSICAL_DEVICE_FEATURE_ROBUST_BUFFER_ACCESS,
	HERO_PHYSICAL_DEVICE_FEATURE_FULL_DRAW_INDEX_UINT32,
	HERO_PHYSICAL_DEVICE_FEATURE_IMAGE_CUBE_ARRAY,
	HERO_PHYSICAL_DEVICE_FEATURE_INDEPENDENT_BLEND,
	HERO_PHYSICAL_DEVICE_FEATURE_GEOMETRY_SHADER,
	HERO_PHYSICAL_DEVICE_FEATURE_TESSELLATION_SHADER,
	HERO_PHYSICAL_DEVICE_FEATURE_SAMPLE_RATE_SHADING,
	HERO_PHYSICAL_DEVICE_FEATURE_DUAL_SRC_BLEND,
	HERO_PHYSICAL_DEVICE_FEATURE_LOGIC_OP,
	HERO_PHYSICAL_DEVICE_FEATURE_MULTI_DRAW_INDIRECT,
	HERO_PHYSICAL_DEVICE_FEATURE_DRAW_INDIRECT_FIRST_INSTANCE,
	HERO_PHYSICAL_DEVICE_FEATURE_DEPTH_CLAMP,
	HERO_PHYSICAL_DEVICE_FEATURE_DEPTH_BIAS_CLAMP,
	HERO_PHYSICAL_DEVICE_FEATURE_FILL_MODE_NON_SOLID,
	HERO_PHYSICAL_DEVICE_FEATURE_DEPTH_BOUNDS,
	HERO_PHYSICAL_DEVICE_FEATURE_WIDE_LINES,
	HERO_PHYSICAL_DEVICE_FEATURE_LARGE_POINTS,
	HERO_PHYSICAL_DEVICE_FEATURE_ALPHA_TO_ONE,
	HERO_PHYSICAL_DEVICE_FEATURE_MULTI_VIEWPORT,
	HERO_PHYSICAL_DEVICE_FEATURE_SAMPLER_ANISOTROPY,
	HERO_PHYSICAL_DEVICE_FEATURE_TEXTURE_COMPRESSION_ETC2,
	HERO_PHYSICAL_DEVICE_FEATURE_TEXTURE_COMPRESSION_ASTC_LDR,
	HERO_PHYSICAL_DEVICE_FEATURE_TEXTURE_COMPRESSION_BC,
	HERO_PHYSICAL_DEVICE_FEATURE_OCCLUSION_QUERY_PRECISE,
	HERO_PHYSICAL_DEVICE_FEATURE_PIPELINE_STATISTICS_QUERY,
	HERO_PHYSICAL_DEVICE_FEATURE_VERTEX_PIPELINE_STORES_AND_ATOMICS,
	HERO_PHYSICAL_DEVICE_FEATURE_FRAGMENT_STORES_AND_ATOMICS,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_TESSELLATION_AND_GEOMETRY_POINT_SIZE,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_IMAGE_GATHER_EXTENDED,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_EXTENDED_FORMATS,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_MULTISAMPLE,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_READ_WITHOUTFORMAT,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_UNIFORM_BUFFER_ARRAY_DYNAMIC_INDEXING,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_BUFFER_ARRAY_DYNAMIC_INDEXING,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_ARRAY_DYNAMIC_INDEXING,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_CLIP_DISTANCE,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_CULL_DISTANCE,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_FLOAT64,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_INT64,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_INT16,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_RESOURCE_RESIDENCY,
	HERO_PHYSICAL_DEVICE_FEATURE_SHADER_RESOURCE_MIN_LOD,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_BINDING,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY_BUFFER,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE2_D,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE3_D,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY2_SAMPLES,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY4_SAMPLES,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY8_SAMPLES,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY16_SAMPLES,
	HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY_ALIASED,
	HERO_PHYSICAL_DEVICE_FEATURE_VARIABLE_MULTISAMPLE_RATE,
	HERO_PHYSICAL_DEVICE_FEATURE_INHERITED_QUERIES,

	HERO_PHYSICAL_DEVICE_FEATURE_COUNT,
};

typedef U8 HeroQueueSupportFlags;
enum {
	HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS       = 0x1,
	HERO_QUEUE_SUPPORT_FLAGS_COMPUTE        = 0x2,
	HERO_QUEUE_SUPPORT_FLAGS_PRESENT        = 0x4,
	HERO_QUEUE_SUPPORT_FLAGS_ASYNC_COMPUTE  = 0x8,
	HERO_QUEUE_SUPPORT_FLAGS_ASYNC_TRANSFER = 0x10,
};

typedef struct HeroPhysicalDeviceFeatureFlags HeroPhysicalDeviceFeatureFlags;
struct HeroPhysicalDeviceFeatureFlags {
	U64 bitset[2];
};
#define HERO_PHYSICAL_DEVICE_FEATURE_FLAGS_HAS(flags, feature) (((flags).bitset[(feature) / 64] & ((U64)1 << ((feature) % 64))) != 0)
#define HERO_PHYSICAL_DEVICE_FEATURE_FLAGS_INSERT(flags, feature) ((flags).bitset[(feature) / 64] |= ((U64)1 << ((feature) % 64)))

typedef struct HeroPhysicalDevice HeroPhysicalDevice;
struct HeroPhysicalDevice {
	const char* device_name;
	HeroPhysicalDeviceFeatureFlags feature_flags;
	U32 driver_version;
	U32 vendor_id;
	U32 device_id;
	HeroPhysicalDeviceType device_type;
	HeroQueueSupportFlags queue_support_flags;
};

#define HERO_SURFACE_IMAGE_FORMAT HERO_IMAGE_FORMAT_B8G8R8A8_UNORM

HeroResult hero_physical_device_get(U32 idx, HeroPhysicalDevice** physical_devices_out);
HeroResult hero_physical_device_surface_image_formats_supported(HeroPhysicalDevice* physical_device, HeroSurface surface, HeroImageFormat* formats, U32 formats_count);

// ===========================================
//
//
// Logical Device
//
//
// ===========================================

typedef struct HeroLogicalDeviceSetup HeroLogicalDeviceSetup;
struct HeroLogicalDeviceSetup {
	HeroPhysicalDeviceFeatureFlags feature_flags;
	HeroQueueSupportFlags queue_support_flags;
	HeroIAlctor alctor;
	U32 buffers_cap;
	U32 images_cap;
	U32 samplers_cap;
	U32 shader_modules_cap;
	U32 shaders_cap;
	U32 descriptor_pools_cap;
	U32 shader_globals_cap;
	U32 render_pass_layouts_cap;
	U32 render_passes_cap;
	U32 frame_buffers_cap;
	U32 pipeline_caches_cap;
	U32 pipelines_cap;
	U32 materials_cap;
	U32 swapchains_cap;
	U32 command_pools_cap;
	U32 render_graphs_cap;
};

typedef struct HeroLogicalDevice HeroLogicalDevice;
struct HeroLogicalDevice {
	HeroPhysicalDevice*             physical_device;
	HeroObjectPool(HeroVertexArray) vertex_array_pool;
	HeroIAlctor                     alctor;
	HeroQueueSupportFlags           queue_support_flags;
	HeroGfxFrameIdx                 last_submitted_frame_idx;
	HeroGfxFrameIdx                 last_completed_frame_idx;
};

HeroResult hero_logical_device_init(HeroPhysicalDevice* physical_device, HeroLogicalDeviceSetup* setup, HeroLogicalDevice** out);
HeroResult hero_logical_device_deinit(HeroLogicalDevice* ldev);

HeroResult hero_logical_device_frame_start(HeroLogicalDevice* ldev);

HeroResult hero_logical_device_queue_transfer(HeroLogicalDevice* ldev);
HeroResult hero_logical_device_queue_command_buffers(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandPoolBufferId* command_pool_buffer_ids, U32 command_pool_buffers_count);
HeroResult hero_logical_device_submit(HeroLogicalDevice* ldev, HeroSwapchainId* swapchain_ids, U32 swapchains_count);

// ===========================================
//
//
// Vertex Layout
//
//
// ===========================================

typedef U8 HeroVertexElmtType;
enum {
	HERO_VERTEX_ELMT_TYPE_U8,
	HERO_VERTEX_ELMT_TYPE_S8,
	HERO_VERTEX_ELMT_TYPE_U8_F32,
	HERO_VERTEX_ELMT_TYPE_S8_F32,
	HERO_VERTEX_ELMT_TYPE_U8_F32_NORMALIZE,
	HERO_VERTEX_ELMT_TYPE_S8_F32_NORMALIZE,
	HERO_VERTEX_ELMT_TYPE_U16,
	HERO_VERTEX_ELMT_TYPE_S16,
	HERO_VERTEX_ELMT_TYPE_U16_F32,
	HERO_VERTEX_ELMT_TYPE_S16_F32,
	HERO_VERTEX_ELMT_TYPE_U16_F32_NORMALIZE,
	HERO_VERTEX_ELMT_TYPE_S16_F32_NORMALIZE,
	HERO_VERTEX_ELMT_TYPE_U32,
	HERO_VERTEX_ELMT_TYPE_S32,
	HERO_VERTEX_ELMT_TYPE_U64,
	HERO_VERTEX_ELMT_TYPE_S64,
	HERO_VERTEX_ELMT_TYPE_F16,
	HERO_VERTEX_ELMT_TYPE_F32,
	HERO_VERTEX_ELMT_TYPE_F64,

	HERO_VERTEX_ELMT_TYPE_COUNT,
};

extern U8 hero_vertex_elmt_type_sizes[HERO_VERTEX_ELMT_TYPE_COUNT];
extern U8 hero_vertex_elmt_type_aligns[HERO_VERTEX_ELMT_TYPE_COUNT];

typedef U8 HeroVertexVectorType;
enum {
	HERO_VERTEX_VECTOR_TYPE_1,
	HERO_VERTEX_VECTOR_TYPE_2,
	HERO_VERTEX_VECTOR_TYPE_3,
	HERO_VERTEX_VECTOR_TYPE_4,

	HERO_VERTEX_VECTOR_TYPE_COUNT,
};
#define HERO_VERTEX_VECTOR_TYPE_ELMTS_COUNT(vector_type) ((U32)(vector_type + 1))
#define HERO_VERTEX_VECTOR_TYPE_INIT(elmts_count) ((U32)(elmts_count - 1))

typedef struct HeroVertexAttribInfo HeroVertexAttribInfo;
struct HeroVertexAttribInfo {
	U8                   location;
	HeroVertexElmtType   elmt_type: 6;
	HeroVertexVectorType vector_type: 2;
};

typedef struct HeroVertexBindingInfo HeroVertexBindingInfo;
struct HeroVertexBindingInfo {
	HeroVertexAttribInfo* attribs;
	U16                   attribs_count;
	U16                   size;
};

typedef struct HeroVertexLayout HeroVertexLayout;
struct HeroVertexLayout {
	U16                    bindings_count;
	HeroVertexBindingInfo* bindings;
};

HeroResult hero_vertex_layout_register(HeroVertexLayout* vl, bool is_static, HeroVertexLayoutId* id_out);
HeroResult hero_vertex_layout_deregister(HeroVertexLayoutId id);
HeroResult hero_vertex_layout_get(HeroVertexLayoutId id, HeroVertexLayout** out);

// ===========================================
//
//
// Buffer
//
//
// ===========================================

typedef U8 HeroBufferType;
enum {
	HERO_BUFFER_TYPE_NONE,
	HERO_BUFFER_TYPE_VERTEX,
	HERO_BUFFER_TYPE_INDEX,
	HERO_BUFFER_TYPE_UNIFORM,
	HERO_BUFFER_TYPE_STORAGE,
};

typedef U32 HeroBufferFlags;
enum {
	HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS = 0x1,
	HERO_BUFFER_FLAGS_USED_FOR_COMPUTE  = 0x2,
	HERO_BUFFER_FLAGS_INDIRECT_DRAW     = 0x4,
};

typedef struct HeroBuffer HeroBuffer;
struct HeroBuffer {
	HeroBufferType type;
	HeroBufferFlags flags;
	HeroMemoryLocation memory_location;
	HeroQueueSupportFlags queue_support_flags;
	U64 elmts_count;
	U32 elmt_size;
	HeroGfxFrameIdx last_submitted_frame_idx;
	union {
		struct {
			HeroVertexLayoutId layout_id;
			U32 binding_idx;
		} vertex;
		HeroIndexType index_type;
	} typed;
};

typedef struct HeroBufferSetup HeroBufferSetup;
struct HeroBufferSetup {
	HeroBufferFlags flags;
	HeroBufferType type;
	HeroMemoryLocation memory_location;
	HeroQueueSupportFlags queue_support_flags;
	U64 elmts_count;
	U32 elmt_size; // ignored for HERO_BUFFER_TYPE_VERTEX & HERO_BUFFER_TYPE_INDEX
	union {
		struct {
			HeroVertexLayoutId layout_id;
			U32 binding_idx;
		} vertex;
		HeroIndexType index_type;
	} typed;
};

HeroResult hero_buffer_init(HeroLogicalDevice* ldev, HeroBufferSetup* setup, HeroBufferId* id_out);
HeroResult hero_buffer_deinit(HeroLogicalDevice* ldev, HeroBufferId id);
HeroResult hero_buffer_get(HeroLogicalDevice* ldev, HeroBufferId id, HeroBuffer** out);

HeroResult hero_buffer_resize(HeroLogicalDevice* ldev, HeroBufferId id, U64 elmts_count);
HeroResult hero_buffer_reserve(HeroLogicalDevice* ldev, HeroBufferId id, U64 elmts_count);

// only for buffers with a memory_location of HERO_MEMORY_LOCATION_SHARED or HERO_MEMORY_LOCATION_SHARED_CACHED
// the mapped memory is only returned when HERO_GFX_FRAME_IDX_LESS_THAN_OR_EQUAL(HeroBuffer.last_submitted_frame_idx, HeroLogicalDevice.last_completed_frame_idx)
HeroResult hero_buffer_map(HeroLogicalDevice* ldev, HeroBufferId id, void** addr_out);

HeroResult hero_buffer_read(HeroLogicalDevice* ldev, HeroBufferId id, U64 start_idx, Uptr elmts_count, void* destination);
HeroResult hero_buffer_write(HeroLogicalDevice* ldev, HeroBufferId id, U64 start_idx, Uptr elmts_count, void** destination_out);

// ===========================================
//
//
// Vertex Array
//
//
// ===========================================

struct HeroVertexArray {
	HeroObjectHeader header;
	HeroBufferId* vertex_buffer_ids;
	HeroVertexLayoutId layout_id;
	HeroBufferId index_buffer_id; // optional
	U16 vertex_buffers_count;
};

#define HERO_OBJECT_ID_TYPE HeroVertexArrayId
#define HERO_OBJECT_TYPE HeroVertexArray
#include "object_pool_gen_impl.inl"

typedef struct HeroVertexArraySetup HeroVertexArraySetup;
struct HeroVertexArraySetup {
	HeroBufferId* vertex_buffer_ids;
	HeroVertexLayoutId layout_id;
	HeroBufferId index_buffer_id; // optional
	U16 vertex_buffers_count;
};

HeroResult hero_vertex_array_init(HeroLogicalDevice* ldev, HeroVertexArraySetup* setup, HeroVertexArrayId* id_out);
HeroResult hero_vertex_array_deinit(HeroLogicalDevice* ldev, HeroVertexArrayId id);
HeroResult hero_vertex_array_get(HeroLogicalDevice* ldev, HeroVertexArrayId id, HeroVertexArray** out);

// ===========================================
//
//
// Image
//
//
// ===========================================

typedef U8 HeroImageType;
enum {
	HERO_IMAGE_TYPE_1D,
	HERO_IMAGE_TYPE_1D_ARRAY,
	HERO_IMAGE_TYPE_2D,
	HERO_IMAGE_TYPE_2D_ARRAY,
	HERO_IMAGE_TYPE_3D,
	HERO_IMAGE_TYPE_CUBE,
	HERO_IMAGE_TYPE_CUBE_ARRAY,
};

typedef U8 HeroImageFlags;
enum {
	HERO_IMAGE_FLAGS_SAMPLED                  = 0x1,
	HERO_IMAGE_FLAGS_STORAGE                  = 0x2,
	HERO_IMAGE_FLAGS_COLOR_ATTACHMENT         = 0x4,
	HERO_IMAGE_FLAGS_DEPTH_STENCIL_ATTACHMENT = 0x8,
	HERO_IMAGE_FLAGS_INPUT_ATTACHMENT         = 0x10,
	HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS        = 0x20,
	HERO_IMAGE_FLAGS_USED_FOR_COMPUTE         = 0x40,
	HERO_IMAGE_FLAGS_SWAPCHAIN                = 0x80,
};

typedef struct HeroImage HeroImage;
struct HeroImage {
	HeroImageType type;
	HeroImageFormat internal_format;
	HeroImageFormat format;
	HeroImageFlags flags;
	HeroSampleCount samples;
	HeroMemoryLocation memory_location;
	HeroQueueSupportFlags queue_support_flags;
	U32 width;
	U32 height;
	U32 depth;
	U32 mip_levels;
	U32 array_layers_count;
	HeroGfxFrameIdx last_submitted_frame_idx;
};

typedef struct HeroImageSetup HeroImageSetup;
struct HeroImageSetup {
	HeroImageType type;
	HeroImageFormat internal_format;
	HeroImageFormat format;
	HeroImageFlags flags;
	HeroSampleCount samples;
	HeroMemoryLocation memory_location;
	HeroQueueSupportFlags queue_support_flags;
	U32 width;
	U32 height;
	U16 depth;
	U16 mip_levels;
	U16 array_layers_count;
};

typedef struct HeroImageArea HeroImageArea;
struct HeroImageArea {
	U32 offset_x;
	U32 offset_y;
	U32 offset_z;
	U32 width;
	U32 height;
	U16 depth;
	U16 mip_level;
	U16 array_layer;
	U16 array_layers_count;
};

HeroResult hero_image_init(HeroLogicalDevice* ldev, HeroImageSetup* setup, HeroImageId* id_out);
HeroResult hero_image_deinit(HeroLogicalDevice* ldev, HeroImageId id);
HeroResult hero_image_get(HeroLogicalDevice* ldev, HeroImageId id, HeroImage** out);

HeroResult hero_image_resize(HeroLogicalDevice* ldev, HeroImageId id, U32 width, U32 height, U32 depth, U32 mip_levels, U32 array_layers_count);

// only for buffers with a memory_location of HERO_MEMORY_LOCATION_SHARED or HERO_MEMORY_LOCATION_SHARED_CACHED
// the mapped memory is only returned when HERO_GFX_FRAME_IDX_LESS_THAN_OR_EQUAL(HeroBuffer.last_submitted_frame_idx, HeroLogicalDevice.last_completed_frame_idx)
HeroResult hero_image_map(HeroLogicalDevice* ldev, HeroImageId id, void** addr_out);

HeroResult hero_image_read(HeroLogicalDevice* ldev, HeroImageId id, HeroImageArea* area, void* destination);
HeroResult hero_image_write(HeroLogicalDevice* ldev, HeroImageId id, HeroImageArea* area, void** destination_out);

// ===========================================
//
//
// Sampler
//
//
// ===========================================

typedef U8 HeroFilter;
enum {
	HERO_FILTER_NEAREST,
	HERO_FILTER_LINEAR,
	HERO_FILTER_CUBIC_IMG,
};

typedef U8 HeroSamplerMipmapMode;
enum {
	HERO_SAMPLER_MIPMAP_MODE_NEAREST,
	HERO_SAMPLER_MIPMAP_MODE_LINEAR,
};

typedef U8 HeroSamplerAddressMode;
enum {
	HERO_SAMPLER_ADDRESS_MODE_REPEAT,
	HERO_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	HERO_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

typedef U8 HeroBorderColor;
enum {
	HERO_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
	HERO_BORDER_COLOR_INT_TRANSPARENT_BLACK,
	HERO_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	HERO_BORDER_COLOR_INT_OPAQUE_BLACK,
	HERO_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	HERO_BORDER_COLOR_INT_OPAQUE_WHITE,
};

typedef struct HeroSampler HeroSampler;
struct HeroSampler {
	HeroFilter mag_filter;
	HeroFilter min_filter;
	HeroSamplerMipmapMode mipmap_mode;
	HeroSamplerAddressMode address_mode_u;
	HeroSamplerAddressMode address_mode_v;
	HeroSamplerAddressMode address_mode_w;
	F32 mip_lod_bias;
	F32 max_anisotropy;
	F32 min_lod;
	F32 max_lod;
	HeroCompareOp compare_op;
	HeroBorderColor border_color;
};

typedef U8 HeroSamplerSetupFlags;
enum {
	HERO_SAMPLER_SETUP_FLAGS_ANISOTROPY_ENABLE = 0x1,
	HERO_SAMPLER_SETUP_FLAGS_COMPARE_ENABLE = 0x2,
	HERO_SAMPLER_SETUP_FLAGS_UNNORMALIZED_COORDINATES = 0x4,
};

typedef struct HeroSamplerSetup HeroSamplerSetup;
struct HeroSamplerSetup {
	HeroSamplerSetupFlags flags;
	HeroFilter mag_filter;
	HeroFilter min_filter;
	HeroSamplerMipmapMode mipmap_mode;
	HeroSamplerAddressMode address_mode_u;
	HeroSamplerAddressMode address_mode_v;
	HeroSamplerAddressMode address_mode_w;
	F32 mip_lod_bias;
	F32 max_anisotropy;
	F32 min_lod;
	F32 max_lod;
	HeroCompareOp compare_op;
	HeroBorderColor border_color;
};

HeroResult hero_sampler_init(HeroLogicalDevice* ldev, HeroSamplerSetup* setup, HeroSamplerId* id_out);
HeroResult hero_sampler_deinit(HeroLogicalDevice* ldev, HeroSamplerId id);
HeroResult hero_sampler_get(HeroLogicalDevice* ldev, HeroSamplerId id, HeroSampler** out);

// ===========================================
//
//
// Shader Metadata
//
//
// ===========================================

typedef U8 HeroSpirVVertexElmtType;
enum {
	HERO_SHADER_VERTEX_ELMT_TYPE_FLOAT,
	HERO_SHADER_VERTEX_ELMT_TYPE_UINT,
	HERO_SHADER_VERTEX_ELMT_TYPE_SINT,

	HERO_SHADER_VERTEX_ELMT_TYPE_COUNT,
};

typedef struct HeroSpirVVertexAttribInfo HeroSpirVVertexAttribInfo;
struct HeroSpirVVertexAttribInfo {
	U8                       location;
	HeroSpirVVertexElmtType  elmt_type: 2;
	HeroVertexVectorType     vector_type: 2;
};

typedef U8 HeroDescriptorType;
enum {
	HERO_DESCRIPTOR_TYPE_SAMPLER,
	HERO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	HERO_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	HERO_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	HERO_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
	HERO_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
	HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	HERO_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,

	HERO_DESCRIPTOR_TYPE_COUNT,
};

typedef struct HeroSpirVDescriptorBinding HeroSpirVDescriptorBinding;
struct HeroSpirVDescriptorBinding {
	U32 stage_flags; // VkShaderStageFlags
	HeroDescriptorType descriptor_type;
	U16 set;
	U16 binding;
	U16 descriptor_count;
	U32 elmt_size;
};

typedef struct HeroShaderStage HeroShaderStage;
struct HeroShaderStage {
	union {
		struct {
			const char* entry_point_name;
		} spir_v;
	} backend;
	HeroShaderModuleId module_id;
};

typedef U8 HeroShaderType;
enum {
	HERO_SHADER_TYPE_GRAPHICS,
	HERO_SHADER_TYPE_GRAPHICS_MESH,
	HERO_SHADER_TYPE_COMPUTE,
	HERO_SHADER_TYPE_COUNT,
};
extern U8 HERO_SHADER_TYPE_STAGES_COUNTS[HERO_SHADER_TYPE_COUNT];

#define HERO_SHADER_STAGES_COUNT 5
typedef struct HeroShaderStages HeroShaderStages;
struct HeroShaderStages {
	HeroShaderType type;
	union {
		struct {
			HeroShaderStage compute;
			HeroPipelineCacheId cache_id;
		} compute;
		struct {
			HeroShaderStage vertex;
			HeroShaderStage tessellation_control;
			HeroShaderStage tessellation_evaluation;
			HeroShaderStage geometry;
			HeroShaderStage fragment;
		} graphics;
		struct {
			HeroShaderStage task;
			HeroShaderStage mesh;
			HeroShaderStage fragment;
		} graphics_mesh;
		HeroShaderStage array[HERO_SHADER_STAGES_COUNT];
	} data;
};

typedef U8 HeroShaderStageFlags;
enum {
	HERO_SHADER_STAGE_FLAGS_VERTEX                  = 0x1,
	HERO_SHADER_STAGE_FLAGS_TESSELLATION_CONTROL    = 0x2,
	HERO_SHADER_STAGE_FLAGS_TESSELLATION_EVALUATION = 0x4,
	HERO_SHADER_STAGE_FLAGS_GEOMETRY                = 0x8,
	HERO_SHADER_STAGE_FLAGS_FRAGMENT                = 0x10,
	HERO_SHADER_STAGE_FLAGS_ALL_GRAPHICS            = 0x1f,

	HERO_SHADER_STAGE_FLAGS_COMPUTE                 = 0x20,
	HERO_SHADER_STAGE_FLAGS_TASK                    = 0x40,
	HERO_SHADER_STAGE_FLAGS_MESH                    = 0x80,
	HERO_SHADER_STAGE_FLAGS_ALL_GRAPHICS_MESH       = HERO_SHADER_STAGE_FLAGS_TASK | HERO_SHADER_STAGE_FLAGS_MESH | HERO_SHADER_STAGE_FLAGS_FRAGMENT,
};

#define HERO_SHADER_METADATA_VERSION 0
typedef struct HeroShaderMetadata HeroShaderMetadata;
struct HeroShaderMetadata {
	U32 version;
	HeroHash stages_checksum;

	struct {
		U16 vertex_attribs_offset;
		U16 vertex_attribs_count;

		U16 descriptor_bindings_offset;
		U16 descriptor_bindings_counts[HERO_GFX_DESCRIPTOR_SET_COUNT];

		HeroShaderStageFlags push_constants_shader_stage_flags;
		U16 push_constants_size;
	} spir_v;
};

typedef struct HeroShaderMetadataAllocSetup HeroShaderMetadataAllocSetup;
struct HeroShaderMetadataAllocSetup {
	struct {
		U16 vertex_attribs_count;
		U16 descriptor_bindings_counts[HERO_GFX_DESCRIPTOR_SET_COUNT];
	} spir_v;
};

typedef struct HeroShaderMetadataSetup HeroShaderMetadataSetup;
struct HeroShaderMetadataSetup {
	HeroShaderStages* stages;
};

HeroResult hero_shader_stages_validate(HeroShaderStages* stages);

HeroShaderMetadata* hero_shader_metadata_alloc(HeroShaderMetadataAllocSetup* setup);
HeroResult hero_shader_metadata_calculate(HeroLogicalDevice* ldev, HeroShaderMetadataSetup* setup, HeroShaderMetadata** out);

static inline HeroSpirVVertexAttribInfo* hero_shader_metadata_spir_v_vertex_attribs(HeroShaderMetadata* metadata) {
	return (HeroSpirVVertexAttribInfo*)HERO_PTR_ADD(metadata, metadata->spir_v.vertex_attribs_offset);
}

static inline HeroSpirVDescriptorBinding* hero_shader_metadata_spir_v_descriptor_bindings(HeroShaderMetadata* metadata) {
	return (HeroSpirVDescriptorBinding*)HERO_PTR_ADD(metadata, metadata->spir_v.descriptor_bindings_offset);
}

static inline U32 hero_shader_metadata_spir_v_descriptor_bindings_count(HeroShaderMetadata* metadata) {
	U16 count = 0;
	for (U32 i = 0; i < HERO_GFX_DESCRIPTOR_SET_COUNT; i += 1) {
		count += metadata->spir_v.descriptor_bindings_counts[i];
	}
	return count;
}

// ===========================================
//
//
// Shader
//
//
// ===========================================

typedef U8 HeroShaderFormat;
enum {
	HERO_SHADER_FORMAT_NONE,
	HERO_SHADER_FORMAT_SPIR_V,
	HERO_SHADER_FORMAT_HLSL,
	HERO_SHADER_FORMAT_MSL,
};

typedef struct HeroShaderModule HeroShaderModule;
struct HeroShaderModule {
	U8* code;
	U32 code_size;
	HeroShaderFormat format;
};

typedef struct HeroShader HeroShader;
struct HeroShader {
	HeroShaderMetadata* metadata;
	HeroShaderStages stages;
};

typedef struct HeroShaderModuleSetup HeroShaderModuleSetup;
struct HeroShaderModuleSetup {
	HeroShaderFormat format;
	U8* code;
	U32 code_size;
};

typedef struct HeroShaderSetup HeroShaderSetup;
struct HeroShaderSetup {
	HeroShaderMetadata* metadata;
	HeroShaderStages* stages;
};

HeroResult hero_shader_module_init(HeroLogicalDevice* ldev, HeroShaderModuleSetup* setup, HeroShaderModuleId* id_out);
HeroResult hero_shader_module_deinit(HeroLogicalDevice* ldev, HeroShaderModuleId id);
HeroResult hero_shader_module_get(HeroLogicalDevice* ldev, HeroShaderModuleId id, HeroShaderModule** out);

HeroHash hero_shader_stages_checksum(HeroShaderStages* stages);

HeroResult hero_shader_init(HeroLogicalDevice* ldev, HeroShaderSetup* setup, HeroShaderId* id_out);
HeroResult hero_shader_deinit(HeroLogicalDevice* ldev, HeroShaderId id);
HeroResult hero_shader_get(HeroLogicalDevice* ldev, HeroShaderId id, HeroShader** out);

// ===========================================
//
//
// Descriptor Allocator
//
//
// ===========================================

typedef union HeroDescriptorData HeroDescriptorData;
union HeroDescriptorData {
	struct {
		HeroSamplerId sampler_id;
		HeroImageId id;
	} image;
	struct {
		HeroBufferId id;
		U64 offset;
	} buffer;
};

typedef struct HeroDescriptorShaderInfo HeroDescriptorShaderInfo;
struct HeroDescriptorShaderInfo {
	HeroShaderId shader_id;
	U16 advised_pool_counts[HERO_GFX_DESCRIPTOR_SET_COUNT];
};

typedef struct HeroDescriptorPoolSetup HeroDescriptorPoolSetup;
struct HeroDescriptorPoolSetup {
	HeroDescriptorShaderInfo* shader_infos;
	U32 shader_infos_count;
};

HeroResult hero_descriptor_pool_init(HeroLogicalDevice* ldev, HeroDescriptorPoolSetup* setup, HeroDescriptorPoolId* id_out);
HeroResult hero_descriptor_pool_deinit(HeroLogicalDevice* ldev, HeroDescriptorPoolId id);
HeroResult hero_descriptor_pool_reset(HeroLogicalDevice* ldev, HeroDescriptorPoolId id);

// ===========================================
//
//
// Shader Globals
//
//
// ===========================================

typedef struct HeroShaderGlobals HeroShaderGlobals;
struct HeroShaderGlobals {
	HeroShaderId shader_id;
	HeroDescriptorPoolId descriptor_pool_id;
};

typedef struct HeroShaderGlobalsSetup HeroShaderGlobalsSetup;
struct HeroShaderGlobalsSetup {
	HeroShaderId shader_id;
	HeroDescriptorPoolId descriptor_pool_id;
};

HeroResult hero_shader_globals_init(HeroLogicalDevice* ldev, HeroShaderGlobalsSetup* setup, HeroShaderGlobalsId* id_out);
HeroResult hero_shader_globals_deinit(HeroLogicalDevice* ldev, HeroShaderGlobalsId id);
HeroResult hero_shader_globals_get(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, HeroShaderGlobals** out);

HeroResult hero_shader_globals_set_descriptor(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data);
HeroResult hero_shader_globals_set_sampler(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroSamplerId sampler_id);
HeroResult hero_shader_globals_set_image(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id);
HeroResult hero_shader_globals_set_image_storage(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id);
HeroResult hero_shader_globals_set_image_sampler(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id, HeroSamplerId sampler_id);
HeroResult hero_shader_globals_set_uniform_buffer(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset);
HeroResult hero_shader_globals_set_storage_buffer(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset);
HeroResult hero_shader_globals_update(HeroLogicalDevice* ldev, HeroShaderGlobalsId id);

// ===========================================
//
//
// Render Pass Layout
//
//
// ===========================================

typedef U8 HeroAttachementPostUsage;
enum {
	HERO_ATTACHEMENT_POST_USAGE_NONE,
	HERO_ATTACHEMENT_POST_USAGE_PRESENT,
	HERO_ATTACHEMENT_POST_USAGE_SAMPLED,
};

typedef struct HeroAttachmentLayout HeroAttachmentLayout;
struct HeroAttachmentLayout {
	HeroImageFormat format;
	HeroSampleCount samples_count;
	HeroAttachementPostUsage post_usage;
};

typedef struct HeroRenderPassLayout HeroRenderPassLayout;
struct HeroRenderPassLayout {
	HeroAttachmentLayout* attachments;
	U32 attachments_count;
};

typedef struct HeroRenderPassLayoutSetup HeroRenderPassLayoutSetup;
struct HeroRenderPassLayoutSetup {
	HeroAttachmentLayout* attachments;
	U32 attachments_count;
};

HeroResult hero_render_pass_layout_init(HeroLogicalDevice* ldev, HeroRenderPassLayoutSetup* setup, HeroRenderPassLayoutId* id_out);
HeroResult hero_render_pass_layout_deinit(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id);
HeroResult hero_render_pass_layout_get(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id, HeroRenderPassLayout** out);

// ===========================================
//
//
// Render Pass
//
//
// ===========================================

typedef U8 HeroAttachmentLoadOp;
enum {
	HERO_ATTACHMENT_LOAD_OP_PRESERVE,
	HERO_ATTACHMENT_LOAD_OP_CLEAR,
	HERO_ATTACHMENT_LOAD_OP_UNINITIALIZED,
};

typedef U8 HeroAttachmentStoreOp;
enum {
	HERO_ATTACHMENT_STORE_OP_PRESERVE,
	HERO_ATTACHMENT_STORE_OP_DISCARD,
};

typedef struct HeroAttachmentInfo HeroAttachmentInfo;
struct HeroAttachmentInfo {
	HeroAttachmentLoadOp load_op;
	HeroAttachmentStoreOp store_op;
	HeroAttachmentLoadOp stencil_load_op;
	HeroAttachmentStoreOp stencil_store_op;
};

typedef struct HeroRenderPass HeroRenderPass;
struct HeroRenderPass {
	HeroRenderPassLayoutId layout_id;
	HeroDescriptorPoolId draw_cmd_descriptor_pool_id;
	HeroClearValue* attachment_clear_values;
	U16 attachments_count;
};

typedef struct HeroRenderPassSetup HeroRenderPassSetup;
struct HeroRenderPassSetup {
	HeroRenderPassLayoutId layout_id;
	HeroDescriptorPoolId draw_cmd_descriptor_pool_id;
	HeroAttachmentInfo* attachments;
	HeroClearValue* attachment_clear_values; // zeroed if set to NULL
	U16 attachments_count;
};

HeroResult hero_render_pass_init(HeroLogicalDevice* ldev, HeroRenderPassSetup* setup, HeroRenderPassId* id_out);
HeroResult hero_render_pass_deinit(HeroLogicalDevice* ldev, HeroRenderPassId id);
HeroResult hero_render_pass_get(HeroLogicalDevice* ldev, HeroRenderPassId id, HeroRenderPass** out);

// ===========================================
//
//
// Frame Buffer
//
//
// ===========================================

typedef struct HeroFrameBuffer HeroFrameBuffer;
struct HeroFrameBuffer {
	HeroImageId* attachments;
	U16 attachments_count;
	U16 layers;
	HeroRenderPassLayoutId render_pass_layout_id;
	U32 width;
	U32 height;
};

typedef struct HeroFrameBufferSetup HeroFrameBufferSetup;
struct HeroFrameBufferSetup {
	HeroImageId* attachments;
	U16 attachments_count;
	U16 layers;
	HeroRenderPassLayoutId render_pass_layout_id;
	U32 width;
	U32 height;
};

HeroResult hero_frame_buffer_init(HeroLogicalDevice* ldev, HeroFrameBufferSetup* setup, HeroFrameBufferId* id_out);
HeroResult hero_frame_buffer_deinit(HeroLogicalDevice* ldev, HeroFrameBufferId id);
HeroResult hero_frame_buffer_get(HeroLogicalDevice* ldev, HeroFrameBufferId id, HeroFrameBuffer** out);

// ===========================================
//
//
// Pipeline Cache
//
//
// ===========================================

typedef struct HeroPipelineCacheSetup HeroPipelineCacheSetup;
struct HeroPipelineCacheSetup {
	void* data;
	Uptr size;
};

HeroResult hero_pipeline_cache_init(HeroLogicalDevice* ldev, HeroPipelineCacheSetup* setup, HeroPipelineCacheId* id_out);
HeroResult hero_pipeline_cache_deinit(HeroLogicalDevice* ldev, HeroPipelineCacheId id);

// ===========================================
//
//
// Pipeline Layout
//
//
// ===========================================

typedef U8 HeroPipelineType;
enum {
	HERO_PIPELINE_TYPE_GRAPHICS,
	HERO_PIPELINE_TYPE_COMPUTE,
};

typedef struct HeroPipeline HeroPipeline;
struct HeroPipeline {
	HeroPipelineType type;
	HeroShaderId shader_id;
	HeroRenderPassLayoutId render_pass_layout_id;
	HeroVertexLayoutId vertex_layout_id;
};

typedef struct HeroPipelineGraphicsSetup HeroPipelineGraphicsSetup;
struct HeroPipelineGraphicsSetup {
	HeroRenderState* render_state;
	HeroShaderId shader_id;
	HeroRenderPassLayoutId render_pass_layout_id;
	HeroVertexLayoutId vertex_layout_id;
	HeroPipelineCacheId cache_id;
};

HeroResult hero_pipeline_graphics_init(HeroLogicalDevice* ldev, HeroPipelineGraphicsSetup* setup, HeroPipelineId* id_out);
HeroResult hero_pipeline_deinit(HeroLogicalDevice* ldev, HeroPipelineId id);
HeroResult hero_pipeline_get(HeroLogicalDevice* ldev, HeroPipelineId id, HeroPipeline** out);

// ===========================================
//
//
// Material
//
//
// ===========================================

typedef struct HeroMaterial HeroMaterial;
struct HeroMaterial {
	HeroPipelineId pipeline_id;
	HeroShaderId shader_id;
	HeroShaderGlobalsId shader_globals_id;
	HeroDescriptorPoolId descriptor_pool_id;
};

typedef struct HeroMaterialSetup HeroMaterialSetup;
struct HeroMaterialSetup {
	HeroPipelineId pipeline_id;
	HeroShaderGlobalsId shader_globals_id;
	HeroDescriptorPoolId descriptor_pool_id;
};

HeroResult hero_material_init(HeroLogicalDevice* ldev, HeroMaterialSetup* setup, HeroMaterialId* id_out);
HeroResult hero_material_deinit(HeroLogicalDevice* ldev, HeroMaterialId id);
HeroResult hero_material_get(HeroLogicalDevice* ldev, HeroMaterialId id, HeroMaterial** out);

HeroResult hero_material_set_shader_globals(HeroLogicalDevice* ldev, HeroMaterialId id, HeroShaderGlobalsId shader_globals_id);
HeroResult hero_material_set_descriptor(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data);
HeroResult hero_material_set_sampler(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroSamplerId sampler_id);
HeroResult hero_material_set_image(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id);
HeroResult hero_material_set_image_sampler(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id, HeroSamplerId sampler_id);
HeroResult hero_material_set_uniform_buffer(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset);
HeroResult hero_material_set_storage_buffer(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset);
HeroResult hero_material_update(HeroLogicalDevice* ldev, HeroMaterialId id);

// ===========================================
//
//
// Swapchain
//
//
// ===========================================

typedef struct HeroSwapchain HeroSwapchain;
struct HeroSwapchain {
	HeroWindowId window_id;
	U16 array_layers_count;
	B8 vsync;
	B8 fifo;

	HeroImageId* image_ids;
	U32 images_count;
	U32 width;
	U32 height;
};

typedef struct HeroSwapchainSetup HeroSwapchainSetup;
struct HeroSwapchainSetup {
	HeroWindowId window_id;
	U16 array_layers_count;
	B8 vsync;
	B8 fifo;
};

HeroResult hero_swapchain_init(HeroLogicalDevice* ldev, HeroSwapchainSetup* setup, HeroSwapchainId* id_out, HeroSwapchain** out);
HeroResult hero_swapchain_deinit(HeroLogicalDevice* ldev, HeroSwapchainId id);
HeroResult hero_swapchain_get(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain** out);

HeroResult hero_swapchain_next_image(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain** swapchain_out, U32* next_image_idx_out);

// ===========================================
//
//
// Command Pool
//
//
// ===========================================

typedef struct HeroCommandPoolSetup HeroCommandPoolSetup;
struct HeroCommandPoolSetup {
	bool support_static;
	U32 command_buffers_cap;
};

HeroResult hero_command_pool_init(HeroLogicalDevice* ldev, HeroCommandPoolSetup* setup, HeroCommandPoolId* id_out);
HeroResult hero_command_pool_deinit(HeroLogicalDevice* ldev, HeroCommandPoolId id);
HeroResult hero_command_pool_reset(HeroLogicalDevice* ldev, HeroCommandPoolId id);

// ===========================================
//
//
// Command Buffer
//
//
// ===========================================

typedef struct HeroCommandRecorder HeroCommandRecorder;
struct HeroCommandRecorder {
	HeroLogicalDevice* ldev;
	HeroCommandPoolId command_pool_id;
	HeroRenderPassId render_pass_id;
	HeroMaterialId material_id;
	bool is_static;
};

//
// destroy static buffers or recorded buffers that where never used
HeroResult hero_command_buffer_deinit(HeroLogicalDevice* ldev, HeroCommandPoolId pool_id, HeroCommandPoolBufferId buffer_id);

HeroResult hero_command_recorder_start(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandRecorder** out);
HeroResult hero_command_recorder_start_static(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandRecorder** out);
HeroResult hero_command_recorder_end(HeroCommandRecorder* command_recorder, HeroCommandPoolBufferId* id_out);

HeroResult hero_cmd_render_pass_start(HeroCommandRecorder* command_recorder, HeroRenderPassId render_pass_id, HeroFrameBufferId frame_buffer_id, HeroViewport* viewport, HeroUAabb* scissor);
HeroResult hero_cmd_render_pass_end(HeroCommandRecorder* command_recorder);

HeroResult hero_cmd_draw_start(HeroCommandRecorder* command_recorder, HeroMaterialId material_id);
HeroResult hero_cmd_draw_end_vertexed(HeroCommandRecorder* command_recorder, U32 vertices_start_idx, U32 vertices_count);
HeroResult hero_cmd_draw_end_indexed(HeroCommandRecorder* command_recorder, HeroBufferId index_buffer_id, U32 indices_start_idx, U32 indices_count, U32 vertices_start_idx);

HeroResult hero_cmd_draw_set_vertex_buffer(HeroCommandRecorder* command_recorder, HeroBufferId buffer_id, U32 binding, U64 offset);

HeroResult hero_cmd_draw_set_push_constants(HeroCommandRecorder* command_recorder, void* data, U32 offset, U32 size);
HeroResult hero_cmd_draw_set_instances(HeroCommandRecorder* command_recorder, U32 instances_start_idx, U32 instances_count);

HeroResult hero_cmd_draw_set_sampler(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, HeroSamplerId sampler_id);
HeroResult hero_cmd_draw_set_texture(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, HeroImageId texture_id);
HeroResult hero_cmd_draw_set_uniform_buffer(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset);
HeroResult hero_cmd_draw_set_storage_buffer(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset);
HeroResult hero_cmd_draw_set_dynamic_descriptor_offset(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, U32 dynamic_offset);

HeroResult hero_cmd_compute_dispatch(HeroCommandRecorder* command_recorder, HeroShaderId compute_shader_id, HeroShaderGlobalsId shader_globals_id, U32 group_count_x, U32 group_count_y, U32 group_count_z);

// ===========================================
//
//
// Render State
//
//
// ===========================================

typedef U8 HeroPrimitiveTopology;
enum {
	HERO_PRIMITIVE_TOPOLOGY_POINT_LIST,
	HERO_PRIMITIVE_TOPOLOGY_LINE_LIST,
	HERO_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
	HERO_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
	HERO_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
	HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
	HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
	HERO_PRIMITIVE_TOPOLOGY_PATCH_LIST,
};

typedef U8 HeroRasterizationFlags;
enum {
	HERO_RASTERIZATION_FLAGS_ENABLE_DEPTH_CLAMP = 0x1,
	HERO_RASTERIZATION_FLAGS_ENABLE_DISCARD     = 0x2,
	HERO_RASTERIZATION_FLAGS_ENABLE_DEPTH_BIAS  = 0x4,
};

typedef U8 HeroPolygonMode;
enum {
	HERO_POLYGON_MODE_FILL,
	HERO_POLYGON_MODE_LINE,
	HERO_POLYGON_MODE_POINT,
};

typedef U8 HeroCullModeFlags;
enum {
	HERO_CULL_MODE_FLAGS_NONE           = 0x0,
	HERO_CULL_MODE_FLAGS_FRONT          = 0x1,
	HERO_CULL_MODE_FLAGS_BACK           = 0x2,
	HERO_CULL_MODE_FLAGS_FRONT_AND_BACK = HERO_CULL_MODE_FLAGS_FRONT | HERO_CULL_MODE_FLAGS_BACK,
};

typedef U8 HeroFrontFace;
enum {
	HERO_FRONT_FACE_COUNTER_CLOCKWISE,
	HERO_FRONT_FACE_CLOCKWISE,
};

typedef U8 HeroMultisampleFlags;
enum {
	HERO_MULTISAMPLE_FLAGS_ENABLE_SAMPLE_SHADING    = 0x1,
	HERO_MULTISAMPLE_FLAGS_ENABLE_ALPHA_TO_COVERAGE = 0x2,
	HERO_MULTISAMPLE_FLAGS_ENABLE_ALPHA_TO_ONE      = 0x4,
};

typedef U32 HeroSampleMask;

typedef U8 HeroDepthStencilFlags;
enum {
	HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_TEST = 0x1,
	HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_WRITE = 0x2,
	HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_BOUNDS_TEST = 0x4,
	HERO_DEPTH_STENCIL_FLAGS_ENABLE_STENCIL_TEST = 0x8,
};

typedef U8 HeroStencilOp;
enum {
	HERO_STENCIL_OP_KEEP,
	HERO_STENCIL_OP_ZERO,
	HERO_STENCIL_OP_REPLACE,
	HERO_STENCIL_OP_INCREMENT_AND_CLAMP,
	HERO_STENCIL_OP_DECREMENT_AND_CLAMP,
	HERO_STENCIL_OP_INVERT,
	HERO_STENCIL_OP_INCREMENT_AND_WRAP,
	HERO_STENCIL_OP_DECREMENT_AND_WRAP,
};

typedef struct HeroStencilOpState HeroStencilOpState;
struct HeroStencilOpState {
	HeroStencilOp fail_op;
	HeroStencilOp pass_op;
	HeroStencilOp depth_fail_op;
	HeroCompareOp compare_op;
	U32           compare_mask;
	U32           write_mask;
	U32           reference;
};

typedef U8 HeroBlendFactor;
enum {
	HERO_BLEND_FACTOR_ZERO,
	HERO_BLEND_FACTOR_ONE,
	HERO_BLEND_FACTOR_SRC_COLOR,
	HERO_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	HERO_BLEND_FACTOR_DST_COLOR,
	HERO_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	HERO_BLEND_FACTOR_SRC_ALPHA,
	HERO_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	HERO_BLEND_FACTOR_DST_ALPHA,
	HERO_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	HERO_BLEND_FACTOR_CONSTANT_COLOR,
	HERO_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	HERO_BLEND_FACTOR_CONSTANT_ALPHA,
	HERO_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	HERO_BLEND_FACTOR_SRC_ALPHA_SATURATE,
	HERO_BLEND_FACTOR_SRC1_COLOR,
	HERO_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	HERO_BLEND_FACTOR_SRC1_ALPHA,
	HERO_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
};

typedef U8 HeroBlendOp;
enum {
	HERO_BLEND_OP_ADD,
	HERO_BLEND_OP_SUBTRACT,
	HERO_BLEND_OP_REVERSE_SUBTRACT,
	HERO_BLEND_OP_MIN,
	HERO_BLEND_OP_MAX,
};

typedef U8 HeroColorComponentFlags;
enum {
	HERO_COLOR_COMPONENT_FLAGS_R = 0x1,
	HERO_COLOR_COMPONENT_FLAGS_G = 0x2,
	HERO_COLOR_COMPONENT_FLAGS_B = 0x4,
	HERO_COLOR_COMPONENT_FLAGS_A = 0x8,
	HERO_COLOR_COMPONENT_FLAGS_ALL = 0xf,
};

typedef U8 HeroLogicOp;
enum {
	HERO_LOGIC_OP_CLEAR,
	HERO_LOGIC_OP_AND,
	HERO_LOGIC_OP_AND_REVERSE,
	HERO_LOGIC_OP_COPY,
	HERO_LOGIC_OP_AND_INVERTED,
	HERO_LOGIC_OP_NO_OP,
	HERO_LOGIC_OP_XOR,
	HERO_LOGIC_OP_OR,
	HERO_LOGIC_OP_NOR,
	HERO_LOGIC_OP_EQUIVALENT,
	HERO_LOGIC_OP_INVERT,
	HERO_LOGIC_OP_OR_REVERSE,
	HERO_LOGIC_OP_COPY_INVERTED,
	HERO_LOGIC_OP_OR_INVERTED,
	HERO_LOGIC_OP_NAND,
	HERO_LOGIC_OP_SET,
};

typedef struct HeroRenderStateRasterization HeroRenderStateRasterization;
struct HeroRenderStateRasterization {
	HeroRasterizationFlags flags;
	HeroPolygonMode        polygon_mode;
	HeroCullModeFlags      cull_mode_flags;
	HeroFrontFace          front_face;
	F32                    depth_bias_constant_factor;
	F32                    depth_bias_clamp;
	F32                    depth_bias_slope_factor;
	F32                    line_width;
};

typedef struct HeroRenderStateMultisample HeroRenderStateMultisample;
struct HeroRenderStateMultisample {
	HeroSampleMask*      sample_mask;
	F32                  min_sample_shading;
	HeroMultisampleFlags flags;
	HeroSampleCount      rasterization_samples_count;
};

typedef struct HeroRenderStateDepthStencil HeroRenderStateDepthStencil;
struct HeroRenderStateDepthStencil {
	HeroDepthStencilFlags flags;
	HeroCompareOp         depth_compare_op;
	HeroStencilOpState    front;
	HeroStencilOpState    back;
};

typedef struct HeroRenderStateBlendAttachment HeroRenderStateBlendAttachment;
struct HeroRenderStateBlendAttachment {
	bool                    blend_enable;
	HeroBlendFactor         src_color_blend_factor;
	HeroBlendFactor         dst_color_blend_factor;
	HeroBlendOp             color_blend_op;
	HeroBlendFactor         src_alpha_blend_factor;
	HeroBlendFactor         dst_alpha_blend_factor;
	HeroBlendOp             alpha_blend_op;
	HeroColorComponentFlags color_write_mask;
};

typedef struct HeroRenderStateBlend HeroRenderStateBlend;
struct HeroRenderStateBlend {
	HeroRenderStateBlendAttachment* attachments;
	U32                             attachments_count;
	bool                            enable_logic_op;
	HeroLogicOp                     logic_op;
	F32                             blend_constants[4];
};

struct HeroRenderState {
	bool                         enable_primitive_restart;
	HeroPrimitiveTopology        topology;
	U32                          tessellation_patch_control_points;

	U32                          viewports_count;

	HeroRenderStateRasterization rasterization;
	HeroRenderStateMultisample   multisample;
	HeroRenderStateDepthStencil  depth_stencil;
	HeroRenderStateBlend         blend;
};

// ===========================================
//
//
// Render Graph
//
//
// ===========================================

typedef U16 HeroImageEnum;
typedef U16 HeroBufferEnum;
typedef U16 HeroPassEnum;
typedef U16 HeroPassImageInputEnum;
typedef U16 HeroPassBufferInputEnum;
typedef U16 HeroPassImageOutputEnum;
typedef U16 HeroPassBufferOutputEnum;

#define HERO_IMAGE_ENUM_INVALID              ((U16)-1)
#define HERO_BUFFER_ENUM_INVALID             ((U16)-1)
#define HERO_PASS_ENUM_INVALID               ((U16)-1)
#define HERO_PASS_IMAGE_INPUT_ENUM_INVALID   ((U16)-1)
#define HERO_PASS_BUFFER_INPUT_ENUM_INVALID  ((U16)-1)
#define HERO_PASS_IMAGE_OUTPUT_ENUM_INVALID  ((U16)-1)
#define HERO_PASS_BUFFER_OUTPUT_ENUM_INVALID ((U16)-1)
#define HERO_ATTACHMENT_IDX_INVALID          ((U16)-1)

typedef U8 HeroImageInfoFlags;
enum {
	HERO_IMAGE_INFO_FLAGS_PERSISTENT = 0x1,
	HERO_IMAGE_INFO_FLAGS_READBACK =   0x2,

	HERO_IMAGE_INFO_FLAGS_IS_USED =  0x4, // is set after hero_render_graph_init if image is read from or set to readback and written too.
};

struct HeroImageInfo {
	char* debug_name;

	// if this is set, the following fields are ignored since we get them from the swapchain directly:
	//     width, height, array_layers_count, mip_levels_count, samples_count_log2, image_format
	HeroSwapchainId swapchain_id;

	U32 width;
	U32 height;
	U32 array_layers_count;
	U32 mip_levels_count;
	U32 samples_count_log2;
	HeroImageFormat image_format;
	HeroImageInfoFlags flags;
};

typedef U8 HeroBufferInfoFlags;
enum {
	HERO_BUFFER_INFO_FLAGS_TRANSFER_DST = 0x1,
	HERO_BUFFER_INFO_FLAGS_TRANSFER_SRC = 0x2,
	HERO_BUFFER_INFO_FLAGS_PERSISTENT =   0x4,
	HERO_BUFFER_INFO_FLAGS_READBACK =     0x8,

	HERO_BUFFER_INFO_FLAGS_IS_USED =    0x10, // is set after hero_render_graph_init if buffer is read from or set to readback and written too
};

struct HeroBufferInfo {
	char* debug_name;
	U64 size;
	HeroBufferInfoFlags flags;
};

typedef struct HeroImageInput HeroImageInput;
struct HeroImageInput {
	// the attachment index of the HeroRenderPassLayout.attachments in HeroPassInfo.layout_id
	// can be HERO_ATTACHMENT_IDX_INVALID
	U16                     attachment_idx;

	HeroImageEnum           image_enum;

	HeroPassEnum            pass_enum;              // if is set to HERO_PASS_ENUM_INVALID, then image is must be initialized from the CPU or be persistant.
	HeroPassImageOutputEnum pass_image_output_enum;
};

typedef struct HeroImageOutput HeroImageOutput;
struct HeroImageOutput {
	U32           attachment_idx; // the attachment index of the HeroRenderPassLayout.attachments in HeroPassInfo.layout_id
	HeroImageEnum image_enum;
};

typedef struct HeroBufferInput HeroBufferInput;
struct HeroBufferInput {
	HeroBufferEnum           buffer_enum;
	HeroPassEnum             pass_enum;              // if is set to HERO_PASS_ENUM_INVALID, then image is must be initialized from the CPU or be persistant.
	HeroPassBufferOutputEnum pass_buffer_output_enum;
};

typedef struct HeroBufferOutput HeroBufferOutput;
struct HeroBufferOutput {
	HeroBufferEnum buffer_enum;
};

typedef struct HeroPassInfo HeroPassInfo;
typedef struct HeroImageInfo HeroImageInfo;
typedef struct HeroBufferInfo HeroBufferInfo;

typedef U32 (*HeroPassCmdEstimateFn)(HeroPassInfo* info);
typedef HeroResult (*HeroPassRecordFn)(HeroPassInfo* info, HeroCommandRecorder* command_recorder);

typedef U8 HeroPassInfoFlags;
enum {
	HERO_PASS_INFO_FLAGS_IS_USED = 0x1, // is set after hero_render_graph_init if pass is joined to the graph that outputs to a readback or swapchain resource
};

struct HeroPassInfo {
	char* debug_name;
	HeroRenderPassLayoutId  layout_id; // if NULL {0} then this is a compute pass, otherwise this is a render pass
	HeroPassInfoFlags       flags;
	HeroPassCmdEstimateFn   cmd_estimate_fn;
	HeroPassRecordFn        record_fn;
	void*                   record_userdata;
	HeroPassImageOutputEnum depth_stencil_image_output_enum;
	HeroImageOutput*        image_outputs;
	HeroImageInput*         image_inputs;
	HeroBufferOutput*       buffer_outputs;
	HeroBufferInput*        buffer_inputs;
	U16                     image_outputs_count;
	U16                     image_inputs_count;
	U16                     buffer_outputs_count;
	U16                     buffer_inputs_count;
	U16                     execution_unit_idx;
};

#define HERO_STACK_ELMT_TYPE HeroPassEnum
#include "stack_gen.inl"

typedef U16 HeroRenderGraphErrorType;
enum {
	HERO_RENDER_GRAPH_ERROR_TYPE_NONE,

	//
	// enums[0] = HeroPassEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_RECORDED_FN,
	HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_CMD_ESTIMATE_FN,
	HERO_RENDER_GRAPH_ERROR_TYPE_PASS_INVALID_RENDER_PASS_LAYOUT_ID,
	HERO_RENDER_GRAPH_ERROR_TYPE_PASS_UNUSED,

	//
	// enums[0] = HeroImageEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_UNUSED,

	//
	// enums[0] = HeroBufferEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_UNUSED,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = expected_attachments_count
	// enums[2] = got_attachments_count
	HERO_RENDER_GRAPH_ERROR_TYPE_PASS_ATTACHMENTS_COUNT_MISMATCH,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassImageOutputEnum
	// enums[2] = HeroPassImageOutputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_DUPLICATED_IMAGE,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassImageInputEnum
	// enums[2] = HeroPassImageInputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_DUPLICATED_IMAGE,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassImageInputEnum
	// enums[2] = HeroPassImageOutputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_USED_AS_INPUT_AND_OUTPUT,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassBufferOutputEnum
	// enums[2] = HeroPassBufferOutputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_OUTPUT_DUPLICATED_BUFFER,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassBufferInputEnum
	// enums[2] = HeroPassBufferInputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_DUPLICATED_BUFFER,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassBufferInputEnum
	// enums[2] = HeroPassBufferOutputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_USED_AS_INPUT_AND_OUTPUT,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassImageOutputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_FORMAT_MISMATCH,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_SAMPLES_COUNT_MISMATCH,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_COLOR_IMAGE_FORMAT_FOR_COLOR_ATTACHMENT,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_DEPTH_IMAGE_FORMAT_FOR_DEPTH_ATTACHMENT,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_IMAGE_ENUM_DOES_NOT_EXIST,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_DOES_NOT_EXIST,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_WIDTH_MISMATCH,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_HEIGHT_MISMATCH,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ARRAY_LAYERS_MISMATCH,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassImageInputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_ATTACHMENT_FORMAT_MISMATCH,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_ATTACHMENT_SAMPLES_COUNT_MISMATCH,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_EXPECTED_COLOR_IMAGE_FORMAT_FOR_COLOR_ATTACHMENT,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_EXPECTED_DEPTH_IMAGE_FORMAT_FOR_DEPTH_ATTACHMENT,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_IMAGE_ENUM_DOES_NOT_EXIST,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_ATTACHMENT_DOES_NOT_EXIST,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_CANNOT_BE_SWAPCHAIN,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_ENUM_DOES_NOT_EXIST,
	HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_IMAGE_OUTPUT_ENUM_DOES_NOT_EXIST,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassBufferOutputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_OUTPUT_BUFFER_ENUM_DOES_NOT_EXIST,

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassBufferInputEnum
	HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_BUFFER_ENUM_DOES_NOT_EXIST,
	HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_ENUM_DOES_NOT_EXIST,
	HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_BUFFER_OUTPUT_ENUM_DOES_NOT_EXIST,

	HERO_RENDER_GRAPH_ERROR_TYPE_COUNT,
};

extern const char* hero_render_graph_error_type_fmt_strings[HERO_RENDER_GRAPH_ERROR_TYPE_COUNT];

typedef struct HeroRenderGraphError HeroRenderGraphError;
struct HeroRenderGraphError {
	HeroRenderGraphErrorType type;
	U16 enums[4];
};

typedef struct HeroRenderGraphSetup HeroRenderGraphSetup;
struct HeroRenderGraphSetup {
	char*           debug_name;
	HeroImageInfo*  images;
	HeroBufferInfo* buffers;
	HeroPassInfo*   passes;
	U16             images_count;
	U16             buffers_count;
	U16             passes_count;

	HeroRenderGraphError* errors_out;
	U32 errors_count;
	U32 errors_cap;

	bool            report_unused_error;
};

typedef struct HeroRenderGraph HeroRenderGraph;
struct HeroRenderGraph {
	char*           debug_name;
	HeroImageInfo*  images;
	HeroBufferInfo* buffers;
	HeroPassInfo*   passes;
	HeroPassEnum*   execution_units_passes;
	HeroRangeU16*   execution_units_ranges;
	U16             images_count;
	U16             buffers_count;
	U16             passes_count;
	U16             execution_units_count;
};

HeroResult hero_render_graph_init(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroRenderGraphId* id_out);
HeroResult hero_render_graph_deinit(HeroLogicalDevice* ldev, HeroRenderGraphId id);
HeroResult hero_render_graph_get(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph** out);

HeroResult hero_render_graph_deinit_gpu_resources(HeroLogicalDevice* ldev, HeroRenderGraphId id);
HeroResult hero_render_graph_execute(HeroLogicalDevice* ldev, HeroRenderGraphId id);
void hero_render_graph_print_errors(HeroRenderGraphSetup* setup);
void hero_render_graph_print_execution_units(HeroLogicalDevice* ldev, HeroRenderGraphId id, FILE* f, bool color);
void hero_render_graph_print_graphviz_dot(HeroLogicalDevice* ldev, HeroRenderGraphId id, FILE* f);

// ===========================================
//
//
// Backend
//
//
// ===========================================

typedef HeroResult (*HeroGfxPhysicalDeviceSurfaceImageFormatsSupportedFn)(HeroPhysicalDevice* physical_device, HeroSurface surface, HeroImageFormat* formats, U32 formats_count);

typedef HeroResult (*HeroGfxLogicalDeviceInitFn)(HeroPhysicalDevice* physical_device, HeroLogicalDeviceSetup* setup, HeroLogicalDevice** out);
typedef HeroResult (*HeroGfxLogicalDeviceDeinitFn)(HeroLogicalDevice* ldev);

typedef HeroResult (*HeroLogicalDeviceFrameStartFn)(HeroLogicalDevice* ldev);
typedef HeroResult (*HeroLogicalDeviceQueueTransferFn)(HeroLogicalDevice* ldev);
typedef HeroResult (*HeroLogicalDeviceQueueCommandBuffersFn)(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandPoolBufferId* command_pool_buffer_ids, U32 command_pool_buffers_count);
typedef HeroResult (*HeroLogicalDeviceSubmitFn)(HeroLogicalDevice* ldev, HeroSwapchainId* swapchain_ids, U32 swapchains_count);

typedef HeroResult (*HeroVertexLayoutRegisterFn)(HeroVertexLayout* vl, HeroVertexLayoutId* id_out, HeroVertexLayout** out);
typedef HeroResult (*HeroVertexLayoutDeregisterFn)(HeroVertexLayoutId id, HeroVertexLayout* vertex_layout);
typedef HeroResult (*HeroVertexLayoutGetFn)(HeroVertexLayoutId id, HeroVertexLayout** out);

typedef HeroResult (*HeroBufferInitFn)(HeroLogicalDevice* ldev, HeroBufferSetup* setup, HeroBufferId* id_out, HeroBuffer** out);
typedef HeroResult (*HeroBufferDeinitFn)(HeroLogicalDevice* ldev, HeroBufferId id, HeroBuffer* buffer);
typedef HeroResult (*HeroBufferGetFn)(HeroLogicalDevice* ldev, HeroBufferId id, HeroBuffer** id_out);
typedef HeroResult (*HeroBufferResizeFn)(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 elmts_count);
typedef HeroResult (*HeroBufferMapFn)(HeroLogicalDevice* ldev, HeroBuffer* buffer, void** addr_out);
typedef HeroResult (*HeroBufferReadFn)(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 start_idx, Uptr elmts_count, void* destination);
typedef HeroResult (*HeroBufferWriteFn)(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 start_idx, Uptr elmts_count, void** destination_out);

typedef HeroResult (*HeroImageInitFn)(HeroLogicalDevice* ldev, HeroImageSetup* setup, HeroImageId* id_out, HeroImage** out);
typedef HeroResult (*HeroImageDeinitFn)(HeroLogicalDevice* ldev, HeroImageId id, HeroImage* image);
typedef HeroResult (*HeroImageGetFn)(HeroLogicalDevice* ldev, HeroImageId id, HeroImage** out);
typedef HeroResult (*HeroImageResizeFn)(HeroLogicalDevice* ldev, HeroImage* image, U32 width, U32 height, U32 depth, U32 mip_levels, U32 array_layers_count);
typedef HeroResult (*HeroImageMapFn)(HeroLogicalDevice* ldev, HeroImage* image, void** addr_out);
typedef HeroResult (*HeroImageReadFn)(HeroLogicalDevice* ldev, HeroImage* image, HeroImageArea* area, void* destination);
typedef HeroResult (*HeroImageWriteFn)(HeroLogicalDevice* ldev, HeroImage* image, HeroImageArea* area, void** destination_out);

typedef HeroResult (*HeroSamplerInitFn)(HeroLogicalDevice* ldev, HeroSamplerSetup* setup, HeroSamplerId* id_out, HeroSampler** out);
typedef HeroResult (*HeroSamplerDeinitFn)(HeroLogicalDevice* ldev, HeroSamplerId id, HeroSampler* sampler);
typedef HeroResult (*HeroSamplerGetFn)(HeroLogicalDevice* ldev, HeroSamplerId id, HeroSampler** out);

typedef HeroResult (*HeroGfxShaderMetadataCalculateFn)(HeroLogicalDevice* ldev, HeroShaderMetadataSetup* setup, HeroShaderMetadata** out);

typedef HeroResult (*HeroGfxShaderModuleInitFn)(HeroLogicalDevice* ldev, HeroShaderModuleSetup* setup, HeroShaderModuleId* id_out, HeroShaderModule** out);
typedef HeroResult (*HeroGfxShaderModuleDeinitFn)(HeroLogicalDevice* ldev, HeroShaderModuleId id, HeroShaderModule* shader_module);
typedef HeroResult (*HeroGfxShaderModuleGetFn)(HeroLogicalDevice* ldev, HeroShaderModuleId id, HeroShaderModule** out);

typedef HeroResult (*HeroGfxShaderInitFn)(HeroLogicalDevice* ldev, HeroShaderSetup* setup, HeroShaderId* id_out, HeroShader** out);
typedef HeroResult (*HeroGfxShaderDeinitFn)(HeroLogicalDevice* ldev, HeroShaderId id, HeroShader* shader);
typedef HeroResult (*HeroGfxShaderGetFn)(HeroLogicalDevice* ldev, HeroShaderId id, HeroShader** out);

typedef HeroResult (*HeroDescriptorPoolInitFn)(HeroLogicalDevice* ldev, HeroDescriptorPoolSetup* setup, HeroDescriptorPoolId* id_out);
typedef HeroResult (*HeroDescriptorPoolDeinitFn)(HeroLogicalDevice* ldev, HeroDescriptorPoolId id);
typedef HeroResult (*HeroDescriptorPoolResetFn)(HeroLogicalDevice* ldev, HeroDescriptorPoolId id);

typedef HeroResult (*HeroShaderGlobalsInitFn)(HeroLogicalDevice* ldev, HeroShaderGlobalsSetup* setup, HeroShaderGlobalsId* id_out, HeroShaderGlobals** out);
typedef HeroResult (*HeroShaderGlobalsDeinitFn)(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, HeroShaderGlobals* shader_globals);
typedef HeroResult (*HeroShaderGlobalsGetFn)(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, HeroShaderGlobals** out);
typedef HeroResult (*HeroShaderGlobalsSetDescriptorFn)(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data);
typedef HeroResult (*HeroShaderGlobalsUpdateFn)(HeroLogicalDevice* ldev, HeroShaderGlobals* shader_globals);

typedef HeroResult (*HeroRenderPassLayoutInitFn)(HeroLogicalDevice* ldev, HeroRenderPassLayoutSetup* setup, HeroRenderPassLayoutId* id_out, HeroRenderPassLayout** out);
typedef HeroResult (*HeroRenderPassLayoutDeinitFn)(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id, HeroRenderPassLayout* render_pass_layout);
typedef HeroResult (*HeroRenderPassLayoutGetFn)(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id, HeroRenderPassLayout** out);

typedef HeroResult (*HeroRenderPassInitFn)(HeroLogicalDevice* ldev, HeroRenderPassSetup* setup, HeroRenderPassLayout* render_pass_layout, HeroRenderPassId* id_out, HeroRenderPass** out);
typedef HeroResult (*HeroRenderPassDeinitFn)(HeroLogicalDevice* ldev, HeroRenderPassId id, HeroRenderPass* render_pass);
typedef HeroResult (*HeroRenderPassGetFn)(HeroLogicalDevice* ldev, HeroRenderPassId id, HeroRenderPass** out);

typedef HeroResult (*HeroFrameBufferInitFn)(HeroLogicalDevice* ldev, HeroFrameBufferSetup* setup, HeroFrameBufferId* id_out, HeroFrameBuffer** out);
typedef HeroResult (*HeroFrameBufferDeinitFn)(HeroLogicalDevice* ldev, HeroFrameBufferId id, HeroFrameBuffer* frame_buffer);
typedef HeroResult (*HeroFrameBufferGetFn)(HeroLogicalDevice* ldev, HeroFrameBufferId id, HeroFrameBuffer** out);

typedef HeroResult (*HeroPipelineCacheInitFn)(HeroLogicalDevice* ldev, HeroPipelineCacheSetup* setup, HeroPipelineCacheId* id_out);
typedef HeroResult (*HeroPipelineCacheDeinitFn)(HeroLogicalDevice* ldev, HeroPipelineCacheId id);

typedef HeroResult (*HeroPipelineGraphicsInitFn)(HeroLogicalDevice* ldev, HeroPipelineGraphicsSetup* setup, HeroPipelineId* id_out, HeroPipeline** out);
typedef HeroResult (*HeroPipelineDeinitFn)(HeroLogicalDevice* ldev, HeroPipelineId id, HeroPipeline* pipeline);
typedef HeroResult (*HeroPipelineGetFn)(HeroLogicalDevice* ldev, HeroPipelineId id, HeroPipeline** out);

typedef HeroResult (*HeroMaterialInitFn)(HeroLogicalDevice* ldev, HeroMaterialSetup* setup, HeroMaterialId* id_out, HeroMaterial** out);
typedef HeroResult (*HeroMaterialDeinitFn)(HeroLogicalDevice* ldev, HeroMaterialId id, HeroMaterial* material);
typedef HeroResult (*HeroMaterialGetFn)(HeroLogicalDevice* ldev, HeroMaterialId id, HeroMaterial** out);
typedef HeroResult (*HeroMaterialSetDescriptorFn)(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data);
typedef HeroResult (*HeroMaterialUpdateFn)(HeroLogicalDevice* ldev, HeroMaterial* material);

typedef HeroResult (*HeroSwapchainInitFn)(HeroLogicalDevice* ldev, HeroSwapchainSetup* setup, HeroSwapchainId* id_out, HeroSwapchain** out);
typedef HeroResult (*HeroSwapchainDeinitFn)(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain* swapchain);
typedef HeroResult (*HeroSwapchainGetFn)(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain** out);
typedef HeroResult (*HeroSwapchainNextImageFn)(HeroLogicalDevice* ldev, HeroSwapchain* swapchain, U32* next_image_idx_out);

typedef HeroResult (*HeroCommandPoolInitFn)(HeroLogicalDevice* ldev, HeroCommandPoolSetup* setup, HeroCommandPoolId* id_out);
typedef HeroResult (*HeroCommandPoolDeinitFn)(HeroLogicalDevice* ldev, HeroCommandPoolId id);
typedef HeroResult (*HeroCommandPoolResetFn)(HeroLogicalDevice* ldev, HeroCommandPoolId id);

typedef HeroResult (*HeroCommandBufferDeinitFn)(HeroLogicalDevice* ldev, HeroCommandPoolId pool_id, HeroCommandPoolBufferId buffer_id);
typedef HeroResult (*HeroCommandRecorderStartFn)(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, bool is_static, HeroCommandRecorder** out);
typedef HeroResult (*HeroCommandRecorderEndFn)(HeroCommandRecorder* command_recorder, HeroCommandPoolBufferId* id_out);
typedef HeroResult (*HeroCmdRenderPassStartFn)(HeroCommandRecorder* command_recorder, HeroRenderPassId render_pass_id, HeroFrameBufferId frame_buffer_id, HeroViewport* viewport, HeroUAabb* scissor);
typedef HeroResult (*HeroCmdRenderPassEndFn)(HeroCommandRecorder* command_recorder);
typedef HeroResult (*HeroCmdDrawStartFn)(HeroCommandRecorder* command_recorder, HeroMaterialId material_id);
typedef HeroResult (*HeroCmdDrawEndVertexedFn)(HeroCommandRecorder* command_recorder, U32 vertices_start_idx, U32 vertices_count);
typedef HeroResult (*HeroCmdDrawEndIndexedFn)(HeroCommandRecorder* command_recorder, HeroBufferId index_buffer_id, U32 indices_start_idx, U32 indices_count, U32 vertices_start_idx);
typedef HeroResult (*HeroCmdDrawSetVertexBufferFn)(HeroCommandRecorder* command_recorder, HeroBufferId buffer_id, U32 binding, U64 offset);
typedef HeroResult (*HeroCmdDrawSetPushConstantsFn)(HeroCommandRecorder* command_recorder, void* data, U32 offset, U32 size);
typedef HeroResult (*HeroCmdDrawSetInstancesFn)(HeroCommandRecorder* command_recorder, U32 instances_start_idx, U32 instances_count);
typedef HeroResult (*HeroCmdComputeDispatchFn)(HeroCommandRecorder* command_recorder, HeroShaderId compute_shader_id, HeroShaderGlobalsId shader_globals_id, U32 group_count_x, U32 group_count_y, U32 group_count_z);
typedef HeroResult (*HeroRenderGraphInitFn)(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroRenderGraphId* id_out, HeroRenderGraph** out);
typedef HeroResult (*HeroRenderGraphDeinitFn)(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph* render_graph);
typedef HeroResult (*HeroRenderGraphGetFn)(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph** out);

typedef HeroResult (*HeroRenderGraphDeinitGPUResourcesFn)(HeroLogicalDevice* ldev, HeroRenderGraphId id);
typedef HeroResult (*HeroRenderGraphExecuteFn)(HeroLogicalDevice* ldev, HeroRenderGraphId id);

typedef struct HeroGfxBackendVTable HeroGfxBackendVTable;
struct HeroGfxBackendVTable {
	HeroGfxPhysicalDeviceSurfaceImageFormatsSupportedFn physical_device_surface_image_formats_supported;
	HeroGfxLogicalDeviceInitFn                          logical_device_init;
	HeroGfxLogicalDeviceDeinitFn                        logical_device_deinit;
	HeroLogicalDeviceFrameStartFn                       logical_device_frame_start;
	HeroLogicalDeviceQueueTransferFn                    logical_device_queue_transfer;
	HeroLogicalDeviceQueueCommandBuffersFn              logical_device_queue_command_buffers;
	HeroLogicalDeviceSubmitFn                           logical_device_submit;
	HeroVertexLayoutRegisterFn                          vertex_layout_register;
	HeroVertexLayoutDeregisterFn                        vertex_layout_deregister;
	HeroVertexLayoutGetFn                               vertex_layout_get;
	HeroBufferInitFn                                    buffer_init;
	HeroBufferDeinitFn                                  buffer_deinit;
	HeroBufferGetFn                                     buffer_get;
	HeroBufferResizeFn                                  buffer_resize;
	HeroBufferMapFn                                     buffer_map;
	HeroBufferReadFn                                    buffer_read;
	HeroBufferWriteFn                                   buffer_write;
	HeroImageInitFn                                     image_init;
	HeroImageDeinitFn                                   image_deinit;
	HeroImageGetFn                                      image_get;
	HeroImageResizeFn                                   image_resize;
	HeroImageMapFn                                      image_map;
	HeroImageReadFn                                     image_read;
	HeroImageWriteFn                                    image_write;
	HeroSamplerInitFn                                   sampler_init;
	HeroSamplerDeinitFn                                 sampler_deinit;
	HeroSamplerGetFn                                    sampler_get;
	HeroGfxShaderModuleInitFn                           shader_module_init;
	HeroGfxShaderModuleDeinitFn                         shader_module_deinit;
	HeroGfxShaderModuleGetFn                            shader_module_get;
	HeroGfxShaderMetadataCalculateFn                    shader_metadata_calculate;
	HeroGfxShaderInitFn                                 shader_init;
	HeroGfxShaderDeinitFn                               shader_deinit;
	HeroGfxShaderGetFn                                  shader_get;
	HeroDescriptorPoolInitFn                            descriptor_pool_init;
	HeroDescriptorPoolDeinitFn                          descriptor_pool_deinit;
	HeroDescriptorPoolResetFn                           descriptor_pool_reset;
	HeroShaderGlobalsInitFn                             shader_globals_init;
	HeroShaderGlobalsDeinitFn                           shader_globals_deinit;
	HeroShaderGlobalsGetFn                              shader_globals_get;
	HeroShaderGlobalsSetDescriptorFn                    shader_globals_set_descriptor;
	HeroShaderGlobalsUpdateFn                           shader_globals_update;
	HeroRenderPassLayoutInitFn                          render_pass_layout_init;
	HeroRenderPassLayoutDeinitFn                        render_pass_layout_deinit;
	HeroRenderPassLayoutGetFn                           render_pass_layout_get;
	HeroRenderPassInitFn                                render_pass_init;
	HeroRenderPassDeinitFn                              render_pass_deinit;
	HeroRenderPassGetFn                                 render_pass_get;
	HeroFrameBufferInitFn                               frame_buffer_init;
	HeroFrameBufferDeinitFn                             frame_buffer_deinit;
	HeroFrameBufferGetFn                                frame_buffer_get;
	HeroPipelineCacheInitFn                             pipeline_cache_init;
	HeroPipelineCacheDeinitFn                           pipeline_cache_deinit;
	HeroPipelineGraphicsInitFn                          pipeline_graphics_init;
	HeroPipelineDeinitFn                                pipeline_deinit;
	HeroPipelineGetFn                                   pipeline_get;
	HeroMaterialInitFn                                  material_init;
	HeroMaterialDeinitFn                                material_deinit;
	HeroMaterialGetFn                                   material_get;
	HeroMaterialSetDescriptorFn                         material_set_descriptor;
	HeroMaterialUpdateFn                                material_update;
	HeroSwapchainInitFn                                 swapchain_init;
	HeroSwapchainDeinitFn                               swapchain_deinit;
	HeroSwapchainGetFn                                  swapchain_get;
	HeroSwapchainNextImageFn                            swapchain_next_image;
	HeroCommandPoolInitFn                               command_pool_init;
	HeroCommandPoolDeinitFn                             command_pool_deinit;
	HeroCommandPoolResetFn                              command_pool_reset;
	HeroCommandBufferDeinitFn                           command_buffer_deinit;
	HeroCommandRecorderStartFn                          command_recorder_start;
	HeroCommandRecorderEndFn                            command_recorder_end;
	HeroCmdRenderPassStartFn                            cmd_render_pass_start;
	HeroCmdRenderPassEndFn                              cmd_render_pass_end;
	HeroCmdDrawStartFn                                  cmd_draw_start;
	HeroCmdDrawEndVertexedFn                            cmd_draw_end_vertexed;
	HeroCmdDrawEndIndexedFn                             cmd_draw_end_indexed;
	HeroCmdDrawSetVertexBufferFn                        cmd_draw_set_vertex_buffer;
	HeroCmdDrawSetPushConstantsFn                       cmd_draw_set_push_constants;
	HeroCmdDrawSetInstancesFn                           cmd_draw_set_instances;
	HeroCmdComputeDispatchFn                            cmd_compute_dispatch;
	HeroRenderGraphInitFn                               render_graph_init;
	HeroRenderGraphDeinitFn                             render_graph_deinit;
	HeroRenderGraphGetFn                                render_graph_get;
	HeroRenderGraphDeinitGPUResourcesFn                 render_graph_deinit_gpu_resources;
	HeroRenderGraphExecuteFn                            render_graph_execute;
};

// ===========================================
//
//
// Backend: Vulkan
//
//
// ===========================================

#if HERO_VULKAN_ENABLE

#endif // HERO_VULKAN_ENABLE

// ===========================================
//
//
// Gfx System
//
//
// ===========================================

struct HeroGfxSysSetup {
	HeroGfxBackendType backend_type;
	HeroGfxDisplayManagerType display_manager_type;
	const char* application_name;
	U32 vertex_layouts_cap;
};

typedef struct HeroGfxSys HeroGfxSys;
struct HeroGfxSys {
	HeroGfxBackendType backend_type;
	HeroGfxBackendVTable backend_vtable;
	void* physical_devices;
	U32 physical_device_size;
	U32 physical_devices_count;
};

extern HeroGfxSys hero_gfx_sys;

HeroResult hero_gfx_sys_init(HeroGfxSysSetup* setup);

#endif // _HERO_GFX_H_


