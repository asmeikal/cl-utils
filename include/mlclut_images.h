#ifndef __ML_CLUT_IMAGES_H
#define __ML_CLUT_IMAGES_H

#include "mlclut.h"
#include "mlclut_descriptions.h"

cl_mem clut_loadImageFromFile(cl_context context, const char * const filename, int *width, int*height, cl_bool use_float);

void clut_saveImageToFile(const char * const filename, cl_command_queue command_queue, cl_mem image);

cl_mem clut_getDuplicateEmptyImage(cl_context context, cl_mem image);

#endif

