/**
 * libjabcode - JABCode Encoding/Decoding Library
 *
 * Copyright 2016 by Fraunhofer SIT. All rights reserved.
 * See LICENSE file for full terms of use and distribution.
 *
 * Contact: Huajian Liu <liu@sit.fraunhofer.de>
 *			Waldemar Berchtold <waldemar.berchtold@sit.fraunhofer.de>
 *
 * @file jabcode.h
 * @brief Main libjabcode header
 */

#ifndef JABCODE_H
#define JABCODE_H           __declspec(dllexport) 

/*#ifdef BUILD_DLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
*/

#define VERSION "2.0.0"     
#define BUILD_DATE __DATE__ 

#define MAX_SYMBOL_NUMBER       61      
#define MAX_COLOR_NUMBER        256     
#define MAX_SIZE_ENCODING_MODE  256     
#define JAB_ENCODING_MODES      6       
#define ENC_MAX                 1000000 
#define NUMBER_OF_MASK_PATTERNS	8       

#define DEFAULT_SYMBOL_NUMBER 			1  
#define DEFAULT_MODULE_SIZE				12 
#define DEFAULT_COLOR_NUMBER 			8  
#define DEFAULT_MODULE_COLOR_MODE 		2  
#define DEFAULT_ECC_LEVEL				3  
#define DEFAULT_MASKING_REFERENCE 		7   


#define DISTANCE_TO_BORDER      4
#define MAX_ALIGNMENT_NUMBER    9 
#define COLOR_PALETTE_NUMBER	4

#define BITMAP_BITS_PER_PIXEL	32
#define BITMAP_BITS_PER_CHANNEL	8 
#define BITMAP_CHANNEL_COUNT	4  

#define	JAB_SUCCESS		1
#define	JAB_FAILURE		0 

#define NORMAL_DECODE		0 
#define COMPATIBLE_DECODE	1 

#define VERSION2SIZE(x)		(x * 4 + 17)   
#define SIZE2VERSION(x)		((x - 17) / 4) 

#define JAB_REPORT_ERROR(x)	{ printf("JABCode Error: "); printf x; printf("\n"); }
#define JAB_REPORT_INFO(x)	{ printf("JABCode Info: "); printf x; printf("\n"); }

namespace jabcode {
	typedef unsigned char 		jab_byte;
	typedef char 				jab_char;
	typedef unsigned char 		jab_boolean;
	typedef int 				jab_int32;
	typedef unsigned int 		jab_uint32;
	typedef short 				jab_int16;
	typedef unsigned short 		jab_uint16;
	typedef long long 			jab_int64;
	typedef unsigned long long	jab_uint64;
	typedef float				jab_float;
	typedef double              jab_double;

	/**
		* @brief 2-dimensional integer vector
	*/
	typedef struct {
		jab_int32	x;
		jab_int32	y;
	}jab_vector2d;

	/**
		* @brief 2-dimensional float vector
	*/
	typedef struct {
		jab_float	x;
		jab_float	y;
	}jab_point;

	/**
		* @brief Data structure
	*/
	typedef struct {
		jab_int32	length;
		jab_char	data[1];
	}jab_data;

	/**
		* @brief Code bitmap
	*/
	typedef struct {
		jab_int32	width;
		jab_int32	height;
		jab_int32	bits_per_pixel;
		jab_int32	bits_per_channel;
		jab_int32	channel_count;
		jab_byte    pixel[1];
	}jab_bitmap;

	/**
		* @brief Symbol parameters
	*/
	typedef struct {
		jab_int32		index;
		jab_vector2d	side_size;
		jab_int32		host;
		jab_int32		slaves[4];
		jab_int32 		wcwr[2];
		jab_data* data;
		jab_byte* data_map;
		jab_data* metadata;
		jab_byte* matrix;
	}jab_symbol;

	/**
		* @brief Encode parameters
	*/
	typedef struct {
		jab_int32		color_number;
		jab_int32		symbol_number;
		jab_int32		module_size;
		jab_int32		master_symbol_width;
		jab_int32		master_symbol_height;
		jab_byte* palette;				///< Palette holding used module colors in format RGB
		jab_vector2d* symbol_versions;
		jab_byte* symbol_ecc_levels;
		jab_int32* symbol_positions;
		jab_symbol* symbols;				///< Pointer to internal representation of JAB Code symbols
		jab_bitmap* bitmap;
	}jab_encode;

	/**
		* @brief Decoded metadata
	*/
	typedef struct {
		jab_boolean default_mode;
		jab_byte Nc;
		jab_byte mask_type;
		jab_byte docked_position;
		jab_vector2d side_version;
		jab_vector2d ecl;
	}jab_metadata;

	/**
		* @brief Decoded symbol
	*/
	typedef struct {
		jab_int32 index;
		jab_int32 host_index;
		jab_int32 host_position;
		jab_vector2d side_size;
		jab_float module_size;
		jab_point pattern_positions[4];
		jab_metadata metadata;
		jab_metadata slave_metadata[4];
		jab_byte* palette;
		jab_data* data;
	}jab_decoded_symbol;

	__declspec(dllexport) jab_int32   __stdcall MIN(jab_int32 a, jab_int32 b);
	__declspec(dllexport) jab_int32   __stdcall MAX(jab_int32 a, jab_int32 b);
	__declspec(dllexport) void        __stdcall getAveVar(jab_byte* rgb, jab_double* ave, jab_double* var);
	__declspec(dllexport) void        __stdcall getMinMax(jab_byte* rgb, jab_byte* min, jab_byte* mid, jab_byte* max, jab_int32* index_min, jab_int32* index_mid, jab_int32* index_max);

	__declspec(dllexport) void        __stdcall destroyEncode(jab_encode* enc);
	__declspec(dllexport) jab_encode* __stdcall createEncode(jab_int32 color_number, jab_int32 symbol_number);
	__declspec(dllexport) jab_int32   __stdcall generateJABCode(jab_encode* enc, jab_data* data);
	__declspec(dllexport) jab_data*   __stdcall decodeJABCode(jab_bitmap* bitmap, jab_int32 mode, jab_int32* status);
	__declspec(dllexport) jab_data*   __stdcall decodeJABCodeEx(jab_bitmap* bitmap, jab_int32 mode, jab_int32* status, jab_decoded_symbol* symbols, jab_int32 max_symbol_number);

	__declspec(dllexport) jab_bitmap* __stdcall readImage(jab_char*   filename);
	__declspec(dllexport) jab_boolean __stdcall saveImage(jab_bitmap* bitmap, jab_char* filename);
	__declspec(dllexport) jab_boolean __stdcall saveImageCMYK(jab_bitmap* bitmap, jab_boolean isCMYK, jab_char* filename);
	__declspec(dllexport) void        __stdcall reportError(const jab_char* message);
}
#endif
