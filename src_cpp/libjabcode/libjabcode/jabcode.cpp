/**
 * libjabcode - JABCode Encoding/Decoding Library
 *
 * Copyright 2016 by Fraunhofer SIT. All rights reserved.
 * See LICENSE file for full terms of use and distribution.
 *
 * Contact: Huajian Liu <liu@sit.fraunhofer.de>
 *			Waldemar Berchtold <waldemar.berchtold@sit.fraunhofer.de>
 *			Miles J. Litteral <Mandaloe2@gmail.com> (C++ Refactoring)
 *
 * @file jabcode.cpp
 * @brief Data module masking
 */

#include "pch.h"

namespace jabcode {

jab_int32 MIN(jab_int32 a, jab_int32 b){
	if (a < b) {
		return a;
	}
	else
	{
		return b;
	}
}

jab_int32 MAX(jab_int32 a, jab_int32 b) {
	if (a > b) {
		return a;
	}
	else
	{
		return b;
	}
}

/**
 * @brief Report error message
 * @param message the error message
*/
void reportError(const jab_char* message)
{
	printf("JABCode Error: %s\n", message);
}

/**
 * @brief Swap two variables
*/
void swap(jab_int32* a, jab_int32* b)
{
	jab_int32 tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

/**
 * @brief Get the average and variance of RGB values
 * @param rgb the pixel with RGB values
 * @param ave the average value
 * @param var the variance value
*/
void getAveVar(jab_byte* rgb, jab_double* ave, jab_double* var)
{
	//calculate mean
	*ave = (rgb[0] + rgb[1] + rgb[2]) / 3;
	//calculate variance
	jab_double sum = 0.0;
	for (jab_int32 i = 0; i < 3; i++)
	{
		sum += (rgb[i] - (*ave)) * (rgb[i] - (*ave));
	}
	*var = sum / 3;
}

/**
 * @brief Get the min, middle and max value of three values and the corresponding indexes
 * @param rgb the pixel with RGB values
 * @param min the min value
 * @param mid the middle value
 * @param max the max value
*/
void getMinMax(jab_byte* rgb, jab_byte* min, jab_byte* mid, jab_byte* max, jab_int32* index_min, jab_int32* index_mid, jab_int32* index_max)
{
	*index_min = 0;
	*index_mid = 1;
	*index_max = 2;
	if (rgb[*index_min] > rgb[*index_max])
		swap(index_min, index_max);
	if (rgb[*index_min] > rgb[*index_mid])
		swap(index_min, index_mid);
	if (rgb[*index_mid] > rgb[*index_max])
		swap(index_mid, index_max);
	*min = rgb[*index_min];
	*mid = rgb[*index_mid];
	*max = rgb[*index_max];
}

/**
 * @brief Warp points from source image to destination image in place
 * @param pt the transformation matrix
 * @param points the source points
 * @param length the number of source points
*/
void warpPoints(jab_perspective_transform* pt, jab_point* points, jab_int32 length)
{
	for (jab_int32 i = 0; i < length; i++) {
		jab_float x = points[i].x;
		jab_float y = points[i].y;
		jab_float denominator = pt->a13 * x + pt->a23 * y + pt->a33;
		points[i].x = (pt->a11 * x + pt->a21 * y + pt->a31) / denominator;
		points[i].y = (pt->a12 * x + pt->a22 * y + pt->a32) / denominator;
	}
}

 /**
  * @brief Save code bitmap in RGB as png image
  * @param bitmap the code bitmap
  * @param filename the image filename
  * @return JAB_SUCCESS | JAB_FAILURE
 */
jab_boolean saveImage(jab_bitmap* bitmap, jab_char* filename)
{
	png_image image;
	memset(&image, 0, sizeof(image));
	image.version = PNG_IMAGE_VERSION;

	if (bitmap->channel_count == 4)
	{
		image.format = PNG_FORMAT_RGBA;
		image.flags = PNG_FORMAT_FLAG_ALPHA | PNG_FORMAT_FLAG_COLOR;
	}
	else
	{
		image.format = PNG_FORMAT_GRAY;
	}

	image.width = bitmap->width;
	image.height = bitmap->height;

	if (png_image_write_to_file(&image,
		filename,
		0/*convert_to_8bit*/,
		bitmap->pixel,
		0/*row_stride*/,
		NULL/*colormap*/) == 0)
	{
		reportError(image.message);
		reportError("Saving png image failed");
		return JAB_FAILURE;
	}
	return JAB_SUCCESS;
}

/**
 * @brief Convert a bitmap from RGB to CMYK color space
 * @param bitmap the bitmap in RGB
 * @return the bitmap in CMYK | JAB_FAILURE
*/
jab_bitmap* convertRGB2CMYK(jab_bitmap* rgb)
{
	if (rgb->channel_count < 3)
	{
		JAB_REPORT_ERROR(("Not true color RGB bitmap"))
			return JAB_FAILURE;
	}
	jab_int32 w = rgb->width;
	jab_int32 h = rgb->height;
	jab_bitmap* cmyk = (jab_bitmap*)malloc(sizeof(jab_bitmap) + w * h * BITMAP_CHANNEL_COUNT * sizeof(jab_byte));
	if (cmyk == NULL)
	{
		JAB_REPORT_ERROR(("Memory allocation for CMYK bitmap failed"))
			return JAB_FAILURE;
	}
	cmyk->width = w;
	cmyk->height = h;
	cmyk->bits_per_pixel = BITMAP_BITS_PER_PIXEL;
	cmyk->bits_per_channel = BITMAP_BITS_PER_CHANNEL;
	cmyk->channel_count = BITMAP_CHANNEL_COUNT;

	jab_int32 rgb_bytes_per_pixel = rgb->bits_per_pixel / 8;
	jab_int32 rgb_bytes_per_row = rgb->width * rgb_bytes_per_pixel;
	jab_int32 cmyk_bytes_per_pixel = rgb->bits_per_pixel / 8;
	jab_int32 cmyk_bytes_per_row = rgb->width * cmyk_bytes_per_pixel;


	for (jab_int32 i = 0; i < h; i++)
	{
		for (jab_int32 j = 0; j < w; j++)
		{
			jab_double r1 = (jab_double)rgb->pixel[i * rgb_bytes_per_row + j * rgb_bytes_per_pixel + 0] / 255.0;
			jab_double g1 = (jab_double)rgb->pixel[i * rgb_bytes_per_row + j * rgb_bytes_per_pixel + 1] / 255.0;
			jab_double b1 = (jab_double)rgb->pixel[i * rgb_bytes_per_row + j * rgb_bytes_per_pixel + 2] / 255.0;

			jab_double k = 1 - MAX(r1, MAX(g1, b1));

			if (k == 1)
			{
				cmyk->pixel[i * cmyk_bytes_per_row + j * cmyk_bytes_per_pixel + 0] = 0;	//C
				cmyk->pixel[i * cmyk_bytes_per_row + j * cmyk_bytes_per_pixel + 1] = 0;	//M
				cmyk->pixel[i * cmyk_bytes_per_row + j * cmyk_bytes_per_pixel + 2] = 0;	//Y
				cmyk->pixel[i * cmyk_bytes_per_row + j * cmyk_bytes_per_pixel + 3] = 255;	//K
			}
			else
			{
				cmyk->pixel[i * cmyk_bytes_per_row + j * cmyk_bytes_per_pixel + 0] = (jab_byte)((1.0 - r1 - k) / (1.0 - k) * 255);	//C
				cmyk->pixel[i * cmyk_bytes_per_row + j * cmyk_bytes_per_pixel + 1] = (jab_byte)((1.0 - g1 - k) / (1.0 - k) * 255);	//M
				cmyk->pixel[i * cmyk_bytes_per_row + j * cmyk_bytes_per_pixel + 2] = (jab_byte)((1.0 - b1 - k) / (1.0 - k) * 255);	//Y
				cmyk->pixel[i * cmyk_bytes_per_row + j * cmyk_bytes_per_pixel + 3] = (jab_byte)(k * 255);								//K
			}
		}
	}
	return cmyk;
}

/**
 * @brief Save code bitmap in CMYK as TIFF image
 * @param bitmap the code bitmap
 * @param isCMYK set TRUE if the code bitmap is already in CMYK
 * @param filename the image filename
 * @return JAB_SUCCESS | JAB_FAILURE
*/
jab_boolean saveImageCMYK(jab_bitmap* bitmap, jab_boolean isCMYK, jab_char* filename)
{
	jab_bitmap* cmyk = 0;

	if (isCMYK)
	{
		cmyk = bitmap;
	}
	else
	{
		cmyk = convertRGB2CMYK(bitmap);
		if (cmyk == NULL)
		{
			JAB_REPORT_ERROR(("Converting RGB to CMYK failed"))
				return JAB_FAILURE;
		}
	}

	//save CMYK image as TIFF
	TIFF* out = TIFFOpen(filename, "w");
	if (out == NULL)
	{
		JAB_REPORT_ERROR(("Cannot open %s for writing", filename))
			if (!isCMYK)	free(cmyk);
		return JAB_FAILURE;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, cmyk->width);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, cmyk->height);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, cmyk->channel_count);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, cmyk->bits_per_channel);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
	jab_int32 rows_per_strip = TIFFDefaultStripSize(out, -1);
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, rows_per_strip);

	//write image to the file one scanline at a time
	jab_int32 row_size = cmyk->width * cmyk->channel_count;
	jab_boolean status = JAB_SUCCESS;
	for (jab_int32 row = 0; row < cmyk->height; row++)
	{
		if (TIFFWriteScanline(out, &cmyk->pixel[row * row_size], row, 0) < 0)
		{
			status = JAB_FAILURE;
			break;
		}
	}

	TIFFClose(out);
	if (!isCMYK)	free(cmyk);
	return status;
}

