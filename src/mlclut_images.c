#include "mlclut_images.h"

#include <stdlib.h>
#include <stdio.h>

#include <pgm.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image.h>
#include <stb_image_write.h>

#include <StringUtils.h>

/*!
 * @function clut_loadImageFromFile
 * Opens the image at [filename]. Supported image formats are pgm and all the
 * image formats supported by stb_image (github.com/nothings/stb).
 * @param context
 * The context in which the image will be created.
 * @param filename
 * The filename of the image to be opened.
 * @param width
 * A pointer where the width of the image will be stored. It can be NULL.
 * @param height
 * A pointer where the height of the image will be stored. It can be NULL.
 * @retun
 * NULL on failure, or a valid cl_image.
 */
cl_mem clut_loadImageFromFile(cl_context context, const char * const filename, int *width, int *height, cl_bool use_float)
{
	if (NULL == filename) {
		fprintf(stderr, "clut_loadImageFromFile: NULL pointer argument.\n");
	}
	cl_mem result = NULL;
	int l_width, l_height;
	unsigned char *img;
	int ret, is_pgm;
	cl_int cl_ret;

	cl_image_format image_format = {0, 0};
	cl_image_desc image_desc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	/* load image from file into buffer */
	if ((is_pgm = StringUtils_endsWith(filename, "pgm"))) {
		/* pgm image, use demetrescu's micro library */
		ret = pgm_load(&img, &l_height, &l_width, filename);
		if (0 != ret) {
			fprintf(stderr, "Unable to open pgm image '%s'.\n", filename);
			goto error1;
		}
		image_format.image_channel_order = CL_R;
	} else {
		/* use stb for any other format */
		int channel_order;
		img = stbi_load(filename, &l_width, &l_height, &channel_order, 0);
		if (NULL == img) {
			fprintf(stderr, "Unable to open image '%s'.\n", filename);
			goto error1;
		}
		switch (channel_order) {
			case 1:
				image_format.image_channel_order = CL_R;
				break;
			case 2:
				image_format.image_channel_order = CL_RA;
				break;
			case 3:
				/* openCL doesn't like plain RGB images
				 * so, I'll force stb to open RGB images as RGBA */
				stbi_image_free(img);
				img = stbi_load(filename, &l_width, &l_height, &channel_order, 4);
				if (NULL == img) {
					fprintf(stderr, "Unable to open image '%s'.\n", filename);
					goto error1;
				}
				image_format.image_channel_order = CL_RGBA;
				break;
			case 4:
				image_format.image_channel_order = CL_RGBA;
				break;
			default:
				fprintf(stderr, "Unrecognized stb components number %d.\n", channel_order);
				goto error2;
		}
	}
	if (CL_TRUE == use_float) {
		image_format.image_channel_data_type = CL_UNORM_INT8;
	} else {
		image_format.image_channel_data_type = CL_UNSIGNED_INT8;
	}
	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	image_desc.image_width = l_width;
	image_desc.image_height = l_height;

	printf("Opening image with channel order '%s' and data type '%s'.\n",
		clut_get_CL_CHANNEL_ORDER_Description(image_format.image_channel_order),
		clut_get_CL_CHANNEL_TYPE_Description(image_format.image_channel_data_type));

	/* create image */
	result = clCreateImage(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &image_format, &image_desc, img, &cl_ret);
	CLUT_CHECK_ERROR(cl_ret, "Unable to create cl_image", error2);

	/* set width and height */
	if (NULL != width) {
		*width = l_width;
	}
	if (NULL != height) {
		*height = l_height;
	}

error2:
	if (is_pgm) {
		free(img);
	} else {
		stbi_image_free(img);
	}
error1:
	return result;
}

/*!
 * @function clut_saveImageToFile
 * Saves a cl_image object to [filename], with png format.
 * @param filename
 * The filename to save to.
 * @param command_queue
 * A command queue associated with the context in which the image was created.
 * We need it to copy the image to a buffer before exporting.
 * @param image
 * The image to export.
 * @return
 * Nothing.
 */
