/*!
 @file OpenCL 1.2 Utilities Description Functions
 @author Michele Laurenti
 @updated 2015-12-28
 
 @abstract
 A boring and wide set of functions to pretty-print all kinds of informations that can
 be retrieved from OpenCL platforms, devices, contexts, etc.
 
 @discussion
 The functions offered roughly fit into 4 categories:
 1. Descriptive functions (clut_get_*_Description): they require a parameter of the type
 specified in the function name, and return a pointer to a descriptive string of the 
 given parameter. The pointer doesn't have to be freed.
 2. Typed printer functions (clut_info_print_*): they require a pointer and, for vector 
 values, a size, and print what they find at the pointed location according to the type
 specified in the function name. Most of them use the descriptive functions illustrated 
 above.
 3. Platform/device info print functions (clut_*Info_typedPrint): they require a cl_*_info
 value describing what to print, a pointer to the value to be printed, and the size of this 
 value. They act as "demultiplexers", finding the right typed printer function according to
 the OpenCL documentation.
 4. All/single platform/device info printers (clut_print*Info): these are the functions
 declared in the header, and allow the user to specify a platform/device or a 
 platform/device and an info. The first group prints all available info for the 
 platform/device, the second group prints just the specified info. The first group uses
 the second.
 
 Other functions include the cl_getErrorDescription function.
 */

#include "mlclut_descriptions.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Debug.h>

#include <ArrayUtils.h>
#include <MLUtils.h>

#define DESC_WIDTH		32

#define CHANNEL_DATA_TYPE_WIDTH	36
#define CHANNEL_ORDER_WIDTH	4

#define DEBUG_CLUT_DESC		"ml_openCL_utilities_descriptions"

/*!
 Local function declaration
 */

void clut_platformInfo_typedPrint(const cl_platform_info info, const void * const value, const size_t size);
void clut_deviceInfo_typedPrint(const cl_device_info info, const void * const value, const size_t size);

/* describer functions */
const char * clut_get_CL_PLATFORM_INFO_Description(const cl_platform_info value);
const char * clut_get_CL_DEVICE_INFO_Description(const cl_device_info value);
const char * clut_get_CL_COMMAND_QUEUE_PROPERTIES_Description(const cl_command_queue_properties value);
const char * clut_get_CL_DEVICE_AFFINITY_DOMAIN_Description(const cl_device_affinity_domain value);
const char * clut_get_CL_DEVICE_EXEC_CAPABILITIES_Description(const cl_device_exec_capabilities value);
const char * clut_get_CL_DEVICE_FP_CONFIG_Description(const cl_device_fp_config value);
const char * clut_get_CL_DEVICE_MEM_CACHE_TYPE_Description(const cl_device_mem_cache_type value);
const char * clut_get_CL_DEVICE_LOCAL_MEM_TYPE_Description(const cl_device_local_mem_type value);
const char * clut_get_CL_DEVICE_PARTITION_PROPERTY_Description(const cl_device_partition_property value);
const char * clut_get_CL_DEVICE_TYPE_Description(const cl_device_type value);
const char * clut_get_CL_CHANNEL_ORDER_Description(const cl_channel_order value);
const char * clut_get_CL_CHANNEL_TYPE_Description(const cl_channel_type value);
const char * clut_get_CL_IMAGE_TYPE_Description(const cl_mem_object_type value);

/* standard types */
void clut_info_print_String(const void * const value);
void clut_info_print_Int(const void * const value);
void clut_info_print_Float(const void * const value);
void clut_info_print_Double(const void * const value);
void clut_info_print_SIZE_T(const void * const value);
void clut_info_print_SIZE_T_nanoseconds(const void * const value);
void clut_info_print_SIZE_T_pixels(const void * const value);
/* OpenCL types */
void clut_info_print_CL_BOOL(const void * const value);
void clut_info_print_CL_DEVICE_MEM_CACHE_TYPE(const void * const value);
void clut_info_print_CL_DEVICE_TYPE(const void * const value);
void clut_info_print_CL_UINT(const void * const value);
void clut_info_print_CL_UINT_bits(const void * const value);
void clut_info_print_CL_UINT_bytes(const void * const value);
void clut_info_print_CL_UINT_hertz(const void * const value);
void clut_info_print_CL_ULONG_bytes(const void * const value);
/* platform and device names from IDs */
void clut_info_print_CL_DEVICE_NAME_from_ID(const void * const value);
void clut_info_print_CL_PLATFORM_NAME_from_ID(const void * const value);
/* Bit fields */
void clut_info_print_CL_COMMAND_QUEUE_PROPERTIES(const void * const value);
void clut_info_print_CL_DEVICE_AFFINITY_DOMAIN(const void * const value);
void clut_info_print_CL_DEVICE_EXEC_CAPABILITIES(const void * const value);
void clut_info_print_CL_DEVICE_FP_CONFIG(const void * const value);
void clut_info_print_CL_DEVICE_MEM_CACHE_TYPE(const void * const value);
void clut_info_print_CL_DEVICE_LOCAL_MEM_TYPE(const void * const value);
/* Vectors */
void clut_info_print_CL_DEVICE_PARTITION_PROPERTIES(const void * const value, const size_t size);
void clut_info_print_CL_DEVICE_MAX_WORK_ITEM_SIZES(const void * const value, const size_t size);
/* image format matrix */
void clut_print_CL_IMAGE_FORMAT_matrix(const cl_image_format * const formats, const cl_uint n_formats);

/*!
 Local variables
 */

static const cl_platform_info platform_infos[] = {
	CL_PLATFORM_NAME,
	CL_PLATFORM_VENDOR,
	CL_PLATFORM_PROFILE,
	CL_PLATFORM_VERSION,
	//CL_PLATFORM_EXTENSIONS,
};