/**
 * @brief Read image into code bitmap
 * @param filename the image filename
 * @return Pointer to the code bitmap read from image | NULL
*/
jab_bitmap* readImage(jab_char* filename)
{
	png_image image;
	memset(&image, 0, sizeof(image));
	image.version = PNG_IMAGE_VERSION;

	jab_bitmap* bitmap;

	if (png_image_begin_read_from_file(&image, filename))
	{
		image.format = PNG_FORMAT_RGBA;

		bitmap = (jab_bitmap*)calloc(1, sizeof(jab_bitmap) + PNG_IMAGE_SIZE(image));
		if (bitmap == NULL)
		{
			png_image_free(&image);
			reportError("Memory allocation failed");
			return NULL;
		}
		bitmap->width = image.width;
		bitmap->height = image.height;
		bitmap->bits_per_channel = BITMAP_BITS_PER_CHANNEL;
		bitmap->bits_per_pixel = BITMAP_BITS_PER_PIXEL;
		bitmap->channel_count = BITMAP_CHANNEL_COUNT;

		if (png_image_finish_read(&image,
			NULL/*background*/,
			bitmap->pixel,
			0/*row_stride*/,
			NULL/*colormap*/) == 0)
		{
			free(bitmap);
			reportError(image.message);
			reportError("Reading png image failed");
			return NULL;
		}
	}
	else
	{
		reportError(image.message);
		reportError("Opening png image failed");
		return NULL;
	}
	return bitmap;
}

