/**
 * @file
 * @author Michele Laurenti
 * @language c
 */

#include "mlclut.h"
#include "mlclut_descriptions.h"
#include <Debug.h>
#include <LineParser.h>
#include <Vector.h>
#include <ArrayUtils.h>

#include <string.h>
#include <errno.h>
#include <math.h>

#define DEBUG_CLUT	"ml_openCL_utilities"

#define BUILD_OPTS	"-cl-std=CL1.2 " \
			"-cl-kernel-arg-info " \
			"-Werror "

void clut_printDeviceProgramBuildLog(cl_device_id device, cl_program program);

/**
 * @function checkReturn
 * Prints a descriptive string of [value].
 * @param value
 * The return value to be checked.
 */
void clut_checkReturn(const char * const function, cl_int value)
{
	Debug_out(DEBUG_CLUT, "Return value of '%s' is '%s' (%d).\n", function, clut_getErrorDescription(value), value);
}

/**
 * @function returnSuccess
 * Checks if a function returned with success.
 * @param value
 * The return value to be checked.
 */
int clut_returnSuccess(cl_int value)
{
	return CL_SUCCESS == value;
}

/**
 * @function clut_getAllPlatforms
 * @warning Result should be manually freed.
 * @param n_p
 * Where to store the total number of platforms.
 */
cl_platform_id *clut_getAllPlatforms(cl_uint *n_p)
{
	const char * const fname = "clut_getAllPlatforms";
	cl_int ret;
	cl_uint n_platforms = 0, n_platforms_check = 0;

	/* count platforms */
	ret = clGetPlatformIDs(0, NULL, &n_platforms);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: %s.\n", fname, clut_getErrorDescription(ret));
		goto error;
	}
	if (0 == n_platforms) {
		Debug_out(DEBUG_CLUT, "%s: no platforms available.\n", fname);
		goto error;
	}

	/* allocate platforms vector */
	cl_platform_id *platforms = calloc(n_platforms, sizeof(cl_platform_id));
	if (NULL == platforms) {
		Debug_out(DEBUG_CLUT, "%s: calloc failed.\n", fname);
		goto error;
	}

	/* fill platform vector */
	ret = clGetPlatformIDs(n_platforms, platforms, &n_platforms_check);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: %s.\n", fname, clut_getErrorDescription(ret));
		goto clean;
	}
	if (n_platforms_check != n_platforms) {
		Debug_out(DEBUG_CLUT, "%s: platform number went from %d to %d.\n", fname, n_platforms, n_platforms_check);
		goto clean;
	}

	/* return vector */
	if (NULL != n_p) {
		*n_p = n_platforms;
	}
	return platforms;

clean:	free(platforms);
error:	return NULL;
}

/**
 * @function clut_getAllDevices
 * @warning Result should be manually freed.
 * @param platform
 * The platform to query.
 * @param t
 * The desired type of devices.
 * @param n_d
 * Where to store the total number of devices.
 */
cl_device_id *clut_getAllDevices(cl_platform_id platform, cl_device_type t, cl_uint *n_d)
{
	const char * const fname = "clut_getAllDevices";
	cl_int ret;
	cl_uint n_devices = 0, n_devices_check = 0;

	/* count devices */
	ret = clGetDeviceIDs(platform, t, 0, NULL, &n_devices);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: %s.\n", fname, clut_getErrorDescription(ret));
		goto error;
	}
	if (0 == n_devices) {
		Debug_out(DEBUG_CLUT, "%s: no devices available.\n", fname);
		goto error;
	}

	/* allocate device vector */
	cl_device_id *devices = calloc(n_devices, sizeof(cl_device_id));
	if (NULL == devices) {
		Debug_out(DEBUG_CLUT, "%s: calloc failed.\n", fname);
		goto error;
	}

	/* fill device vector */
	ret = clGetDeviceIDs(platform, t, n_devices, devices, &n_devices_check);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: %s.\n", fname, clut_getErrorDescription(ret));
		goto clean;
	}
	if (n_devices_check != n_devices) {
		Debug_out(DEBUG_CLUT, "%s: device number went from %d to %d.\n", fname, n_devices, n_devices_check);
		goto clean;
	}

	/* return vector */
	if (NULL != n_d) {
		*n_d = n_devices;
	}
	return devices;

clean:	free(devices);
error:	return NULL;
}

void * clut_getDeviceInfo(const cl_device_id device,
			  const cl_device_info info,
			  size_t * const size)
{
	const char * const fname = "clut_getDeviceInfo";
	cl_int ret;
	size_t res_size, res_size_check;
	char *result;

	ret = clGetDeviceInfo(device, info, 0, NULL, &res_size);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: unable to get device info size: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto error;
	}
	if (0 >= res_size) {
		Debug_out(DEBUG_CLUT, "%s: invalid info size '%zu'.\n",
			  fname,
			  res_size);
		goto error;
	}

	result = calloc(res_size, 1);
	if (NULL == result) {
		Debug_out(DEBUG_CLUT, "%s: calloc failed.\n",
			  fname);
		goto error;
	}

	ret = clGetDeviceInfo(device, info, res_size, result, &res_size_check);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: unable to get device info: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto clean;
	}
	if (res_size != res_size_check) {
		Debug_out(DEBUG_CLUT, "%s: info size changed from '%zu' to '%zu'.\n",
			  fname,
			  res_size,
			  res_size_check);
		goto clean;
	}

	if (NULL != size) {
		*size = res_size;
	}
	return result;

clean:	free(result);
error:	return NULL;
}