static const cl_device_info device_infos[] = {
	// basic info
	CL_DEVICE_NAME,
	CL_DEVICE_TYPE,
	CL_DEVICE_VENDOR,
	CL_DEVICE_VENDOR_ID,
	CL_DEVICE_MAX_CLOCK_FREQUENCY,
	CL_DEVICE_MAX_COMPUTE_UNITS,
	CL_DEVICE_MAX_WORK_GROUP_SIZE,
	CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
	CL_DEVICE_MAX_WORK_ITEM_SIZES,
	// versions
	CL_DEVICE_PROFILE,
	CL_DRIVER_VERSION,
	CL_DEVICE_VERSION,
	CL_DEVICE_OPENCL_C_VERSION,
	// parent device & platforms
	//CL_DEVICE_PARENT_DEVICE,
	CL_DEVICE_PLATFORM,
	// bool stuff
	CL_DEVICE_AVAILABLE,
	CL_DEVICE_COMPILER_AVAILABLE,
	CL_DEVICE_LINKER_AVAILABLE,
	CL_DEVICE_ERROR_CORRECTION_SUPPORT,
	CL_DEVICE_ENDIAN_LITTLE,
	CL_DEVICE_PREFERRED_INTEROP_USER_SYNC,
	CL_DEVICE_PROFILING_TIMER_RESOLUTION,
	// memory
	CL_DEVICE_ADDRESS_BITS,
	CL_DEVICE_HOST_UNIFIED_MEMORY,
	CL_DEVICE_GLOBAL_MEM_SIZE,
	CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
	CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
	CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,
	CL_DEVICE_LOCAL_MEM_SIZE,
	CL_DEVICE_LOCAL_MEM_TYPE,
	CL_DEVICE_PRINTF_BUFFER_SIZE,
	// images
	CL_DEVICE_IMAGE_SUPPORT,
	CL_DEVICE_IMAGE_MAX_ARRAY_SIZE,
	CL_DEVICE_IMAGE_MAX_BUFFER_SIZE,
	CL_DEVICE_IMAGE2D_MAX_HEIGHT,
	CL_DEVICE_IMAGE2D_MAX_WIDTH,
	CL_DEVICE_IMAGE3D_MAX_DEPTH,
	CL_DEVICE_IMAGE3D_MAX_HEIGHT,
	CL_DEVICE_IMAGE3D_MAX_WIDTH,
	CL_DEVICE_MAX_READ_IMAGE_ARGS,
	CL_DEVICE_MAX_WRITE_IMAGE_ARGS,
	// kernel stuff
	CL_DEVICE_MAX_CONSTANT_ARGS,
	CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
	CL_DEVICE_MAX_MEM_ALLOC_SIZE,
	CL_DEVICE_MAX_PARAMETER_SIZE,
	CL_DEVICE_MAX_SAMPLERS,
	CL_DEVICE_MEM_BASE_ADDR_ALIGN,
	CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,
	// partition
	CL_DEVICE_PARTITION_MAX_SUB_DEVICES,
	CL_DEVICE_PARTITION_PROPERTIES,
	CL_DEVICE_PARTITION_AFFINITY_DOMAIN,
	CL_DEVICE_PARTITION_TYPE,
	// vectors
	CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,
	// complex stuff
	//CL_DEVICE_HALF_FP_CONFIG,
	CL_DEVICE_SINGLE_FP_CONFIG,
	CL_DEVICE_DOUBLE_FP_CONFIG,
	CL_DEVICE_QUEUE_PROPERTIES,
	CL_DEVICE_REFERENCE_COUNT,
	CL_DEVICE_EXECUTION_CAPABILITIES,
	CL_DEVICE_BUILT_IN_KERNELS,
	//CL_DEVICE_EXTENSIONS,
};

static const cl_device_affinity_domain cl_device_affinity_domains[] = {
	CL_DEVICE_AFFINITY_DOMAIN_NUMA,
	CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE,
	CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE,
	CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE,
	CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE,
	CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE
};

static const cl_device_exec_capabilities cl_device_exec_capabilities_array[] = {
	CL_EXEC_KERNEL,
	CL_EXEC_NATIVE_KERNEL,
};

static const cl_command_queue_properties cl_command_queue_properties_array[] = {
	CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
	CL_QUEUE_PROFILING_ENABLE,
};

static const cl_device_fp_config cl_device_fp_configs_array[] = {
	CL_FP_DENORM,
	CL_FP_INF_NAN,
	CL_FP_ROUND_TO_NEAREST,
	CL_FP_ROUND_TO_ZERO,
	CL_FP_ROUND_TO_INF,
	CL_FP_FMA,
	CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT,
	CL_FP_SOFT_FLOAT,
};

static const cl_channel_type cl_channel_types[] = {
	CL_SNORM_INT8,
	CL_SNORM_INT16,
	CL_UNORM_INT8,
	CL_UNORM_INT16,
	CL_UNORM_SHORT_565,
	CL_UNORM_SHORT_555,
	CL_UNORM_INT_101010,
	CL_SIGNED_INT8,
	CL_SIGNED_INT16,
	CL_SIGNED_INT32,
	CL_UNSIGNED_INT8,
	CL_UNSIGNED_INT16,
	CL_UNSIGNED_INT32,
	CL_HALF_FLOAT,
	CL_FLOAT,
};

static const cl_channel_order cl_channel_orders[] = {
	CL_R,
	CL_Rx,
	CL_A,
	CL_INTENSITY,
	CL_LUMINANCE,
	CL_RG,
	CL_RGx,
	CL_RA,
	CL_RGB,
	CL_RGBx,
	CL_RGBA,
	CL_ARGB,
	CL_BGRA,
	// Apple stuff I have on my device
	CL_1RGB_APPLE,
	CL_ABGR_APPLE,
	CL_BGR1_APPLE,
	CL_CbYCrY_APPLE,
	CL_YCbYCr_APPLE,
};

/*!
 @functiongroup All info printers
 */

void clut_printPlatformInfos(const cl_platform_id platform)
{
	size_t i;
	for (i = 0; i < ARRAY_LEN(platform_infos); ++i) {
		clut_printPlatformInfo(platform, platform_infos[i]);
	}
}

void clut_printDeviceInfos(const cl_device_id device)
{
	size_t i;
	for (i = 0; i < ARRAY_LEN(device_infos); ++i) {
		clut_printDeviceInfo(device, device_infos[i]);
	}
}

/*!
 @functiongroup Single info printers
 */

void clut_printPlatformInfo(const cl_platform_id platform, const cl_platform_info info)
{
	const char * const fname = "clut_printPlatformInfo";
	size_t size;
	void *result = clut_getPlatformInfo(platform, info, &size);

	if (NULL != result) {
		printf("\t%-*s ", DESC_WIDTH, clut_get_CL_PLATFORM_INFO_Description(info));
		clut_platformInfo_typedPrint(info, result, size);
		printf("\n");
		free(result);
	} else {
		Debug_out(DEBUG_CLUT_DESC, "%s: unable to print platform info '%s'.\n",
			  fname,
			  clut_get_CL_PLATFORM_INFO_Description(info));
	}
}

void clut_printDeviceInfo(const cl_device_id device, const cl_device_info info)
{
	const char * const fname = "clut_printDeviceInfo";
	size_t size;
	void *result = clut_getDeviceInfo(device, info, &size);

	if (NULL != result) {
		printf("\t%-*s ", DESC_WIDTH, clut_get_CL_DEVICE_INFO_Description(info));
		clut_deviceInfo_typedPrint(info, result, size);
		printf("\n");
		free(result);
	} else {
		Debug_out(DEBUG_CLUT_DESC, "%s: unable to print device info '%s'.\n",
			  fname,
			  clut_get_CL_DEVICE_INFO_Description(info));
	}
}