/**
 * @brief In-place interleaving
 * @param data the input data to be interleaved
*/
void interleaveData(jab_data* data)
{
	setSeed(INTERLEAVE_SEED);
	for (jab_int32 i = 0; i < data->length; i++)
	{
		jab_int32 pos = (jab_int32)((jab_float)lcg64_temper() / (jab_float)UINT32_MAX * (data->length - i));
		jab_char  tmp = data->data[data->length - 1 - i];
		data->data[data->length - 1 - i] = data->data[pos];
		data->data[pos] = tmp;
	}
}

/**
 * @brief In-place deinterleaving
 * @param data the input data to be deinterleaved
*/
void deinterleaveData(jab_data* data)
{
	jab_int32* index = (jab_int32*)malloc(data->length * sizeof(jab_int32));
	if (index == NULL)
	{
		reportError("Memory allocation for index buffer in deinterleaver failed");
		return;
	}
	for (jab_int32 i = 0; i < data->length; i++)
	{
		index[i] = i;
	}
	//interleave index
	setSeed(INTERLEAVE_SEED);
	for (jab_int32 i = 0; i < data->length; i++)
	{
		jab_int32 pos = (jab_int32)((jab_float)lcg64_temper() / (jab_float)UINT32_MAX * (data->length - i));
		jab_int32 tmp = index[data->length - 1 - i];
		index[data->length - 1 - i] = index[pos];
		index[pos] = tmp;
	}
	//deinterleave data
	jab_char* tmp_data = (jab_char*)malloc(data->length * sizeof(jab_char));
	if (tmp_data == NULL)
	{
		reportError("Memory allocation for temporary buffer in deinterleaver failed");
		return;
	}
	memcpy(tmp_data, data->data, data->length * sizeof(jab_char));
	for (jab_int32 i = 0; i < data->length; i++)
	{
		data->data[index[i]] = tmp_data[i];
	}
	free(tmp_data);
	free(index);
}

 /**
  * @brief Apply mask penalty rule 1
  * @param matrix the symbol matrix
  * @param width the symbol matrix width
  * @param height the symbol matrix height
  * @param color_number the number of module colors
  * @return the penalty score
 */
