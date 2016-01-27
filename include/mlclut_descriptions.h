/*!
 @file OpenCL 1.2 Utilities Description Functions
 @author Michele Laurenti
 @updated 2015-12-28
 */

#ifndef mlclut_descriptions_h
#define mlclut_descriptions_h

#include "mlclut.h"

/*!
 @function clut_getErrorDescription
 @abstract
 Returns a descriptive string of 'value'.
 @param value
 The error value to describe.
 */
const char * clut_getErrorDescription(const cl_int value);

/*!
 @function clut_printPlatformInfos
 @abstract
 Prints all info of 'platform'.
 @param platform
 The 'platform' to describe.
 */
void clut_printPlatformInfos(const cl_platform_id platform);

/*!
 @function clut_printDeviceInfos
 @abstract
 Prints all info of 'device'.
 @param device
 The 'device' to describe.
 */
void clut_printDeviceInfos(const cl_device_id device);

/*!
 @function clut_printPlatformInfo
 @abstract
 Prints a descriptive string of the cl_platform_info 'info' from the cl_platform_id
 'platform'.
 @param platform
 The cl_platform_id of the platform from which the info is retrieved.
 @param info
 The cl_platform_info to display.
 */
void clut_printPlatformInfo(const cl_platform_id platform, const cl_platform_info info);

/*!
 @function clut_printDeviceInfo
 @abstract
 Prints a descriptive string of the cl_device_info 'info' from the cl_device_id
 'device'.
 @param device
 The cl_device_id of the device from which the info is retrieved.
 @param info
 The cl_device_info to display.
 */
void clut_printDeviceInfo(const cl_device_id device, const cl_device_info info);

void clut_printDeviceSupportedImageFormats(const cl_device_id device);

/*!
 * Print function declaration
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

#endif /* mlclut_descriptions_h */