void clut_printDeviceSupportedImageFormats(const cl_device_id device)
{
	const char * const fname = "clut_printDeviceSupportedImageFormats";
	cl_int ret;
	cl_context context = clCreateContext(0, 1, &device, NULL, NULL, &ret);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT_DESC, "%s: failed to create context: %s.\n", fname, clut_getErrorDescription(ret));
		return;
	}

	const cl_mem_object_type image_types[] = {
		CL_MEM_OBJECT_IMAGE1D,
		CL_MEM_OBJECT_IMAGE1D_BUFFER,
		CL_MEM_OBJECT_IMAGE2D,
		CL_MEM_OBJECT_IMAGE3D,
		CL_MEM_OBJECT_IMAGE1D_ARRAY,
		CL_MEM_OBJECT_IMAGE2D_ARRAY,
	};

	cl_uint n_formats;
	cl_image_format *formats;
	size_t i;

	for (i = 0; i < ARRAY_LEN(image_types); ++i) {
		ret = clGetSupportedImageFormats(context, CL_MEM_READ_WRITE, image_types[i], 0, NULL, &n_formats);
		if (!clut_returnSuccess(ret)) {
			Debug_out(DEBUG_CLUT_DESC, "%s: unable to get available image formats: %s.\n", fname, clut_getErrorDescription(ret));
			continue;
		}
		if (0 >= n_formats) {
			Debug_out(DEBUG_CLUT_DESC, "%s: illegal number of formats: %d.\n", fname, n_formats);
			continue;
		}

		formats = calloc(n_formats, sizeof(cl_image_format));
		if (NULL == formats) {
			Debug_out(DEBUG_CLUT_DESC, "%s: calloc failed.\n", fname);
			continue;
		}

		ret = clGetSupportedImageFormats(context, CL_MEM_READ_WRITE, image_types[i], n_formats, formats, &n_formats);
		if (!clut_returnSuccess(ret)) {
			Debug_out(DEBUG_CLUT_DESC, "%s: unable to get available image formats: %s.\n", fname, clut_getErrorDescription(ret));
			free(formats);
			continue;
		}
		if (0 >= n_formats) {
			Debug_out(DEBUG_CLUT_DESC, "%s: illegal number of formats: %d.\n", fname, n_formats);
			free(formats);
			continue;
		}

		printf("\nPrinting matrix for %s.\n", clut_get_CL_IMAGE_TYPE_Description(image_types[i]));
		clut_print_CL_IMAGE_FORMAT_matrix(formats, n_formats);

		free(formats);
	}

	clReleaseContext(context);

}

/*!
 @functiongroup Print function "demultiplexers"
 */

/*!
 @function clut_deviceInfo_typedPrint
 @abstract
 Prints the property retrieved with clGetDeviceInfo and stored in 'result',
 according to its type specified by 'info'.
 @discussion
 The function chooses the right way to display the value stored in 'result' by 
 clGetDeviceInfo, according to the documentation of the OpenCL standard, version 1.2
 @param info
 The cl_device_info value that was passed to clGetDeviceInfo.
 @param result
 The pointer that was passed as 'param_value' to clGetDeviceInfo.
 @param size
 The value that clGetDeviceInfo stored in 'param_value_size_ret'.
 @warning
 Not all cl_device_info values require the 'size' parameter: at the time of writing, 
 it's required only by CL_DEVICE_MAX_WORK_ITEM_SIZES and CL_DEVICE_PARTITION_PROPERTIES.
 */
void clut_deviceInfo_typedPrint(const cl_device_info info,
				const void * const result,
				const size_t size)
{
	switch (info) {
		case CL_DEVICE_BUILT_IN_KERNELS:
		case CL_DEVICE_EXTENSIONS:
		case CL_DEVICE_NAME:
		case CL_DEVICE_OPENCL_C_VERSION:
		case CL_DEVICE_PROFILE:
		case CL_DEVICE_VENDOR:
		case CL_DEVICE_VERSION:
		case CL_DRIVER_VERSION:
			clut_info_print_String(result);
			break;
		case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
		case CL_DEVICE_MAX_PARAMETER_SIZE:
		case CL_DEVICE_MAX_WORK_GROUP_SIZE:
		case CL_DEVICE_PRINTF_BUFFER_SIZE:
			clut_info_print_SIZE_T(result);
			break;
		case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
			clut_info_print_SIZE_T_nanoseconds(result);
			break;
		case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
		case CL_DEVICE_IMAGE2D_MAX_WIDTH:
		case CL_DEVICE_IMAGE3D_MAX_DEPTH:
		case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
		case CL_DEVICE_IMAGE3D_MAX_WIDTH:
		case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
			clut_info_print_SIZE_T_pixels(result);
			break;
		case CL_DEVICE_AVAILABLE:
		case CL_DEVICE_COMPILER_AVAILABLE:
		case CL_DEVICE_ENDIAN_LITTLE:
		case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
		case CL_DEVICE_HOST_UNIFIED_MEMORY:
		case CL_DEVICE_IMAGE_SUPPORT:
		case CL_DEVICE_LINKER_AVAILABLE:
		case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
			clut_info_print_CL_BOOL(result);
			break;
		case CL_DEVICE_MAX_COMPUTE_UNITS:
		case CL_DEVICE_MAX_CONSTANT_ARGS:
		case CL_DEVICE_MAX_READ_IMAGE_ARGS:
		case CL_DEVICE_MAX_SAMPLERS:
		case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
		case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
		case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
		case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
		case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
		case CL_DEVICE_REFERENCE_COUNT:
		case CL_DEVICE_VENDOR_ID:
			clut_info_print_CL_UINT(result);
			break;
		case CL_DEVICE_ADDRESS_BITS:
			clut_info_print_CL_UINT_bits(result);
			break;
		case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
		case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
		case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
			clut_info_print_CL_UINT_bytes(result);
			break;
		case CL_DEVICE_MAX_CLOCK_FREQUENCY:
			clut_info_print_CL_UINT_hertz(result);
			break;
		case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
		case CL_DEVICE_GLOBAL_MEM_SIZE:
		case CL_DEVICE_LOCAL_MEM_SIZE:
			clut_info_print_CL_ULONG_bytes(result);
			break;
		case CL_DEVICE_QUEUE_PROPERTIES:
			clut_info_print_CL_COMMAND_QUEUE_PROPERTIES(result);
			break;
		case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
			clut_info_print_CL_DEVICE_AFFINITY_DOMAIN(result);
			break;
		case CL_DEVICE_EXECUTION_CAPABILITIES:
			clut_info_print_CL_DEVICE_EXEC_CAPABILITIES(result);
			break;
		case CL_DEVICE_DOUBLE_FP_CONFIG:
		case CL_DEVICE_HALF_FP_CONFIG:
		case CL_DEVICE_SINGLE_FP_CONFIG:
			clut_info_print_CL_DEVICE_FP_CONFIG(result);
			break;
		case CL_DEVICE_MAX_WORK_ITEM_SIZES:
			clut_info_print_CL_DEVICE_MAX_WORK_ITEM_SIZES(result, size);
			break;
		case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
			clut_info_print_CL_DEVICE_MEM_CACHE_TYPE(result);
			break;
		case CL_DEVICE_LOCAL_MEM_TYPE:
			clut_info_print_CL_DEVICE_LOCAL_MEM_TYPE(result);
			break;
		case CL_DEVICE_PARENT_DEVICE:
			clut_info_print_CL_DEVICE_NAME_from_ID(result);
			break;
		case CL_DEVICE_PARTITION_PROPERTIES:
			clut_info_print_CL_DEVICE_PARTITION_PROPERTIES(result, size);
			break;
		case CL_DEVICE_TYPE:
			clut_info_print_CL_DEVICE_TYPE(result);
			break;
		case CL_DEVICE_PLATFORM:
			clut_info_print_CL_PLATFORM_NAME_from_ID(result);
			break;
		case CL_DEVICE_PARTITION_TYPE:
			printf("[PRINT NOT IMPLEMENTED]");
			break;
		default:
			printf("UNKNOWN DEVICE INFO");
	}
}