jab_int32 applyRule1(jab_int32* matrix, jab_int32 width, jab_int32 height, jab_int32 color_number)
{
	jab_byte fp0_c1, fp0_c2;
	jab_byte fp1_c1, fp1_c2;
	jab_byte fp2_c1, fp2_c2;
	jab_byte fp3_c1, fp3_c2;
	if (color_number == 2)                            //two colors: black(000) white(111)
	{
		fp0_c1 = 0;	fp0_c2 = 1;
		fp1_c1 = 1;	fp1_c2 = 0;
		fp2_c1 = 1;	fp2_c2 = 0;
		fp3_c1 = 1;	fp3_c2 = 0;
	}
	else if (color_number == 4)
	{
		fp0_c1 = 0;	fp0_c2 = 3;
		fp1_c1 = 1;	fp1_c2 = 2;
		fp2_c1 = 2;	fp2_c2 = 1;
		fp3_c1 = 3;	fp3_c2 = 0;
	}
	else
	{
		fp0_c1 = FP0_CORE_COLOR;	fp0_c2 = 7 - FP0_CORE_COLOR;
		fp1_c1 = FP1_CORE_COLOR;	fp1_c2 = 7 - FP1_CORE_COLOR;
		fp2_c1 = FP2_CORE_COLOR;	fp2_c2 = 7 - FP2_CORE_COLOR;
		fp3_c1 = FP3_CORE_COLOR;	fp3_c2 = 7 - FP3_CORE_COLOR;
	}

	jab_int32 score = 0;
	for (jab_int32 i = 0; i < height; i++)
	{
		for (jab_int32 j = 0; j < width; j++)
		{
			/*			//horizontal check
						if(j + 4 < width)
						{
							if(matrix[i * width + j] 	 == fp0_c1 &&	//finder pattern 0
							   matrix[i * width + j + 1] == fp0_c2 &&
							   matrix[i * width + j + 2] == fp0_c1 &&
							   matrix[i * width + j + 3] == fp0_c2 &&
							   matrix[i * width + j + 4] == fp0_c1)
							   score++;
							else if(									//finder pattern 1
							   matrix[i * width + j] 	 == fp1_c1 &&
							   matrix[i * width + j + 1] == fp1_c2 &&
							   matrix[i * width + j + 2] == fp1_c1 &&
							   matrix[i * width + j + 3] == fp1_c2 &&
							   matrix[i * width + j + 4] == fp1_c1)
							   score++;
							else if(									//finder pattern 2
							   matrix[i * width + j] 	 == fp2_c1 &&
							   matrix[i * width + j + 1] == fp2_c2 &&
							   matrix[i * width + j + 2] == fp2_c1 &&
							   matrix[i * width + j + 3] == fp2_c2 &&
							   matrix[i * width + j + 4] == fp2_c1)
							   score++;
							else if(									//finder pattern 3
							   matrix[i * width + j] 	 == fp3_c1 &&
							   matrix[i * width + j + 1] == fp3_c2 &&
							   matrix[i * width + j + 2] == fp3_c1 &&
							   matrix[i * width + j + 3] == fp3_c2 &&
							   matrix[i * width + j + 4] == fp3_c1)
							   score++;
						}
						//vertical check
						if(i + 4 < height)
						{
							if(matrix[i * width + j] 	   == fp0_c1 &&	//finder pattern 0
							   matrix[(i + 1) * width + j] == fp0_c2 &&
							   matrix[(i + 2) * width + j] == fp0_c1 &&
							   matrix[(i + 3) * width + j] == fp0_c2 &&
							   matrix[(i + 4) * width + j] == fp0_c1)
							   score++;
							else if(									//finder pattern 1
							   matrix[i * width + j] 	   == fp1_c1 &&
							   matrix[(i + 1) * width + j] == fp1_c2 &&
							   matrix[(i + 2) * width + j] == fp1_c1 &&
							   matrix[(i + 3) * width + j] == fp1_c2 &&
							   matrix[(i + 4) * width + j] == fp1_c1)
							   score++;
							else if(									//finder pattern 2
							   matrix[i * width + j] 	   == fp2_c1 &&
							   matrix[(i + 1) * width + j] == fp2_c2 &&
							   matrix[(i + 2) * width + j] == fp2_c1 &&
							   matrix[(i + 3) * width + j] == fp2_c2 &&
							   matrix[(i + 4) * width + j] == fp2_c1)
							   score++;
							else if(									//finder pattern 3
							   matrix[i * width + j] 	   == fp3_c1 &&
							   matrix[(i + 1) * width + j] == fp3_c2 &&
							   matrix[(i + 2) * width + j] == fp3_c1 &&
							   matrix[(i + 3) * width + j] == fp3_c2 &&
							   matrix[(i + 4) * width + j] == fp3_c1)
							   score++;
						}
			*/
			if (j >= 2 && j <= width - 3 && i >= 2 && i <= height - 3)
			{
				if (matrix[i * width + j - 2] == fp0_c1 &&	//finder pattern 0
					matrix[i * width + j - 1] == fp0_c2 &&
					matrix[i * width + j] == fp0_c1 &&
					matrix[i * width + j + 1] == fp0_c2 &&
					matrix[i * width + j + 2] == fp0_c1 &&
					matrix[(i - 2) * width + j] == fp0_c1 &&
					matrix[(i - 1) * width + j] == fp0_c2 &&
					matrix[(i)*width + j] == fp0_c1 &&
					matrix[(i + 1) * width + j] == fp0_c2 &&
					matrix[(i + 2) * width + j] == fp0_c1)
					score++;
				else if (
					matrix[i * width + j - 2] == fp1_c1 &&	//finder pattern 1
					matrix[i * width + j - 1] == fp1_c2 &&
					matrix[i * width + j] == fp1_c1 &&
					matrix[i * width + j + 1] == fp1_c2 &&
					matrix[i * width + j + 2] == fp1_c1 &&
					matrix[(i - 2) * width + j] == fp1_c1 &&
					matrix[(i - 1) * width + j] == fp1_c2 &&
					matrix[(i)*width + j] == fp1_c1 &&
					matrix[(i + 1) * width + j] == fp1_c2 &&
					matrix[(i + 2) * width + j] == fp1_c1)
					score++;
				else if (
					matrix[i * width + j - 2] == fp2_c1 &&	//finder pattern 2
					matrix[i * width + j - 1] == fp2_c2 &&
					matrix[i * width + j] == fp2_c1 &&
					matrix[i * width + j + 1] == fp2_c2 &&
					matrix[i * width + j + 2] == fp2_c1 &&
					matrix[(i - 2) * width + j] == fp2_c1 &&
					matrix[(i - 1) * width + j] == fp2_c2 &&
					matrix[(i)*width + j] == fp2_c1 &&
					matrix[(i + 1) * width + j] == fp2_c2 &&
					matrix[(i + 2) * width + j] == fp2_c1)
					score++;
				else if (
					matrix[i * width + j - 2] == fp3_c1 &&	//finder pattern 3
					matrix[i * width + j - 1] == fp3_c2 &&
					matrix[i * width + j] == fp3_c1 &&
					matrix[i * width + j + 1] == fp3_c2 &&
					matrix[i * width + j + 2] == fp3_c1 &&
					matrix[(i - 2) * width + j] == fp3_c1 &&
					matrix[(i - 1) * width + j] == fp3_c2 &&
					matrix[(i)*width + j] == fp3_c1 &&
					matrix[(i + 1) * width + j] == fp3_c2 &&
					matrix[(i + 2) * width + j] == fp3_c1)
					score++;
			}
		}
	}
	return W1 * score;
}

/**
 * @brief Apply mask penalty rule 2
 * @param matrix the symbol matrix
 * @param width the symbol matrix width
 * @param height the symbol matrix height
 * @return the penalty score
*/
jab_int32 applyRule2(jab_int32* matrix, jab_int32 width, jab_int32 height)
{
	jab_int32 score = 0;
	for (jab_int32 i = 0; i < height - 1; i++)
	{
		for (jab_int32 j = 0; j < width - 1; j++)
		{
			if (matrix[i * width + j] != -1 && matrix[i * width + (j + 1)] != -1 &&
				matrix[(i + 1) * width + j] != -1 && matrix[(i + 1) * width + (j + 1)] != -1)
			{
				if (matrix[i * width + j] == matrix[i * width + (j + 1)] &&
					matrix[i * width + j] == matrix[(i + 1) * width + j] &&
					matrix[i * width + j] == matrix[(i + 1) * width + (j + 1)])
					score++;
			}
		}
	}
	return W2 * score;
}