void clut_saveImageToFile(const char * const filename, cl_command_queue command_queue, cl_mem image)
{
	if (NULL == filename) {
		fprintf(stderr, "clut_loadImageFromFile: NULL pointer argument.\n");
	}
	int ret;
	cl_int cl_ret;
	cl_image_format image_format = {0, 0};
	size_t width, height;
	int components, row_size;
	unsigned char *img;

	/* get image width, height, and format */
	cl_ret = clGetImageInfo(image, CL_IMAGE_WIDTH, sizeof(width), &width, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get image width", error1);
	cl_ret = clGetImageInfo(image, CL_IMAGE_HEIGHT, sizeof(height), &height, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get image height", error1);
	cl_ret = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &image_format, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get image format", error1);

	/* stb_image_write wants all channels to be encoded as chars */
	if (
		(image_format.image_channel_data_type != CL_UNORM_INT8) &&
		(image_format.image_channel_data_type != CL_SNORM_INT8) &&
		(image_format.image_channel_data_type != CL_SIGNED_INT8) &&
		(image_format.image_channel_data_type != CL_UNSIGNED_INT8)
	) {
		fprintf(stderr, "Invalid image channel data type '%s'.\n",
			clut_get_CL_CHANNEL_TYPE_Description(image_format.image_channel_data_type));
		goto error1;
	}

	/* choose correct number of components */
	switch (image_format.image_channel_order) {
		case CL_R:
		case CL_Rx:
		case CL_A:
		case CL_INTENSITY:
		case CL_LUMINANCE:
			components = 1;
			break;
		case CL_RG:
		case CL_RGx:
		case CL_RA:
			components = 2;
			break;
		case CL_RGB:
		case CL_RGBx:
			components = 3;
			break;
		case CL_RGBA:
			components = 4;
			break;
		default:
			fprintf(stderr, "Invalid image channel order '%s'.\n",
				clut_get_CL_CHANNEL_ORDER_Description(image_format.image_channel_order));
			goto error1;
	}
	row_size = width * components;

	/* allocate buffer */
	img = calloc(width * height, components);
	if (NULL == img) {
		fprintf(stderr, "Calloc failed.\n");
		goto error1;
	}

	printf("Saving image with channel order '%s' and data type '%s'.\n",
		clut_get_CL_CHANNEL_ORDER_Description(image_format.image_channel_order),
		clut_get_CL_CHANNEL_TYPE_Description(image_format.image_channel_data_type));

	/* copy image data to buffer */
	const size_t origin[3] = {0, 0, 0};
	const size_t region[3] = {width, height, 1};
	cl_ret = clEnqueueReadImage(command_queue, image, CL_TRUE, origin, region, 0, 0, img, 0, NULL, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Read image failed", error2);
	clFinish(command_queue);
	printf("Image read from device.\n");

	/* save image as png */
	ret = stbi_write_png(filename, (int) width, (int) height, components, img, row_size);
	if (0 == ret) {
		fprintf(stderr, "Write image to file failed.\n");
		goto error2;
	}

error2:
	free(img);
error1:
	return;
}

/*!
 * @clut_getDuplicateEmptyImage
 * Creates an empty cl_image_2d with the same properties as [image].
 * @param context
 * The context in which the image will be created.
 * @param image
 * The image whose params will be copied.
 * @return
 * A valid cl_mem object, or NULL on failure.
 */
cl_mem clut_getDuplicateEmptyImage(cl_context context, cl_mem image)
{
	cl_int cl_ret;
	cl_mem dup_image = NULL;
	size_t width, height;
	cl_image_format image_format = {0, 0};
	cl_image_desc image_desc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	/* get image width, height, and format */
	cl_ret = clGetImageInfo(image, CL_IMAGE_WIDTH, sizeof(width), &width, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get image width", error1);
	cl_ret = clGetImageInfo(image, CL_IMAGE_HEIGHT, sizeof(height), &height, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get image height", error1);
	cl_ret = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &image_format, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get image format", error1);

	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	image_desc.image_width = width;
	image_desc.image_height = height;

	dup_image = clCreateImage(context, CL_MEM_WRITE_ONLY, &image_format, &image_desc, NULL, &cl_ret);
	CLUT_CHECK_ERROR(cl_ret, "Unable to create duplicate image", error1);

error1:
	return dup_image;
}

