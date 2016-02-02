/**
 * @file
 * @author Michele Laurenti
 * @language c
 */

#include "mlclut.h"
#include "mlclut_descriptions.h"
#include <Debug.h>
#include <Array.h>
#include <LineParser.h>
#include <Vector.h>
#include <ArrayUtils.h>
#include <StringUtils.h>
#include <MLUtils.h>

#include <string.h>
#include <errno.h>
#include <math.h>

#define DEBUG_CLUT	"ml_openCL_utilities"

#define BUILD_OPTS	"-cl-std=CL1.2 " \
			"-cl-kernel-arg-info " \
			"-Werror "

/**
 * Function declaration
 */

static void clut_printDeviceProgramBuildLog(cl_device_id device, cl_program program);

/**
 * Function definition
 */

/*!
 * @function checkReturn
 * Prints a descriptive string of [value].
 * @param value
 * The return value to be checked.
 */
void clut_checkReturn(const char * const function, cl_int value)
{
	Debug_out(DEBUG_CLUT, "Return value of '%s' is '%s' (%d).\n", function, clut_getErrorDescription(value), value);
}

/*!
 * @function returnSuccess
 * Checks if a function returned with success.
 * @param value
 * The return value to be checked.
 */
int clut_returnSuccess(cl_int value)
{
	return CL_SUCCESS == value;
}

/*!
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

/*!
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

/*!
 * @function clut_getDeviceInfo
 * Retrieves [info] from [device], and returns a pointer to it. [size], if
 * not NULL, contains the size of the returned value.
 */
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

/*!
 * @function clut_getPlatformInfo
 * Retrieves [info] from [platform], and returns a pointer to it. [size], if
 * not NULL, contains the size of the returned value.
 */
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

/*!
 * @function clut_createProgramFromFile
 * Creates and builds a cl_program from the name of a openCL C [file].
 * @param context
 * The cl_context that will be associated with the program.
 * @param file
 * The path of the file.
 * @param flags
 * An optional pointer to a string of compile flags.
 * @return
 */
cl_program clut_createProgramFromFile(cl_context context, const char * const file, const char * const flags)
{
	const char * const fname = "clut_createProgramFromFile";
	cl_program program = NULL;
	cl_int ret;
	if (NULL == file) {
		Debug_out(DEBUG_CLUT, "%s: NULL pointer argument.\n", fname);
		goto error;
	}

	/* parse file */
	Array *lines = parseLines_array(file);
	if (NULL == lines) {
		Debug_out(DEBUG_CLUT, "%s: Unable to parse file '%s'.\n", fname, file);
		goto error;
	}

	/* create program */
	program = clCreateProgramWithSource(context,
					    Array_length(lines),
					    (const char **) Array_as_C_array(lines),
					    NULL, &ret);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: unable to create program: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto clean2;
	}
	if (NULL == program) {
		Debug_out(DEBUG_CLUT, "%s: unable to create program: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		goto clean1;
	}
	Debug_out(DEBUG_CLUT, "%s: Program source created.\n", fname);

	char *build_options;
	if (NULL != flags) {
		Debug_out(DEBUG_CLUT, "%s: default options are long %zu, custom options are long %zu.\n", fname, strlen(BUILD_OPTS), strlen(flags));
		build_options = calloc(strlen(BUILD_OPTS) + 1 + strlen(flags) + 1, 1);
		if (NULL == build_options) {
			Debug_out(DEBUG_CLUT, "%s: unable to allocate build options string.\n", fname);
			goto clean2;
		}
		sprintf(build_options, "%s ", BUILD_OPTS);
		sprintf(build_options+(strlen(BUILD_OPTS)), "%s", flags);
	} else {
		build_options = StringUtils_clone(BUILD_OPTS);
		if (NULL == build_options) {
			Debug_out(DEBUG_CLUT, "%s: unable to clone default build options.\n", fname);
			goto clean2;
		}
	}
	Debug_out(DEBUG_CLUT, "%s: Build flags are: '%s'.\n", fname, build_options);
	Debug_out(DEBUG_CLUT, "%s: Build options are at %p.\n", fname, build_options);
	Debug_out(DEBUG_CLUT, "%s: Build options set.\n", fname);

	/* build program */
	// 0, NULL -> don't build for specific devices
	// NULL, NULL -> do not set any callback (thus the function is blocking)
	ret = clBuildProgram(program, 0, NULL, build_options, NULL, NULL);
	if (!clut_returnSuccess(ret)) {
		Debug_out(DEBUG_CLUT, "%s: failed to build program: %s.\n",
			  fname,
			  clut_getErrorDescription(ret));
		clut_printProgramBuildLog(program);
		goto clean3;
	}
	Debug_out(DEBUG_CLUT, "%s: Program built.\n", fname);

	free(build_options);
	Array_free(&lines);
	Debug_out(DEBUG_CLUT, "%s: Vector freed.\n", fname);
	return program;

clean3:	free(build_options);
clean2:	clReleaseProgram(program);
clean1:	Array_free(&lines);
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

/*!
 * @function clut_printDeviceProgramBuildLog
 * Prints the program build log of [program] for [device].
 */
static void clut_printDeviceProgramBuildLog(cl_device_id device, cl_program program)
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


/*!
 * Callback functions
 */

/*!
 * @function clut_createContextCallback
 * A general callback that can be associated with a context. Prints the message and the bytes
 * in [private_info], if there are any.
 * [user_data] is a pointer to a string describing the context.
 */
void clut_contextCallback(const char *errinfo, const void *private_info, size_t private_info_size, void *user_data)
{
	UNUSED(private_info);
	UNUSED(private_info_size);
	// user_data is a const char * with a useful name for the context
	const char *context_name = (const char *) user_data;

	Debug_out(DEBUG_CLUT, "%s: Printing error information.\n", context_name);
	fprintf(stderr, "%s\n", errinfo);

//	if (0 < private_info_size) {
//		Debug_out(DEBUG_CLUT, "%s: printing %zu additional bytes of stuff.", context_name, private_info_size);
//		full_print(private_info, private_info_size);
//	}
}

/*!
 * @function clut_getEventDuration
 * Returns the duration, in seconds, as a double, of [event].
 */
cl_double clut_getEventDuration(cl_event event)
{
	const char * const fname = "clut_getEventDuration";
	cl_ulong start, end;
	cl_int ret;

	ret = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get start time", error);
	ret = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get end time", error);

	if (end < start) {
		Debug_out(DEBUG_CLUT, "%s: Event finished before starting..\n", fname);
		goto error;
	}

	Debug_out(DEBUG_CLUT, "%s: Event started at %lld, ended at %lld.\n", fname, start, end);

	return (cl_double) (end - start) * ((cl_double) 1e-09);

error:	return (cl_double) 0;
}

/*!
 * @function clut_getEventDuration
 * Returns the duration, in seconds, as a double, of [event].
 */
cl_ulong clut_getEventDuration_ns(cl_event event)
{
	const char * const fname = "clut_getEventDuration_ns";
	cl_ulong start, end;
	cl_int ret;

	ret = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get start time", error);
	ret = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get end time", error);

	if (end < start) {
		Debug_out(DEBUG_CLUT, "%s: Event finished before starting..\n", fname);
		goto error;
	}

	Debug_out(DEBUG_CLUT, "%s: Event started at %lld, ended at %lld.\n", fname, start, end);

	return end - start;

error:	return 0;
}