void * clut_getPlatformInfo(const cl_platform_id platform,
			    const cl_platform_info info,
			    size_t * const size)
{
	const char * const fname = "clut_getPlatformInfo";
	cl_int ret;
	size_t res_size, res_size_check;
	char *result;

	ret = clGetPlatformInfo(platform, info, 0, NULL, &res_size);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: unable to get platform info size: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto error;
	}
	if (0 >= res_size) {
		Debug_out(DEBUG_CLUT, "%s: invalid info size '%zu'.\n",
			  fname,
			  res_size);
		goto error;
	}

	result = calloc(res_size, 1);
	if (NULL == result) {
		Debug_out(DEBUG_CLUT, "%s: calloc failed.\n",
			  fname);
		goto error;
	}

	ret = clGetPlatformInfo(platform, info, res_size, result, &res_size_check);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: unable to get platform info: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto clean;
	}
	if (res_size != res_size_check) {
		Debug_out(DEBUG_CLUT, "%s: info size changed from '%zu' to '%zu'.\n",
			  fname,
			  res_size,
			  res_size_check);
		goto clean;
	}

	if (NULL != size) {
		*size = res_size;
	}
	return result;

clean:	free(result);
error:	return NULL;
}

/**
 * @function clut_createProgramFromFile
 * Creates and builds a cl_program from the name of a openCL C [file].
 * @param context
 * The cl_context that will be associated with the program.
 * @param file
 * The path of the file.
 * @return
 */
cl_program clut_createProgramFromFile(cl_context context, const char * const file)
{
	const char * const fname = "clut_createProgramFromFile";
	cl_program program;
	cl_int ret;

	/* parse file */
	char **source = parseLines(file);
	if (NULL == source) {
		Debug_out(DEBUG_CLUT, "%s: unable to parse file %s.\n",
			  fname,
			  file);
		goto error;
	}

	/* create program */
	program = clCreateProgramWithSource(context,
					    Vector_length((void **) source),
					    (const char **) source,
					    NULL, &ret);
	if (NULL == program) {
		Debug_out(DEBUG_CLUT, "%s: unable to create program: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto clean1;
	}
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: unable to create program: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto clean2;
	}

	/* build program */
	ret = clBuildProgram(program, 0, NULL, BUILD_OPTS, NULL, NULL);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: failed to build program: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		clut_printProgramBuildLog(program);
		goto clean2;
	}

	Vector_free((void **) source);
	return program;

clean2:	clReleaseProgram(program);
clean1:	Vector_free((void **) source);
error:	return NULL;
}


/*!
 Program build log
 */

/*!
 @function clut_printProgramBuildLog
 Prints the build log of a program.
 @param program
 The cl_program whose build log will be printed.
 */
void clut_printProgramBuildLog(const cl_program program)
{
	const char * const fname = "clut_printProgramBuildLog";

	cl_int ret;
	cl_uint n_devices;
	cl_device_id *devices;

	/* get number of devices associated with the program */
	ret = clGetProgramInfo(program,
			       CL_PROGRAM_NUM_DEVICES,
			       sizeof(n_devices),
			       &n_devices,
			       NULL);

	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT,
			  "%s: unable to fetch device number: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto error;
	}
	if (0 >= n_devices) {
		Debug_out(DEBUG_CLUT,
			  "%s: illegal number of devices (%d).\n",
			  fname,
			  n_devices);
		goto error;
	}

	/* allocate vector of devices */
	//Debug_out(DEBUG_CLUT_DESC, "%s: program has %d device(s).\n", fname, n_devices);
	devices = calloc(n_devices, sizeof(cl_device_id));
	if (NULL == devices) {
		Debug_out(DEBUG_CLUT,
			  "%s: calloc failed.\n",
			  fname);
		goto error;
	}

	/* fill devices vector */
	ret = clGetProgramInfo(program,
			       CL_PROGRAM_DEVICES,
			       n_devices * sizeof(cl_device_id),
			       devices,
			       NULL);

	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT,
			  "%s: unable to fetch devices: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto clean1;
	}

	/* print program build info for each device */
	cl_uint i;
	for (i = 0; i < n_devices; ++i) {
		clut_printDeviceProgramBuildLog(devices[i], program);
	}

clean1: free(devices);
error:	return;
}

void clut_printDeviceProgramBuildLog(cl_device_id device, cl_program program)
{
	const char * const fname = "clut_printDeviceProgramBuildLog";
	cl_int ret;
	size_t log_size, log_size_check;
	char *log;

	/* get log size */
	ret = clGetProgramBuildInfo(program,
				    device,
				    CL_PROGRAM_BUILD_LOG,
				    0,
				    NULL,
				    &log_size);

	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT,
			  "%s: unable to fetch program build log size: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto error;
	}
	if (0 >= log_size) {
		Debug_out(DEBUG_CLUT,
			  "%s: illegal log size (%ld).\n",
			  fname,
			  log_size);
		goto error;
	}

	/* alloc */
	log = calloc(log_size, 1);
	if (NULL == log) {
		Debug_out(DEBUG_CLUT,
			  "%s: calloc failed.\n",
			  fname);
		goto error;
	}

	/* get log */
	ret = clGetProgramBuildInfo(program,
				    device,
				    CL_PROGRAM_BUILD_LOG,
				    log_size,
				    log,
				    &log_size_check);

	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT,
			  "%s: unable to fetch program build log: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto clean;
	}
	if (log_size != log_size_check) {
		Debug_out(DEBUG_CLUT,
			  "%s: log size changed from %ld to %ld.\n",
			  fname,
			  log_size,
			  log_size_check);
		goto clean;
	}

	/* print log */
	printf("Program build log:\n");
	printf("%s", log);
	printf("\n");

clean:	free(log);
error:	return;
}





