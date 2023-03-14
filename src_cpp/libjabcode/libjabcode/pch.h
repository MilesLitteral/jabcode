// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define W1	100
#define W2	3
#define W3	3
#define INTERLEAVE_SEED 226759
#define SAMPLE_AREA_WIDTH	(CROSS_AREA_WIDTH / 2 - 2) //width of the columns where the metadata and palette in slave symbol are located
#define SAMPLE_AREA_HEIGHT	20	//height of the metadata rows including the first row, though it does not contain metadata

#define BLOCK_SIZE_POWER	5
#define BLOCK_SIZE 			(1 << BLOCK_SIZE_POWER)
#define BLOCK_SIZE_MASK 	(BLOCK_SIZE - 1)
#define MINIMUM_DIMENSION 	(BLOCK_SIZE * 5)
#define CAP(val, min, max)	(val < min ? min : (val > max ? max : val))

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// add headers that you want to pre-compile here
#include "framework.h"
#include "jabcode.h"
#include "decoder.h"
#include "encoder.h"
#include "detector.h"

#include "ldpc.h"
#include "pseudo_random.h"
#include "png.h"
#include "pngconf.h"
#include "pnglibconf.h"

#include "tiff.h"
#include "tiffio.h"
#include "tiffconf.h"
#include "tiffvers.h"

#endif //PCH_H