/*!
 @function clut_platformInfo_typedPrint
 @abstract
 Prints the property retrieved with clGetPlatformInfo and stored in 'result',
 according to its type specified by 'info'.
 @discussion
 The function chooses the right way to display the value stored in 'result' by
 clGetPlatformInfo, according to the documentation of the OpenCL standard, version 1.2
 @param info
 The cl_platform_info value that was passed to clGetPlatformInfo.
 @param result
 The pointer that was passed as 'param_value' to clGetPlatformInfo.
 @param size
 The value that clGetPlatformInfo stored in 'param_value_size_ret'.
 @warning
 Right now, 'size' is not used.
 */
void clut_platformInfo_typedPrint(const cl_platform_info info,
				  const void * const result,
				  const size_t size)
{
	UNUSED(size);
	switch (info) {
		case CL_PLATFORM_PROFILE:
		case CL_PLATFORM_VERSION:
		case CL_PLATFORM_NAME:
		case CL_PLATFORM_VENDOR:
		case CL_PLATFORM_EXTENSIONS:
			clut_info_print_String(result);
			break;
		default:
			printf("UNKNOWN PLATFORM INFO");
	}
}

/*!
 @functiongroup Basic print functions
 */

/*!
 @function clut_info_print_String
 @abstract
 Prints 'value' as a String (char *). The empty string is displayed as "N.A.".
 @param value
 A pointer to a NULL terminated vector of chars.
 */
void clut_info_print_String(const void * const value)
{
	const char * const str = (const char * const) value;
	if (0 >= strlen(str)) {
		// the string's the empty string
		printf("N.A.");
	} else {
		printf("%s", str);
	}
}

/*!
 @function clut_info_print_Int
 @abstract
 Prints the memory pointed by 'value' as an int.
 @param value
 Pointer to the int that will be printed.
 */
void clut_info_print_Int(const void * const value)
{
	printf("%d", *((const int * const) value));
}

/*!
 @function clut_info_print_Float
 @abstract
 Prints the memory pointed by 'value' as a float.
 @param value
 Pointer to the float that will be printed.
 */
void clut_info_print_Float(const void * const value)
{
	printf("%f", *((const float * const) value));
}

/*!
 @function clut_info_print_Double
 @abstract
 Prints the memory pointed by 'value' as a double.
 @param value
 Pointer to the double that will be printed.
 */
void clut_info_print_Double(const void * const value)
{
	printf("%f", *((const double * const) value));
}

/*!
 @function clut_info_print_SIZE_T
 @abstract
 Prints the memory pointed by 'value' as a size_t.
 @param value
 Pointer to the size_t that will be printed.
 */
void clut_info_print_SIZE_T(const void * const value)
{
	printf("%zu", *((const size_t * const) value));
}

/*!
 @function clut_info_print_SIZE_T_nanoseconds
 @abstract
 Prints the memory pointed by 'value' as a size_t, followed by the "ns" ("nanoseconds") 
 suffix.
 @param value
 Pointer to the size_t that will be printed.
 */
void clut_info_print_SIZE_T_nanoseconds(const void * const value)
{
	printf("%zu ns", *((const size_t * const) value));
}

/*!
 @function clut_info_print_SIZE_T_pixels
 @abstract
 Prints the memory pointed by 'value' as a size_t, followed by the "pixels" suffix.
 @param value
 Pointer to the size_t that will be printed.
 */
void clut_info_print_SIZE_T_pixels(const void * const value)
{
	printf("%zu pixels", *((const size_t * const) value));
}

/*!
 @function clut_info_print_CL_BOOL
 @abstract
 Prints the memory pointed by 'value' as a cl_bool. The value is printed as "FALSE" if it
 equals CL_FALSE, as "TRUE" otherwise.
 @param value
 Pointer to the cl_bool that will be printed.
 */
void clut_info_print_CL_BOOL(const void * const value)
{
	printf("%s", (CL_FALSE == *((const cl_bool * const) value)) ? "FALSE" : "TRUE");
}

/*!
 @function clut_info_print_CL_UINT
 @abstract
 Prints the memory pointed by 'value' as a cl_uint.
 @param value
 Pointer to the cl_uint that will be printed.
 */
void clut_info_print_CL_UINT(const void * const value)
{
	printf("%u", *((const cl_uint * const) value));
}

/*!
 @function clut_info_print_CL_UINT_bits
 @abstract
 Prints the memory pointed by 'value' as a cl_uint, followed by the "bits" suffix.
 @param value
 Pointer to the cl_uint that will be printed.
 */
void clut_info_print_CL_UINT_bits(const void * const value)
{
	printf("%u bits", *((const cl_uint * const) value));
}

/*!
 @function clut_info_print_CL_UINT_bytes
 @abstract
 Prints the memory pointed by 'value' as a cl_uint representing a byte number. The value is 
 followed by "bytes" if it is less than 1 KB (i.e. 1024U). Otherwise, the nearest unit is 
 found (KB, MB, GB), and the value is shortened and printed accordingly.
 @param value
 Pointer to the cl_uint that will be printed.
 */
void clut_info_print_CL_UINT_bytes(const void * const value)
{
	const char * const bytes_unit[] = { "KB", "MB", "GB", NULL };
	cl_uint bytes = *((const cl_uint * const) value);
	double shorter_bytes = bytes;
	int i = -1;

	while ((shorter_bytes >= 1024) && (i < 3)) {
		shorter_bytes = shorter_bytes / 1024.0;
		++i;
	}

	if (0 <= i) {
		printf("%.2f %s (%u bytes)", shorter_bytes, bytes_unit[i], bytes);
	} else {
		printf("%u bytes", bytes);
	}
}

/*!
 @function clut_info_print_CL_UINT_hertz
 @abstract
 Prints the memory pointed by 'value' as a cl_uint representing a frequency in MhZ. The 
 value is followed by "bytes" if it is less than 1 MhZ (i.e. 1000U). Otherwise, the nearest 
 unit is found (GhZ), and the value is shortened and printed accordingly.
 @param value
 Pointer to the cl_uint that will be printed.
 */
void clut_info_print_CL_UINT_hertz(const void * const value)
{
	const char * const hertz_unit[] = { "MhZ", "GhZ", NULL };
	cl_uint hertz = *((const cl_uint * const) value);
	double shorter_hertz = hertz;
	int i = 0;

	while ((shorter_hertz >= 1000) && (i < 2)) {
		shorter_hertz = shorter_hertz / 1000;
		++i;
	}

	if (0 < i) {
		printf("%.2f %s (%u %s)", shorter_hertz, hertz_unit[i], hertz, hertz_unit[0]);
	} else {
		printf("%u %s", hertz, hertz_unit[0]);
	}
}

/*!
 @function clut_info_print_CL_ULONG_bytes
 @abstract
 Prints the memory pointed by 'value' as a cl_ulong representing a byte number. The value 
 is followed by "bytes" if it is less than 1 KB (i.e. 1024UL). Otherwise, the nearest unit 
 is found (KB, MB, GB, TB, PB), and the value is shortened and printed accordingly.
 @param value
 Pointer to the cl_ulong that will be printed.
 */
void clut_info_print_CL_ULONG_bytes(const void * const value)
{
	const char * const bytes_unit[] = { "KB", "MB", "GB", "TB", "PB", NULL };
	cl_ulong bytes = *((const cl_ulong * const) value);
	double shorter_bytes = bytes;
	int i = -1;

	while ((shorter_bytes >= 1024) && (i < 5)) {
		shorter_bytes = shorter_bytes / 1024.0;
		++i;
	}

	if (0 <= i) {
		printf("%.2f %s (%llu bytes)", shorter_bytes, bytes_unit[i], bytes);
	} else {
		printf("%llu bytes", bytes);
	}
}