/**
 * @brief Apply mask penalty rule 3
 * @param matrix the symbol matrix
 * @param width the symbol matrix width
 * @param height the symbol matrix height
 * @return the penalty score
*/
jab_int32 applyRule3(jab_int32* matrix, jab_int32 width, jab_int32 height)
{
	jab_int32 score = 0;
	for (jab_int32 k = 0; k < 2; k++)
	{
		jab_int32 maxi, maxj;
		if (k == 0)	//horizontal scan
		{
			maxi = height;
			maxj = width;
		}
		else		//vertical scan
		{
			maxi = width;
			maxj = height;
		}
		for (jab_int32 i = 0; i < maxi; i++)
		{
			jab_int32 same_color_count = 0;
			jab_int32 pre_color = -1;
			for (jab_int32 j = 0; j < maxj; j++)
			{
				jab_int32 cur_color = (k == 0 ? matrix[i * width + j] : matrix[j * width + i]);
				if (cur_color != -1)
				{
					if (cur_color == pre_color)
						same_color_count++;
					else
					{
						if (same_color_count >= 5)
							score += W3 + (same_color_count - 5);
						same_color_count = 1;
						pre_color = cur_color;
					}
				}
				else
				{
					if (same_color_count >= 5)
						score += W3 + (same_color_count - 5);
					same_color_count = 0;
					pre_color = -1;
				}
			}
			if (same_color_count >= 5)
				score += W3 + (same_color_count - 5);
		}
	}
	return score;
}

/**
 * @brief Evaluate masking results
 * @param matrix the symbol matrix
 * @param width the symbol matrix width
 * @param height the symbol matrix height
 * @param color_number the number of module colors
 * @return the penalty score
*/
jab_int32 evaluateMask(jab_int32* matrix, jab_int32 width, jab_int32 height, jab_int32 color_number)
{
	return applyRule1(matrix, width, height, color_number) + applyRule2(matrix, width, height) + applyRule3(matrix, width, height);
}

/**
 * @brief Mask modules
 * @param enc the encode parameters
 * @param cp the code parameters
 * @return the mask pattern reference | -1 if fails
*/
jab_int32 maskCode(jab_encode* enc, jab_code* cp)
{
	jab_int32 mask_type = 0;
	jab_int32 min_penalty_score = 10000;

	//allocate memory for masked code
	jab_int32* masked = (jab_int32*)malloc(cp->code_size.x * cp->code_size.y * sizeof(jab_int32));
	if (masked == NULL)
	{
		reportError("Memory allocation for masked code failed");
		return -1;
	}
	memset(masked, -1, cp->code_size.x * cp->code_size.y * sizeof(jab_int32)); //set all bytes in masked as 0xFF

	//evaluate each mask pattern
	for (jab_int32 t = 0; t < NUMBER_OF_MASK_PATTERNS; t++)
	{
		jab_int32 penalty_score = 0;
		maskSymbols(enc, t, masked, cp);
		//calculate the penalty score
		penalty_score = evaluateMask(masked, cp->code_size.x, cp->code_size.y, enc->color_number);
#if TEST_MODE
		//JAB_REPORT_INFO(("Penalty score: %d", penalty_score))
#endif
		if (penalty_score < min_penalty_score)
		{
			mask_type = t;
			min_penalty_score = penalty_score;
		}
	}

	//mask all symbols with the selected mask pattern
	maskSymbols(enc, mask_type, 0, 0);

	//clean memory
	free(masked);
	return mask_type;
}

/**
 * @brief Demask modules
 * @param data the decoded data module values
 * @param data_map the data module positions
 * @param symbol_size the symbol size in module
 * @param mask_type the mask pattern reference
 * @param color_number the number of module colors
*/
void demaskSymbol(jab_data* data, jab_byte* data_map, jab_vector2d symbol_size, jab_int32 mask_type, jab_int32 color_number)
{
	jab_int32 symbol_width = symbol_size.x;
	jab_int32 symbol_height = symbol_size.y;
	jab_int32 count = 0;
	for (jab_int32 x = 0; x < symbol_width; x++)
	{
		for (jab_int32 y = 0; y < symbol_height; y++)
		{
			if (data_map[y * symbol_width + x] == 0)
			{
				if (count > data->length - 1) return;
				jab_int32 index = data->data[count];
				switch (mask_type)
				{
				case 0:
					index ^= (x + y) % color_number;
					break;
				case 1:
					index ^= x % color_number;
					break;
				case 2:
					index ^= y % color_number;
					break;
				case 3:
					index ^= (x / 2 + y / 3) % color_number;
					break;
				case 4:
					index ^= (x / 3 + y / 2) % color_number;
					break;
				case 5:
					index ^= ((x + y) / 2 + (x + y) / 3) % color_number;
					break;
				case 6:
					index ^= ((x * x * y) % 7 + (2 * x * x + 2 * y) % 19) % color_number;
					break;
				case 7:
					index ^= ((x * y * y) % 5 + (2 * x + y * y) % 13) % color_number;
					break;
				}
				data->data[count] = (jab_char)index;
				count++;
			}
		}
	}
}

