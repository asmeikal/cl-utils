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

#endif /* mlclut_descriptions_h */