/*!
 @functiongroup Specialized print functions
 */

/*!
 @function clut_info_print_CL_DEVICE_TYPE
 @abstract
 Prints what is pointed by 'value' as a cl_device_type, according to its cl_get_Description
 function.
 @param value
 Pointer to the cl_device_type that will be printed.
 */
void clut_info_print_CL_DEVICE_TYPE(const void * const value)
{
	const cl_device_type type = *((const cl_device_type * const) value);
	printf("%s", clut_get_CL_DEVICE_TYPE_Description(type));
}

/*!
 @function clut_info_print_CL_DEVICE_MEM_CACHE_TYPE
 @abstract
 Prints what is pointed by 'value' as a cl_device_mem_cache_type, according to its
 cl_get_Description function.
 @param value
 Pointer to the cl_device_mem_cache_type that will be printed.
 */
void clut_info_print_CL_DEVICE_MEM_CACHE_TYPE(const void * const value)
{
	const cl_device_mem_cache_type type = *((const cl_device_mem_cache_type * const) value);
	printf("%s", clut_get_CL_DEVICE_MEM_CACHE_TYPE_Description(type));
}

/*!
 @function clut_info_print_CL_DEVICE_LOCAL_MEM_TYPE
 @abstract
 Prints what is pointed by 'value' as a cl_device_mem_cache_type, according to its
 cl_get_Description function.
 @param value
 Pointer to the cl_device_mem_cache_type that will be printed.
 */
void clut_info_print_CL_DEVICE_LOCAL_MEM_TYPE(const void * const value)
{
	const cl_device_local_mem_type type = *((const cl_device_local_mem_type * const) value);
	printf("%s", clut_get_CL_DEVICE_LOCAL_MEM_TYPE_Description(type));
}

/*!
 @function clut_info_print_CL_DEVICE_NAME_from_ID
 @abstract
 Treats what is pointed by 'value' as a cl_device_id, and prints its CL_DEVICE_NAME.
 @param value
 Pointer to the cl_device_id that will be printed.
 */
void clut_info_print_CL_DEVICE_NAME_from_ID(const void * const value)
{
	const cl_device_id device = *((const cl_device_id * const) value);
	void *result = clut_getDeviceInfo(device, CL_DEVICE_NAME, NULL);

	if (NULL != result) {
		clut_deviceInfo_typedPrint(CL_DEVICE_NAME, result, 0);
		free(result);
	} else {
		printf("N.A.");
	}
}

/*!
 @function clut_info_print_CL_PLATFORM_NAME_from_ID
 @abstract
 Treats what is pointed by 'value' as a cl_platform_id, and prints its CL_PLATFORM_NAME.
 @param value
 Pointer to the cl_platform_id that will be printed.
 */
void clut_info_print_CL_PLATFORM_NAME_from_ID(const void * const value)
{
	const cl_platform_id platform = *((const cl_platform_id * const) value);
	void *result = clut_getPlatformInfo(platform, CL_PLATFORM_NAME, NULL);

	if (NULL != result) {
		clut_platformInfo_typedPrint(CL_PLATFORM_NAME, result, 0);
		free(result);
	} else {
		printf("N.A.");
	}
}

/*!
 @function clut_info_print_CL_DEVICE_AFFINITY_DOMAIN
 @abstract
 Displays a value obtained from clGetDeviceInfo with 'param_name' set to 
 CL_DEVICE_PARTITION_AFFINITY_DOMAIN.
 @discussion
 The documentation states that the result value is a bitfield of type 
 cl_device_affinity_domain. If no partition domain is supported, the result value is 0.
 We check each if each possible flag shown in the documentation is active, and display the 
 corresponding string if the flag is active.
 @param value
 The pointer that was passed as 'param_value' to clGetDeviceInfo.
 */
void clut_info_print_CL_DEVICE_AFFINITY_DOMAIN(const void * const value)
{
	const cl_device_affinity_domain domain = *((const cl_device_affinity_domain * const) value);

	if (0 == domain) {
		/* "If the device does not support any affinity domains, a value of 0 will be returned" */
		printf("%s", clut_get_CL_DEVICE_AFFINITY_DOMAIN_Description(domain));
		return;
	}
	size_t i;
	int printed;
	for (i = 0, printed = 0; i < ARRAY_LEN(cl_device_affinity_domains); ++i) {
		if (domain & cl_device_affinity_domains[i]) {
			printf((0 < printed) ? ", %s" : "%s",
			       clut_get_CL_DEVICE_AFFINITY_DOMAIN_Description(cl_device_affinity_domains[i]));
			++printed;
		}
	}
}

/*!
 @function clut_info_print_CL_DEVICE_EXEC_CAPABILITIES
 @abstract
 Displays a value obtained from clGetDeviceInfo with 'param_name' set to
 CL_DEVICE_EXECUTION_CAPABILITIES.
 @discussion
 The documentation states that the result value is a bitfield of type
 cl_device_exec_capabilities. We check each if each possible flag shown in the 
 documentation is active, and display the corresponding string if the flag is active.
 @param value
 The pointer that was passed as 'param_value' to clGetDeviceInfo.
 */
void clut_info_print_CL_DEVICE_EXEC_CAPABILITIES(const void * const value)
{
	const cl_device_exec_capabilities cap = *((const cl_device_exec_capabilities * const) value);

	size_t i;
	int printed;
	for (i = 0, printed = 0; i < ARRAY_LEN(cl_device_exec_capabilities_array); ++i) {
		if (cap & cl_device_exec_capabilities_array[i]) {
			printf((0 < printed) ? ", %s" : "%s",
			       clut_get_CL_DEVICE_EXEC_CAPABILITIES_Description(cl_device_exec_capabilities_array[i]));
			++printed;
		}
	}
}

/*!
 @function clut_info_print_CL_COMMAND_QUEUE_PROPERTIES
 @abstract
 Displays a value obtained from clGetDeviceInfo with 'param_name' set to
 CL_DEVICE_QUEUE_PROPERTIES.
 @discussion
 The documentation states that the result value is a bitfield of type
 cl_command_queue_properties. We check each if each possible flag shown in the
 documentation is active, and display the corresponding string if the flag is active.
 @param value
 The pointer that was passed as 'param_value' to clGetDeviceInfo.
 */
void clut_info_print_CL_COMMAND_QUEUE_PROPERTIES(const void * const value)
{
	const cl_command_queue_properties prop = *((const cl_command_queue_properties * const) value);

	size_t i;
	int printed;
	for (i = 0, printed = 0; i < ARRAY_LEN(cl_command_queue_properties_array); ++i) {
		if (prop & cl_command_queue_properties_array[i]) {
			printf((0 < printed) ? ", %s" : "%s",
			       clut_get_CL_COMMAND_QUEUE_PROPERTIES_Description(cl_command_queue_properties_array[i]));
			++printed;
		}
	}
}