/**
	* @brief Sample a symbol
	* @param bitmap the image bitmap
	* @param pt the transformation matrix
	* @param side_size the symbol size in module
	* @return the sampled symbol matrix
*/
jab_bitmap* sampleSymbol(jab_bitmap* bitmap, jab_perspective_transform* pt, jab_vector2d side_size)
{
	jab_int32 mtx_bytes_per_pixel = bitmap->bits_per_pixel / 8;
	jab_int32 mtx_bytes_per_row = side_size.x * mtx_bytes_per_pixel;
	jab_bitmap* matrix = (jab_bitmap*)malloc(sizeof(jab_bitmap) + side_size.x * side_size.y * mtx_bytes_per_pixel * sizeof(jab_byte));
	if (matrix == NULL)
	{
		reportError("Memory allocation for symbol bitmap matrix failed");
		return NULL;
	}
	matrix->channel_count = bitmap->channel_count;
	matrix->bits_per_channel = bitmap->bits_per_channel;
	matrix->bits_per_pixel = matrix->bits_per_channel * matrix->channel_count;
	matrix->width = side_size.x;
	matrix->height = side_size.y;

	jab_int32 bmp_bytes_per_pixel = bitmap->bits_per_pixel / 8;
	jab_int32 bmp_bytes_per_row = bitmap->width * bmp_bytes_per_pixel;

	jab_point** points = new jab_point*[side_size.x];
	for (jab_int32 i = 0; i < side_size.y; i++)
	{
		for (jab_int32 j = 0; j < side_size.x; j++)
		{
			points[j]->x = (jab_float)j + 0.5f;
			points[j]->y = (jab_float)i + 0.5f;
		}
		warpPoints(pt, *points, side_size.x);
		for (jab_int32 j = 0; j < side_size.x; j++)
		{
			jab_int32 mapped_x = (jab_int32)points[j]->x;
			jab_int32 mapped_y = (jab_int32)points[j]->y;
			if (mapped_x < 0 || mapped_x > bitmap->width - 1)
			{
				if (mapped_x == -1) mapped_x = 0;
				else if (mapped_x == bitmap->width) mapped_x = bitmap->width - 1;
				else return NULL;
			}
			if (mapped_y < 0 || mapped_y > bitmap->height - 1)
			{
				if (mapped_y == -1) mapped_y = 0;
				else if (mapped_y == bitmap->height) mapped_y = bitmap->height - 1;
				else return NULL;
			}
			for (jab_int32 c = 0; c < matrix->channel_count; c++)
			{
				//get the average of pixel values in 3x3 neighborhood as the sampled value
				jab_float sum = 0;
				for (jab_int32 dx = -1; dx <= 1; dx++)
				{
					for (jab_int32 dy = -1; dy <= 1; dy++)
					{
						jab_int32 px = mapped_x + dx;
						jab_int32 py = mapped_y + dy;
						if (px < 0 || px > bitmap->width - 1)  px = mapped_x;
						if (py < 0 || py > bitmap->height - 1) py = mapped_y;
						sum += bitmap->pixel[py * bmp_bytes_per_row + px * bmp_bytes_per_pixel + c];
					}
				}
				jab_byte ave = (jab_byte)(sum / 9.0f + 0.5f);
				matrix->pixel[i * mtx_bytes_per_row + j * mtx_bytes_per_pixel + c] = ave;
				//matrix->pixel[i*mtx_bytes_per_row + j*mtx_bytes_per_pixel + c] = bitmap->pixel[mapped_y*bmp_bytes_per_row + mapped_x*bmp_bytes_per_pixel + c];
#if TEST_MODE
				test_mode_bitmap->pixel[mapped_y * bmp_bytes_per_row + mapped_x * bmp_bytes_per_pixel + c] = test_mode_color;
				if (c == 3 && test_mode_color == 0)
					test_mode_bitmap->pixel[mapped_y * bmp_bytes_per_row + mapped_x * bmp_bytes_per_pixel + c] = 255;
#endif
			}
		}
	}
	return matrix;
}

/**
	* @brief Sample a cross area between the host and slave symbols
	* @param bitmap the image bitmap
	* @param pt the transformation matrix
	* @return the sampled area matrix
*/
jab_bitmap* sampleCrossArea(jab_bitmap* bitmap, jab_perspective_transform* pt)
{
	jab_int32 mtx_bytes_per_pixel = bitmap->bits_per_pixel / 8;
	jab_int32 mtx_bytes_per_row = SAMPLE_AREA_WIDTH * mtx_bytes_per_pixel;
	jab_bitmap* matrix = (jab_bitmap*)malloc(sizeof(jab_bitmap) + SAMPLE_AREA_WIDTH * SAMPLE_AREA_HEIGHT * mtx_bytes_per_pixel * sizeof(jab_byte));
	if (matrix == NULL)
	{
		reportError("Memory allocation for cross area bitmap matrix failed");
		return NULL;
	}
	matrix->channel_count = bitmap->channel_count;
	matrix->bits_per_channel = bitmap->bits_per_channel;
	matrix->bits_per_pixel = matrix->bits_per_channel * matrix->channel_count;
	matrix->width = SAMPLE_AREA_WIDTH;
	matrix->height = SAMPLE_AREA_HEIGHT;

	jab_int32 bmp_bytes_per_pixel = bitmap->bits_per_pixel / 8;
	jab_int32 bmp_bytes_per_row = bitmap->width * bmp_bytes_per_pixel;

	//only sample the area where the metadata and palette are located
	jab_point points[SAMPLE_AREA_WIDTH];
	for (jab_int32 i = 0; i < SAMPLE_AREA_HEIGHT; i++)
	{
		for (jab_int32 j = 0; j < SAMPLE_AREA_WIDTH; j++)
		{
			points[j].x = (jab_float)j + CROSS_AREA_WIDTH / 2 + 0.5f;
			points[j].y = (jab_float)i + 0.5f;
		}
		warpPoints(pt, points, SAMPLE_AREA_WIDTH);
		for (jab_int32 j = 0; j < SAMPLE_AREA_WIDTH; j++)
		{
			jab_int32 mapped_x = (jab_int32)points[j].x;
			jab_int32 mapped_y = (jab_int32)points[j].y;
			if (mapped_x < 0 || mapped_x > bitmap->width - 1)
			{
				if (mapped_x == -1) mapped_x = 0;
				else if (mapped_x == bitmap->width) mapped_x = bitmap->width - 1;
				else return NULL;
			}
			if (mapped_y < 0 || mapped_y > bitmap->height - 1)
			{
				if (mapped_y == -1) mapped_y = 0;
				else if (mapped_y == bitmap->height) mapped_y = bitmap->height - 1;
				else return NULL;
			}
			for (jab_int32 c = 0; c < matrix->channel_count; c++)
			{
				//get the average of pixel values in 3x3 neighborhood as the sampled value
				jab_float sum = 0;
				for (jab_int32 dx = -1; dx <= 1; dx++)
				{
					for (jab_int32 dy = -1; dy <= 1; dy++)
					{
						jab_int32 px = mapped_x + dx;
						jab_int32 py = mapped_y + dy;
						if (px < 0 || px > bitmap->width - 1)  px = mapped_x;
						if (py < 0 || py > bitmap->height - 1) py = mapped_y;
						sum += bitmap->pixel[py * bmp_bytes_per_row + px * bmp_bytes_per_pixel + c];
					}
				}
				jab_byte ave = (jab_byte)(sum / 9.0f + 0.5f);
				matrix->pixel[i * mtx_bytes_per_row + j * mtx_bytes_per_pixel + c] = ave;
				//matrix->pixel[i*mtx_bytes_per_row + j*mtx_bytes_per_pixel + c] = bitmap->pixel[mapped_y*bmp_bytes_per_row + mapped_x*bmp_bytes_per_pixel + c];
			}
		}
	}
	return matrix;
}

