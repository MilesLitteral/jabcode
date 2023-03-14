/**
 * libjabcode - JABCode Encoding/Decoding Library
 *
 * Copyright 2016 by Fraunhofer SIT. All rights reserved.
 * See LICENSE file for full terms of use and distribution.
 *
 * Contact: Huajian Liu <liu@sit.fraunhofer.de>
 *			Waldemar Berchtold <waldemar.berchtold@sit.fraunhofer.de>
 *
 * @file detector.h
 * @brief Detector header
 */

#ifndef JABCODE_DETECTOR_H
#define JABCODE_DETECTOR_H

#define TEST_MODE			0
#if TEST_MODE
jab_bitmap* test_mode_bitmap;
jab_int32	test_mode_color;
#endif

#define MAX_MODULES 		145	//the number of modules in side-version 32
#define MAX_SYMBOL_ROWS		3
#define MAX_SYMBOL_COLUMNS	3
#define MAX_FINDER_PATTERNS 500
#define PI 					3.14159265
#define CROSS_AREA_WIDTH	14	//the width of the area across the host and slave symbols

#define DIST(x1, y1, x2, y2) (jab_float)(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)))

namespace jabcode {
	/**
	 * @brief Detection modes
	*/
	typedef enum
	{
		QUICK_DETECT = 0,
		NORMAL_DETECT,
		INTENSIVE_DETECT
	}jab_detect_mode;

	/**
	 * @brief Finder pattern, alignment pattern
	*/
	typedef struct {
		jab_int32		type;
		jab_float		module_size;
		jab_point		center;			//coordinates of the center
		jab_int32		found_count;
		jab_int32 		direction;
	}jab_finder_pattern, jab_alignment_pattern;

	/**
	 * @brief Perspective transform
	*/
	typedef struct {
		jab_float a11;
		jab_float a12;
		jab_float a13;
		jab_float a21;
		jab_float a22;
		jab_float a23;
		jab_float a31;
		jab_float a32;
		jab_float a33;
	}jab_perspective_transform;

	__declspec(dllexport) jab_perspective_transform* __stdcall getPerspectiveTransform(jab_point p0, jab_point p1,
		jab_point p2, jab_point p3,
		jab_vector2d side_size);
	__declspec(dllexport) jab_perspective_transform* __stdcall perspectiveTransform(jab_float x0, jab_float y0,
		jab_float x1, jab_float y1,
		jab_float x2, jab_float y2,
		jab_float x3, jab_float y3,
		jab_float x0p, jab_float y0p,
		jab_float x1p, jab_float y1p,
		jab_float x2p, jab_float y2p,
		jab_float x3p, jab_float y3p);
	__declspec(dllexport) jab_bitmap* __stdcall sampleSymbol(jab_bitmap* bitmap, jab_perspective_transform* pt, jab_vector2d side_size);
	__declspec(dllexport) jab_bitmap* __stdcall sampleCrossArea(jab_bitmap* bitmap, jab_perspective_transform* pt);
	__declspec(dllexport) jab_boolean __stdcall binarizerRGB(jab_bitmap* bitmap, jab_bitmap* rgb[3], jab_float* blk_ths);
	__declspec(dllexport) jab_boolean __stdcall detectMaster(jab_bitmap* bitmap, jab_bitmap* ch[], jab_decoded_symbol* master_symbol);
	__declspec(dllexport) jab_boolean __stdcall decodeDockedSlaves(jab_bitmap* bitmap, jab_bitmap* ch[], jab_decoded_symbol* symbols, jab_int32 host_index, jab_int32* total);
}
#endif