/*!
 @function clut_info_print_CL_DEVICE_FP_CONFIG
 @abstract
 Displays a value obtained from clGetDeviceInfo with 'param_name' set to
 CL_DEVICE_SINGLE_FP_CONFIG, CL_DEVICE_DOUBLE_FP_CONFIG, or CL_DEVICE_HALF_FP_CONFIG.
 @discussion
 The documentation states that the result value is a bitfield of type
 cl_device_fp_config. We check each if each possible flag shown in the
 documentation is active, and display the corresponding string if the flag is active.
 If the bitfield is 0, no FP capabilities are available for the selected device.
 @param value
 The pointer that was passed as 'param_value' to clGetDeviceInfo.
 */
void clut_info_print_CL_DEVICE_FP_CONFIG(const void * const value)
{
	const cl_device_fp_config cap = *((const cl_device_fp_config * const) value);

	if (0 == cap) {
		printf("no FP capabilities");
		return;
	}

	size_t i;
	int printed;
	for (i = 0, printed = 0; i < ARRAY_LEN(cl_device_fp_configs_array); ++i) {
		if (cap & cl_device_fp_configs_array[i]) {
			printf((0 < printed) ? ", %s" : "%s",
			       clut_get_CL_DEVICE_FP_CONFIG_Description(cl_device_fp_configs_array[i]));
			++printed;
		}
	}
}

/*!
 @function clut_info_print_CL_DEVICE_MAX_WORK_ITEM_SIZES
 @abstract
 Displays a value obtained from clGetDeviceInfo with 'param_name' set to
 CL_DEVICE_MAX_WORK_ITEM_SIZES.
 @discussion
 The documentation states that the result value is a 'size_t' vector. Thus its length is
 ('size'/sizeof(size_t)) bytes.
 @param value
 The pointer that was passed as 'param_value' to clGetDeviceInfo.
 */
void clut_info_print_CL_DEVICE_MAX_WORK_ITEM_SIZES(const void * const value, const size_t size)
{
	const size_t * const values = ((const size_t * const) value);

	size_t i, printed;
	size_t array_len = size/sizeof(values[0]);

	for (i = 0, printed = 0; i < array_len; ++i) {
		printf((0 < printed) ? ", %zu" : "%zu", values[i]);
		++printed;
	}
}

/*!
 @function clut_info_print_CL_DEVICE_PARTITION_PROPERTIES
 @abstract
 Displays a value obtained from clGetDeviceInfo with 'param_name' set to
 CL_DEVICE_PARTITION_PROPERTIES.
 @discussion
 The documentation states that the result value is a 'cl_device_partition_property' vector. 
 Thus its length is ('size'/sizeof(cl_device_partition_property)) bytes. The documentation 
 also states that if the device does not support partition, the returned value is 0.
 @param value
 The pointer that was passed as 'param_value' to clGetDeviceInfo.
 */
void clut_info_print_CL_DEVICE_PARTITION_PROPERTIES(const void * const value, const size_t size)
{
	const cl_device_partition_property * const values = ((const cl_device_partition_property * const) value);

	size_t i, printed;
	size_t array_len = size/sizeof(values[0]);

	for (i = 0, printed = 0; i < array_len; ++i) {
		printf((0 < printed) ? ", %s" : "%s",
		       clut_get_CL_DEVICE_PARTITION_PROPERTY_Description(values[i]));
		++printed;
	}
}

void clut_print_CL_IMAGE_FORMAT_matrix(const cl_image_format * const formats,
				       const cl_uint n_formats)
{

	size_t i, j, o_size, t_size;
	cl_bool availables[ARRAY_LEN(cl_channel_orders)][ARRAY_LEN(cl_channel_types)] = {{0}};

	o_size = ARRAY_LEN(cl_channel_orders);
	t_size = ARRAY_LEN(cl_channel_types);

	for (i = 0; i < n_formats; ++i) {
		cl_channel_order o = formats[i].image_channel_order;
		cl_channel_type t = formats[i].image_channel_data_type;
		size_t a_o, a_t;
		for (a_o = 0; a_o < o_size; ++a_o) {
			if (o == cl_channel_orders[a_o]) {
				break;
			}
		}
		for (a_t = 0; a_t < t_size; ++a_t) {
			if (t == cl_channel_types[a_t]) {
				break;
			}
		}
		if ((a_o < o_size) && (a_t < t_size)) {
			availables[a_o][a_t] = CL_TRUE;
		}
	}

	printf("%-*.*s ",
	       CHANNEL_DATA_TYPE_WIDTH,
	       CHANNEL_DATA_TYPE_WIDTH,
	       "Data Type");

	for (i = 0; i < o_size; ++i) {
		printf("| %-*.*s ",
		       CHANNEL_ORDER_WIDTH,
		       CHANNEL_ORDER_WIDTH,
		       clut_get_CL_CHANNEL_ORDER_Description(cl_channel_orders[i]));
	}
	printf("\n");

	for (j = 0; j < t_size; ++j) {
		printf("%-*.*s ",
		       CHANNEL_DATA_TYPE_WIDTH,
		       CHANNEL_DATA_TYPE_WIDTH,
		       clut_get_CL_CHANNEL_TYPE_Description(cl_channel_types[j]));

		for (i = 0; i < o_size; ++i) {
			printf("| %-*.*s ",
			       CHANNEL_ORDER_WIDTH,
			       CHANNEL_ORDER_WIDTH,
			       (CL_FALSE == availables[i][j]) ? "" : "x");
		}

		printf("\n");
	}
}

/*!
 Mapper function definitions
 */

const char * clut_getErrorDescription(const cl_int value)
{
	switch (value) {
		case CL_SUCCESS:
			return "success";
		case CL_BUILD_PROGRAM_FAILURE:
			return "program build failed";
		case CL_COMPILER_NOT_AVAILABLE:
			return "compiler not available";
		case CL_DEVICE_NOT_FOUND:
			return "no such device";
		case CL_INVALID_BINARY:
			return "invalid binary";
		case CL_INVALID_BUILD_OPTIONS:
			return "invalid build options";
		case CL_INVALID_COMMAND_QUEUE:
			return "invalid command queue";
		case CL_INVALID_DEVICE:
			return "invalid device";
		case CL_INVALID_DEVICE_TYPE:
			return "invalid device type";
		case CL_INVALID_EVENT_WAIT_LIST:
			return "invalid event wait list";
		case CL_INVALID_KERNEL:
			return "invalid kernel";
		case CL_INVALID_KERNEL_ARGS:
			return "invalid kernel argument(s)";
		case CL_INVALID_GLOBAL_OFFSET:
			return "invalid global offset";
		case CL_INVALID_GLOBAL_WORK_SIZE:
			return "invalid global work size";
//		case CL_INVALID_IMAGE_FORMAT:
//			return "invalid image format";
		case CL_INVALID_IMAGE_SIZE:
			return "invalid image size";
		case CL_INVALID_OPERATION:
			return "invalid operation";
		case CL_INVALID_PLATFORM:
			return "invalid platform";
		case CL_INVALID_PROGRAM_EXECUTABLE:
			return "invalid program executable";
		case CL_INVALID_VALUE:
			return "invalid value";
		case CL_INVALID_WORK_DIMENSION:
			return "invalid work dimension";
		case CL_INVALID_WORK_GROUP_SIZE:
			return "invalid work group size";
		case CL_INVALID_WORK_ITEM_SIZE:
			return "invalid work item size";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			return "failed to allocate memory for image or buffer";
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:
			return "misaligned sub buffer object";
		case CL_OUT_OF_HOST_MEMORY:
			return "failed to allocate resources on host";
		case CL_OUT_OF_RESOURCES:
			return "failed to allocate resources on device";
		default:
			return "UNKNOWN ERROR";
	}
}