/**
 * libjabcode - JABCode Encoding/Decoding Library
 *
 * Copyright 2016 by Fraunhofer SIT. All rights reserved.
 * See LICENSE file for full terms of use and distribution.
 *
 * Contact: Huajian Liu <liu@sit.fraunhofer.de>
 *			Waldemar Berchtold <waldemar.berchtold@sit.fraunhofer.de>
 *
 * @file transform.c
 * @brief Matrix transform
 */

 /**
  * @brief Calculate transformation matrix of square to quadrilateral
  * @param x0 the x coordinate of the 1st destination point
  * @param y0 the y coordinate of the 1st destination point
  * @param x1 the x coordinate of the 2nd destination point
  * @param y1 the y coordinate of the 2nd destination point
  * @param x2 the x coordinate of the 3rd destination point
  * @param y2 the y coordinate of the 3rd destination point
  * @param x3 the x coordinate of the 4th destination point
  * @param y3 the y coordinate of the 4th destination point
  * @return the transformation matrix
 */
jab_perspective_transform* square2Quad(jab_float x0, jab_float y0,
	jab_float x1, jab_float y1,
	jab_float x2, jab_float y2,
	jab_float x3, jab_float y3)
{
	jab_perspective_transform* pt = (jab_perspective_transform*)malloc(sizeof(jab_perspective_transform));
	if (pt == NULL)
	{
		reportError("Memory allocation for perspective transform failed");
		return NULL;
	}
	jab_float dx3 = x0 - x1 + x2 - x3;
	jab_float dy3 = y0 - y1 + y2 - y3;
	if (dx3 == 0 && dy3 == 0) {
		pt->a11 = x1 - x0;
		pt->a21 = x2 - x1;
		pt->a31 = x0;
		pt->a12 = y1 - y0;
		pt->a22 = y2 - y1;
		pt->a32 = y0;
		pt->a13 = 0;
		pt->a23 = 0;
		pt->a33 = 1;
		return pt;
	}
	else
	{
		jab_float dx1 = x1 - x2;
		jab_float dx2 = x3 - x2;
		jab_float dy1 = y1 - y2;
		jab_float dy2 = y3 - y2;
		jab_float denominator = dx1 * dy2 - dx2 * dy1;
		jab_float a13 = (dx3 * dy2 - dx2 * dy3) / denominator;
		jab_float a23 = (dx1 * dy3 - dx3 * dy1) / denominator;
		pt->a11 = x1 - x0 + a13 * x1;
		pt->a21 = x3 - x0 + a23 * x3;
		pt->a31 = x0;
		pt->a12 = y1 - y0 + a13 * y1;
		pt->a22 = y3 - y0 + a23 * y3;
		pt->a32 = y0;
		pt->a13 = a13;
		pt->a23 = a23;
		pt->a33 = 1;
		return pt;
	}
}

/**
 * @brief Calculate transformation matrix of quadrilateral to square
 * @param x0 the x coordinate of the 1st source point
 * @param y0 the y coordinate of the 1st source point
 * @param x1 the x coordinate of the 2nd source point
 * @param y1 the y coordinate of the 2nd source point
 * @param x2 the x coordinate of the 3rd source point
 * @param y2 the y coordinate of the 3rd source point
 * @param x3 the x coordinate of the 4th source point
 * @param y3 the y coordinate of the 4th source point
 * @return the transformation matrix
*/
jab_perspective_transform* quad2Square(jab_float x0, jab_float y0,
	jab_float x1, jab_float y1,
	jab_float x2, jab_float y2,
	jab_float x3, jab_float y3)
{
	jab_perspective_transform* pt = (jab_perspective_transform*)malloc(sizeof(jab_perspective_transform));
	if (pt == NULL)
	{
		reportError("Memory allocation for perspective transform failed");
		return NULL;
	}
	jab_perspective_transform* s2q = square2Quad(x0, y0, x1, y1, x2, y2, x3, y3);
	//calculate the adjugate matrix of s2q
	pt->a11 = s2q->a22 * s2q->a33 - s2q->a23 * s2q->a32;
	pt->a21 = s2q->a23 * s2q->a31 - s2q->a21 * s2q->a33;
	pt->a31 = s2q->a21 * s2q->a32 - s2q->a22 * s2q->a31;
	pt->a12 = s2q->a13 * s2q->a32 - s2q->a12 * s2q->a33;
	pt->a22 = s2q->a11 * s2q->a33 - s2q->a13 * s2q->a31;
	pt->a32 = s2q->a12 * s2q->a31 - s2q->a11 * s2q->a32;
	pt->a13 = s2q->a12 * s2q->a23 - s2q->a13 * s2q->a22;
	pt->a23 = s2q->a13 * s2q->a21 - s2q->a11 * s2q->a23;
	pt->a33 = s2q->a11 * s2q->a22 - s2q->a12 * s2q->a21;
	free(s2q);
	return pt;
}

