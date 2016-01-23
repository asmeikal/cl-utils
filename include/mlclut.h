/**
 * @header
 * @author Michele Laurenti
 * @language c
 */

#ifndef __ML_CLUT_H
#define __ML_CLUT_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "opencl.h"

#define CLUT_CHECK_ERROR(r,s,e)	\
	do { \
		if (!clut_returnSuccess((r))) {\
			fprintf(stderr, "%s: %s.\n", (s), clut_getErrorDescription((r))); \
			goto e; \
		} \
	} while (0)


void clut_checkReturn(const char * const function, cl_int value);

int clut_returnSuccess(cl_int value);

cl_platform_id *clut_getAllPlatforms(cl_uint *n_p);
cl_device_id *clut_getAllDevices(cl_platform_id platform, cl_device_type t, cl_uint *n_d);

void * clut_getDeviceInfo(const cl_device_id device, const cl_device_info info, size_t * const size);
void * clut_getPlatformInfo(const cl_platform_id platform, const cl_platform_info info, size_t * const size);

cl_program clut_createProgramFromFile(cl_context context, const char * const file, const char * const flags);

void clut_printProgramBuildLog(const cl_program program);

void clut_contextCallback(const char *errinfo, const void *private_info, size_t private_info_size, void *user_data);

cl_double clut_getEventDuration(cl_event event);

#endif