/*!
 Info descriptors
 */

/*!
 @function clut_get_CL_PLATFORM_INFO_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_platform_info to describe.
 */
const char * clut_get_CL_PLATFORM_INFO_Description(const cl_platform_info value)
{
	switch (value) {
		case CL_PLATFORM_PROFILE:
			return "OpenCL profile";
		case CL_PLATFORM_VERSION:
			return "OpenCL version";
		case CL_PLATFORM_NAME:
			return "Platform name";
		case CL_PLATFORM_VENDOR:
			return "Vendor";
		case CL_PLATFORM_EXTENSIONS:
			return "Available extensions";
		default:
			return "UNKNOWN INFO";
	}
}

/*!
 @function clut_get_CL_DEVICE_INFO_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_device_info to describe.
 */
const char * clut_get_CL_DEVICE_INFO_Description(const cl_device_info value)
{
	switch (value) {
		case CL_DEVICE_ADDRESS_BITS:
			return "Address space";
		case CL_DEVICE_AVAILABLE:
			return "Device available";
		case CL_DEVICE_BUILT_IN_KERNELS:
			return "Supported builtin kernels";
		case CL_DEVICE_COMPILER_AVAILABLE:
			return "Compiler available";
		case CL_DEVICE_DOUBLE_FP_CONFIG:
			return "Double FP capabilities";
		case CL_DEVICE_ENDIAN_LITTLE:
			// CL_TRUE if little endian
			return "Little endian";
		case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
			return "Error correction available";
		case CL_DEVICE_EXECUTION_CAPABILITIES:
			return "Execution capabilities";
		case CL_DEVICE_EXTENSIONS:
			return "Available extensions";
		case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
			return "Global memory cache size";
		case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
			return "Global memory cache type";
		case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
			return "Global memory cache line size";
		case CL_DEVICE_GLOBAL_MEM_SIZE:
			return "Global memory size";
		case CL_DEVICE_HALF_FP_CONFIG:
			return "Half FP capabilities";
		case CL_DEVICE_HOST_UNIFIED_MEMORY:
			return "Memory unified with host";
		case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
			return "Max 2D image height";
		case CL_DEVICE_IMAGE2D_MAX_WIDTH:
			return "Max 2D image width";
		case CL_DEVICE_IMAGE3D_MAX_DEPTH:
			return "Max 3D image depth";
		case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
			return "Max 3D image height";
		case CL_DEVICE_IMAGE3D_MAX_WIDTH:
			return "Max 3D image width";
		case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
			return "Max image[] size";
		case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
			return "Max 1D image size";
		case CL_DEVICE_IMAGE_SUPPORT:
			return "Image support available";
		case CL_DEVICE_LINKER_AVAILABLE:
			return "Linker available";
		case CL_DEVICE_LOCAL_MEM_SIZE:
			return "Local memory size";
		case CL_DEVICE_LOCAL_MEM_TYPE:
			return "Local memory type";
		case CL_DEVICE_MAX_CLOCK_FREQUENCY:
			return "Max clock frequency";
		case CL_DEVICE_MAX_COMPUTE_UNITS:
			return "Max compute units";
		case CL_DEVICE_MAX_CONSTANT_ARGS:
			return "Max kernel constant args";
		case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
			return "Max constant buffer size";
		case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
			return "Max kernel alloc size";
		case CL_DEVICE_MAX_PARAMETER_SIZE:
			return "Max kernel parameter size";
		case CL_DEVICE_MAX_READ_IMAGE_ARGS:
			return "Max readable images";
		case CL_DEVICE_MAX_SAMPLERS:
			return "Max samplers";
		case CL_DEVICE_MAX_WORK_GROUP_SIZE:
			return "Max work group size";
		case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
			return "Max work item dimensions";
		case CL_DEVICE_MAX_WORK_ITEM_SIZES:
			return "Max work item sizes";
		case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
			return "Max writeable images";
		case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
			return "Largest builtin type size";
		case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
			return "Smallest alignment [DEPRECATED]";
		case CL_DEVICE_NAME:
			return "Device name";
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
			return "Native char[] size";
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
			return "Native double[] size";
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
			return "Native float[] size";
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
			return "Native half[] size";
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
			return "Native int[] size";
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
			return "Native long[] size";
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
			return "Native short[] size";
		case CL_DEVICE_OPENCL_C_VERSION:
			return "OpenCL C version";
		case CL_DEVICE_PARENT_DEVICE:
			return "Parent device";
		case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
			return "Supported partition domains";
		case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
			return "Max sub devices";
		case CL_DEVICE_PARTITION_PROPERTIES:
			return "Supported partition types";
		case CL_DEVICE_PARTITION_TYPE:
			return "Specified partition types";
		case CL_DEVICE_PLATFORM:
			return "Platform";
		case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
			// CL_TRUE if user should sync
			return "Prefers user synchronization";
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
			return "Preferred char[] size";
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
			return "Preferred double[] size";
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
			return "Preferred float[] size";
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
			return "Preferred half[] size";
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
			return "Preferred int[] size";
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
			return "Preferred long[] size";
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
			return "Preferred short[] size";
		case CL_DEVICE_PRINTF_BUFFER_SIZE:
			return "Printf buffer size";
		case CL_DEVICE_PROFILE:
			return "OpenCL profile";
		case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
			return "Profiling timer resolution";
		case CL_DEVICE_QUEUE_PROPERTIES:
			return "Queue enabled properties";
		case CL_DEVICE_REFERENCE_COUNT:
			return "Reference count";
		case CL_DEVICE_SINGLE_FP_CONFIG:
			return "Single FP capabilities";
		case CL_DEVICE_TYPE:
			return "Device type";
		case CL_DEVICE_VENDOR:
			return "Vendor";
		case CL_DEVICE_VENDOR_ID:
			return "Vendor ID";
		case CL_DEVICE_VERSION:
			return "OpenCL version";
		case CL_DRIVER_VERSION:
			return "OpenCL driver version";
		default:
			return "UNKNOWN INFO";
	}
}

/*!
 Other mappers
 */

/*!
 @function clut_get_CL_DEVICE_EXEC_CAPABILITIES_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_device_exec_capabilities to describe.
 */
const char * clut_get_CL_DEVICE_EXEC_CAPABILITIES_Description(const cl_device_exec_capabilities value)
{
	switch (value) {
		case CL_EXEC_KERNEL:
			return "OpenCL C kernels";
		case CL_EXEC_NATIVE_KERNEL:
			return "Native kernels";
		default:
			return "UNKNOWN EXEC CAPABILITY";
	}
}

/*!
 @function clut_get_CL_DEVICE_AFFINITY_DOMAIN_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_device_affinity_domain to describe.
 */
const char * clut_get_CL_DEVICE_AFFINITY_DOMAIN_Description(const cl_device_affinity_domain value)
{
	switch (value) {
		case CL_DEVICE_AFFINITY_DOMAIN_NUMA:
			return "NUMA";
		case CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE:
			return "L4 cache";
		case CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE:
			return "L3 cache";
		case CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE:
			return "L2 cache";
		case CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE:
			return "L1 cache";
		case CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE:
			return "Next Partitionable";
		case 0:
			/* "If the device does not support any affinity domains, a value of 0 will be returned" */
			return "no affinity domain supported";
		default:
			return "UNKNOWN PARTITION DOMAIN";
	}
}