/**
 * @brief Calculate matrix multiplication
 * @param m1 the multiplicand
 * @param m2 the multiplier
 * @return m1 x m2
*/
jab_perspective_transform* multiply(jab_perspective_transform* m1, jab_perspective_transform* m2)
{
	jab_perspective_transform* product = (jab_perspective_transform*)malloc(sizeof(jab_perspective_transform));
	if (product == NULL)
	{
		reportError("Memory allocation for perpective transform failed");
		return NULL;
	}
	product->a11 = m1->a11 * m2->a11 + m1->a12 * m2->a21 + m1->a13 * m2->a31;
	product->a21 = m1->a21 * m2->a11 + m1->a22 * m2->a21 + m1->a23 * m2->a31;
	product->a31 = m1->a31 * m2->a11 + m1->a32 * m2->a21 + m1->a33 * m2->a31;
	product->a12 = m1->a11 * m2->a12 + m1->a12 * m2->a22 + m1->a13 * m2->a32;
	product->a22 = m1->a21 * m2->a12 + m1->a22 * m2->a22 + m1->a23 * m2->a32;
	product->a32 = m1->a31 * m2->a12 + m1->a32 * m2->a22 + m1->a33 * m2->a32;
	product->a13 = m1->a11 * m2->a13 + m1->a12 * m2->a23 + m1->a13 * m2->a33;
	product->a23 = m1->a21 * m2->a13 + m1->a22 * m2->a23 + m1->a23 * m2->a33;
	product->a33 = m1->a31 * m2->a13 + m1->a32 * m2->a23 + m1->a33 * m2->a33;
	return product;
}

/**
 * @brief Calculate transformation matrix of quadrilateral to quadrilateral
 * @param x0 the x coordinate of the 1st source point
 * @param y0 the y coordinate of the 1st source point
 * @param x1 the x coordinate of the 2nd source point
 * @param y1 the y coordinate of the 2nd source point
 * @param x2 the x coordinate of the 3rd source point
 * @param y2 the y coordinate of the 3rd source point
 * @param x3 the x coordinate of the 4th source point
 * @param y3 the y coordinate of the 4th source point
 * @param x0p the x coordinate of the 1st destination point
 * @param y0p the y coordinate of the 1st destination point
 * @param x1p the x coordinate of the 2nd destination point
 * @param y1p the y coordinate of the 2nd destination point
 * @param x2p the x coordinate of the 3rd destination point
 * @param y2p the y coordinate of the 3rd destination point
 * @param x3p the x coordinate of the 4th destination point
 * @param y3p the y coordinate of the 4th destination point
 * @return the transformation matrix
*/
jab_perspective_transform* perspectiveTransform(jab_float x0, jab_float y0,
	jab_float x1, jab_float y1,
	jab_float x2, jab_float y2,
	jab_float x3, jab_float y3,
	jab_float x0p, jab_float y0p,
	jab_float x1p, jab_float y1p,
	jab_float x2p, jab_float y2p,
	jab_float x3p, jab_float y3p)
{
	jab_perspective_transform* q2s = quad2Square(x0, y0, x1, y1, x2, y2, x3, y3);
	if (q2s == NULL)
	{
		return NULL;
	}
	jab_perspective_transform* s2q = square2Quad(x0p, y0p, x1p, y1p, x2p, y2p, x3p, y3p);
	if (s2q == NULL)
	{
		return NULL;
	}
	jab_perspective_transform* pt = multiply(q2s, s2q);
	if (pt == NULL)
	{
		return NULL;
	}
	free(q2s);
	free(s2q);
	return pt;
}


/**
 * @brief Get perspetive transformation matrix
 * @param p0 the coordinate of the 1st finder/alignment pattern
 * @param p1 the coordinate of the 2nd finder/alignment pattern
 * @param p2 the coordinate of the 3rd finder/alignment pattern
 * @param p3 the coordinate of the 4th finder/alignment pattern
 * @param side_size the side size of the symbol
 * @return the transformation matrix
*/
jab_perspective_transform* getPerspectiveTransform(jab_point p0,
	jab_point p1,
	jab_point p2,
	jab_point p3,
	jab_vector2d side_size)
{
	return perspectiveTransform(3.5f, 3.5f,
		(jab_float)side_size.x - 3.5f, 3.5f,
		(jab_float)side_size.x - 3.5f, (jab_float)side_size.y - 3.5f,
		3.5f, (jab_float)side_size.y - 3.5f,
		p0.x, p0.y,
		p1.x, p1.y,
		p2.x, p2.y,
		p3.x, p3.y
	);
}
}