/*!
 @function clut_get_CL_DEVICE_FP_CONFIG_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_device_fp_config to describe.
 */
const char * clut_get_CL_DEVICE_FP_CONFIG_Description(const cl_device_fp_config value)
{
	switch (value) {
		case CL_FP_DENORM:
			return "denorms";
		case CL_FP_INF_NAN:
			return "INF and NaN values";
		case CL_FP_ROUND_TO_NEAREST:
			return "rounding to nearest";
		case CL_FP_ROUND_TO_ZERO:
			return "rounding to zero";
		case CL_FP_ROUND_TO_INF:
			return "rouding to INF";
		case CL_FP_FMA:
			return "fused multiply-add";
		case CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT:
			return "correctly rounded divides and sqrt";
		case CL_FP_SOFT_FLOAT:
			return "software float ops";
		default:
			return "UNKNOWN FP CAPABILITY";
	}
}

/*!
 @function clut_get_CL_DEVICE_PARTITION_PROPERTY_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_device_partition_property to describe.
 */
const char * clut_get_CL_DEVICE_PARTITION_PROPERTY_Description(const cl_device_partition_property value)
{
	switch (value) {
		case CL_DEVICE_PARTITION_EQUALLY:
			return "partition equally";
		case CL_DEVICE_PARTITION_BY_COUNTS:
			return "partition by counts";
		case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
			return "partition by domain";
		case 0:
			/* "If the device does not support any partition types, a value of 0 will be returned." */
			return "no partition type supported";
		default:
			return "UNKNOWN PARTITION PROPERTY";
	}
}

/*!
 @function clut_get_CL_DEVICE_MEM_CACHE_TYPE_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_device_mem_cache_type to describe.
 */
const char * clut_get_CL_DEVICE_MEM_CACHE_TYPE_Description(const cl_device_mem_cache_type value)
{
	switch (value) {
		case CL_NONE:
			return "no cache";
		case CL_READ_ONLY_CACHE:
			return "read only cache";
		case CL_READ_WRITE_CACHE:
			return "read/write cache";
		default:
			return "UNKNOWN CACHE TYPE";
	}
}

/*!
 @function clut_get_CL_DEVICE_LOCAL_MEM_TYPE_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_device_local_mem_type to describe.
 */
const char * clut_get_CL_DEVICE_LOCAL_MEM_TYPE_Description(const cl_device_local_mem_type value)
{
	switch (value) {
		case CL_GLOBAL:
			return "global";
		case CL_LOCAL:
			return "local";
		case CL_NONE:
			return "no memory";
		default:
			return "UNKNOWN MEMORY TYPE";
	}
}

/*!
 @function clut_get_CL_COMMAND_QUEUE_PROPERTIES_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_command_queue_properties to describe.
 */
const char * clut_get_CL_COMMAND_QUEUE_PROPERTIES_Description(const cl_command_queue_properties value)
{
	switch (value) {
		case CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
			return "out of order execution";
		case CL_QUEUE_PROFILING_ENABLE:
			return "profiling";
		default:
			return "UNKNOWN QUEUE PROPERTY";
	}
}

/*!
 @function clut_get_CL_DEVICE_TYPE_Description
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The cl_device_type to describe.
 */
const char * clut_get_CL_DEVICE_TYPE_Description(const cl_device_type value)
{
	switch (value) {
		case CL_DEVICE_TYPE_CPU:
			return "CPU";
		case CL_DEVICE_TYPE_GPU:
			return "GPU";
		case CL_DEVICE_TYPE_ACCELERATOR:
			return "Accelerator";
		case CL_DEVICE_TYPE_DEFAULT:
			return "Default device type";
		case CL_DEVICE_TYPE_CUSTOM:
			return "Custom device";
		default:
			return "UNKNOWN DEVICE TYPE";
	}
}

const char * clut_get_CL_CHANNEL_ORDER_Description(cl_channel_order value)
{
	switch (value) {
		case CL_R:
			return "R";
		case CL_Rx:
			return "Rx";
		case CL_A:
			return "A";
		case CL_INTENSITY:
			return "Intensity";
		case CL_LUMINANCE:
			return "Luminance";
		case CL_RG:
			return "RG";
		case CL_RGx:
			return "RGx";
		case CL_RA:
			return "RA";
		case CL_RGB:
			return "RGB";
		case CL_RGBx:
			return "RGBx";
		case CL_RGBA:
			return "RGBA";
		case CL_ARGB:
			return "ARGB";
		case CL_BGRA:
			return "BGRA";
		case CL_1RGB_APPLE:
			return "1RGB Apple";
		case CL_ABGR_APPLE:
			return "ABGR Apple";
		case CL_BGR1_APPLE:
			return "BGR1 Apple";
		case CL_CbYCrY_APPLE:
			return "CbYCrY Apple";
		case CL_YCbYCr_APPLE:
			return "YCbYCr Apple";
		default:
			return "UNKNOWN CHANNEL ORDER";
	}
}

const char * clut_get_CL_CHANNEL_TYPE_Description(cl_channel_type value)
{
	switch (value) {
		case CL_SNORM_INT8:
			return "normalized signed 8-bit int";
		case CL_SNORM_INT16:
			return "normalized signed 16-bit int";
		case CL_UNORM_INT8:
			return "normalized unsigned 8-bit int";
		case CL_UNORM_INT16:
			return "normalized unsigned 16-bit int";
		case CL_UNORM_SHORT_565:
			return "normalized 5-6-5 3chan RGB";
		case CL_UNORM_SHORT_555:
			return "normalized x-5-5-5 4chan xRGB";
		case CL_UNORM_INT_101010:
			return "normalized x-10-10-10 4chan xRGB";
		case CL_SIGNED_INT8:
			return "un-normalized signed 8-bit int";
		case CL_SIGNED_INT16:
			return "un-normalized signed 16-bit int";
		case CL_SIGNED_INT32:
			return "un-normalized signed 32-bit int";
		case CL_UNSIGNED_INT8:
			return "un-normalized unsigned 8-bit int";
		case CL_UNSIGNED_INT16:
			return "un-normalized unsigned 16-bit int";
		case CL_UNSIGNED_INT32:
			return "un-normalized unsigned 32-bit int";
		case CL_HALF_FLOAT:
			return "16-bit half-float";
		case CL_FLOAT:
			return "single precision float";
		default:
			return "UNKNOWN CHANNEL DATA TYPE";
	}
}

const char * clut_get_CL_IMAGE_TYPE_Description(const cl_mem_object_type value)
{
	switch (value) {
		case CL_MEM_OBJECT_IMAGE1D:
			return "1D image";
		case CL_MEM_OBJECT_IMAGE1D_BUFFER:
			return "1D image buffer";
		case CL_MEM_OBJECT_IMAGE2D:
			return "2D image";
		case CL_MEM_OBJECT_IMAGE3D:
			return "3D image";
		case CL_MEM_OBJECT_IMAGE1D_ARRAY:
			return "1D image[]";
		case CL_MEM_OBJECT_IMAGE2D_ARRAY:
			return "2D image[]";
		default:
			return "UNKNOWN IMAGE FORMAT";
	}
}
