#include <string>
namespace slg { namespace ocl {
std::string KernelSource_imagemap_funcs = 
"#line 2 \"imagemap_funcs.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2018 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxCoreRender.                                   *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ImageMaps support\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_IMAGEMAPS)\n"
"\n"
"__global const void *ImageMap_GetPixelsAddress(__global const float* restrict* restrict imageMapBuff,\n"
"		const uint page, const uint offset) {\n"
"	return &imageMapBuff[page][offset];\n"
"}\n"
"\n"
"float ImageMap_GetTexel_Float(\n"
"		const ImageMapStorageType storageType,\n"
"		__global const void *pixels,\n"
"		const uint width, const uint height, const uint channelCount,\n"
"		const ImageWrapType wrapType,\n"
"		const int s, const int t) {\n"
"	uint u, v;\n"
"	switch (wrapType) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_WRAP_REPEAT)\n"
"		case WRAP_REPEAT:\n"
"			u = Mod(s, width);\n"
"			v = Mod(t, height);\n"
"			break;\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_WRAP_BLACK)\n"
"		case WRAP_BLACK:\n"
"			if ((s < 0) || (s >= width) || (t < 0) || (t >= height))\n"
"				return 0.f;\n"
"\n"
"			u = s;\n"
"			v = t;\n"
"			break;\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_WRAP_WHITE)\n"
"		case WRAP_WHITE:\n"
"			if ((s < 0) || (s >= width) || (t < 0) || (t >= height))\n"
"				return 1.f;\n"
"\n"
"			u = s;\n"
"			v = t;\n"
"			break;\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_WRAP_CLAMP)\n"
"		case WRAP_CLAMP:\n"
"			u = clamp(s, 0, (int)width - 1);\n"
"			v = clamp(t, 0, (int)height - 1);\n"
"			break;\n"
"#endif\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"\n"
"	const uint index = v * width + u;\n"
"\n"
"	switch (storageType) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_BYTE_FORMAT)\n"
"		case BYTE: {\n"
"			switch (channelCount) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_1xCHANNELS)\n"
"				case 1: {\n"
"					const uchar a = ((__global const uchar *)pixels)[index];\n"
"					return a * (1.f / 255.f);\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_2xCHANNELS)\n"
"				case 2: {\n"
"					const uchar a = ((__global const uchar *)pixels)[index * 2];\n"
"					return a * (1.f / 255.f);\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_3xCHANNELS)\n"
"				case 3: {\n"
"					__global const uchar *rgb = &((__global const uchar *)pixels)[index * 3];\n"
"					return Spectrum_Y((float3)(rgb[0] * (1.f / 255.f), rgb[1] * (1.f / 255.f), rgb[2] * (1.f / 255.f)));\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_4xCHANNELS)\n"
"				case 4: {\n"
"					__global const uchar *rgba = &((__global const uchar *)pixels)[index * 4];\n"
"					return Spectrum_Y((float3)(rgba[0] * (1.f / 255.f), rgba[1] * (1.f / 255.f), rgba[2] * (1.f / 255.f)));\n"
"				}\n"
"#endif\n"
"				default:\n"
"					return 0.f;\n"
"			}\n"
"		}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_HALF_FORMAT)\n"
"		case HALF: {\n"
"			switch (channelCount) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_1xCHANNELS)\n"
"				case 1: {\n"
"					return vload_half(index, (__global const half *)pixels);\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_2xCHANNELS)\n"
"				case 2: {\n"
"					return vload_half(index * 2, (__global const half *)pixels);\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_3xCHANNELS)\n"
"				case 3: {\n"
"					return Spectrum_Y((float3)(\n"
"							vload_half(index * 3, (__global const half *)pixels),\n"
"							vload_half(index * 3 + 1, (__global const half *)pixels),\n"
"							vload_half(index * 3 + 2, (__global const half *)pixels)));\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_4xCHANNELS)\n"
"				case 4: {\n"
"					return Spectrum_Y((float3)(\n"
"							vload_half(index * 4, (__global const half *)pixels),\n"
"							vload_half(index * 4 + 1, (__global const half *)pixels),\n"
"							vload_half(index * 4 + 2, (__global const half *)pixels)));\n"
"				}\n"
"#endif\n"
"				default:\n"
"					return 0.f;\n"
"			}\n"
"		}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_FLOAT_FORMAT)\n"
"		case FLOAT: {\n"
"			switch (channelCount) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_1xCHANNELS)\n"
"				case 1: {\n"
"					const float a = ((__global const float *)pixels)[index];\n"
"					return a;\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_2xCHANNELS)\n"
"				case 2: {\n"
"					const float a = ((__global const float *)pixels)[index * 2];\n"
"					return a;\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_3xCHANNELS)\n"
"				case 3: {\n"
"					__global const float *rgb = &((__global const float *)pixels)[index * 3];\n"
"					return Spectrum_Y((float3)(rgb[0], rgb[1], rgb[2]));\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_4xCHANNELS)\n"
"				case 4: {\n"
"					__global const float *rgba = &((__global const float *)pixels)[index * 4];\n"
"					return Spectrum_Y((float3)(rgba[0], rgba[1], rgba[2]));\n"
"				}\n"
"#endif\n"
"				default:\n"
"					return 0.f;\n"
"			}\n"
"		}\n"
"#endif\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"}\n"
"\n"
"float3 ImageMap_GetTexel_Spectrum(\n"
"		const ImageMapStorageType storageType,\n"
"		__global const void *pixels,\n"
"		const uint width, const uint height, const uint channelCount,\n"
"		const ImageWrapType wrapType,\n"
"		const int s, const int t) {\n"
"	uint u, v;\n"
"	switch (wrapType) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_WRAP_REPEAT)\n"
"		case WRAP_REPEAT:\n"
"			u = Mod(s, width);\n"
"			v = Mod(t, height);\n"
"			break;\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_WRAP_BLACK)\n"
"		case WRAP_BLACK:\n"
"			if ((s < 0) || (s >= width) || (t < 0) || (t >= height))\n"
"				return BLACK;\n"
"\n"
"			u = s;\n"
"			v = t;\n"
"			break;\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_WRAP_WHITE)\n"
"		case WRAP_WHITE:\n"
"			if ((s < 0) || (s >= width) || (t < 0) || (t >= height))\n"
"				return WHITE;\n"
"\n"
"			u = s;\n"
"			v = t;\n"
"			break;\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_WRAP_CLAMP)\n"
"		case WRAP_CLAMP:\n"
"			u = clamp(s, 0, (int)width - 1);\n"
"			v = clamp(t, 0, (int)height - 1);\n"
"			break;\n"
"#endif\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"\n"
"	const uint index = v * width + u;\n"
"\n"
"	switch (storageType) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_BYTE_FORMAT)\n"
"		case BYTE: {\n"
"			switch (channelCount) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_1xCHANNELS)\n"
"				case 1: {\n"
"					const uchar a = ((__global const uchar *)pixels)[index];\n"
"					return a * (1.f / 255.f);\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_2xCHANNELS)\n"
"				case 2: {\n"
"					const uchar a = ((__global const uchar *)pixels)[index * 2] * (1.f / 255.f);\n"
"					return a;\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_3xCHANNELS)\n"
"				case 3: {\n"
"					__global const uchar *rgb = &((__global const uchar *)pixels)[index * 3];\n"
"					return (float3)(rgb[0] * (1.f / 255.f), rgb[1] * (1.f / 255.f), rgb[2] * (1.f / 255.f));\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_4xCHANNELS)\n"
"				case 4: {\n"
"					__global const uchar *rgba = &((__global const uchar *)pixels)[index * 4];\n"
"					return (float3)(rgba[0] * (1.f / 255.f), rgba[1] * (1.f / 255.f), rgba[2] * (1.f / 255.f));\n"
"				}\n"
"#endif\n"
"				default:\n"
"					return 0.f;\n"
"			}\n"
"		}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_HALF_FORMAT)\n"
"		case HALF: {\n"
"			switch (channelCount) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_1xCHANNELS)\n"
"				case 1: {\n"
"					return vload_half(index, (__global const half *)pixels);\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_2xCHANNELS)\n"
"				case 2: {\n"
"					return vload_half(index * 2, (__global const half *)pixels);\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_3xCHANNELS)\n"
"				case 3: {\n"
"					return (float3)(\n"
"							vload_half(index * 3, (__global const half *)pixels),\n"
"							vload_half(index * 3 + 1, (__global const half *)pixels),\n"
"							vload_half(index * 3 + 2, (__global const half *)pixels));\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_4xCHANNELS)\n"
"				case 4: {\n"
"					return (float3)(\n"
"							vload_half(index * 4, (__global const half *)pixels),\n"
"							vload_half(index * 4 + 1, (__global const half *)pixels),\n"
"							vload_half(index * 4 + 2, (__global const half *)pixels));\n"
"				}\n"
"#endif\n"
"				default:\n"
"					return 0.f;\n"
"			}\n"
"		}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_FLOAT_FORMAT)\n"
"		case FLOAT: {\n"
"			switch (channelCount) {\n"
"#if defined(PARAM_HAS_IMAGEMAPS_1xCHANNELS)\n"
"				case 1: {\n"
"					const float a = ((__global const float *)pixels)[index];\n"
"					return a;\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_2xCHANNELS)\n"
"				case 2: {\n"
"					const float a = ((__global const float *)pixels)[index * 2];\n"
"					return a;\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_3xCHANNELS)\n"
"				case 3: {\n"
"					__global const float *rgb = &((__global const float *)pixels)[index * 3];\n"
"					return (float3)(rgb[0], rgb[1], rgb[2]);\n"
"				}\n"
"#endif\n"
"#if defined(PARAM_HAS_IMAGEMAPS_4xCHANNELS)\n"
"				case 4: {\n"
"					__global const float *rgba = &((__global const float *)pixels)[index * 4];\n"
"					return (float3)(rgba[0], rgba[1], rgba[2]);\n"
"				}\n"
"#endif\n"
"				default:\n"
"					return 0.f;\n"
"			}\n"
"		}\n"
"#endif\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"}\n"
"\n"
"float ImageMap_GetFloat(__global const ImageMap *imageMap,\n"
"		const float u, const float v\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	__global const void *pixels = ImageMap_GetPixelsAddress(\n"
"		imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"	const ImageMapStorageType storageType = imageMap->storageType;\n"
"	const uint channelCount = imageMap->channelCount;\n"
"	const uint width = imageMap->width;\n"
"	const uint height = imageMap->height;\n"
"	const ImageWrapType wrapType = imageMap->wrapType;\n"
"\n"
"	const float s = u * width - .5f;\n"
"	const float t = v * height - .5f;\n"
"\n"
"	const int s0 = Floor2Int(s);\n"
"	const int t0 = Floor2Int(t);\n"
"\n"
"	const float ds = s - s0;\n"
"	const float dt = t - t0;\n"
"\n"
"	const float ids = 1.f - ds;\n"
"	const float idt = 1.f - dt;\n"
"\n"
"	const float c0 = ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, s0, t0);\n"
"	const float c1 = ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, s0, t0 + 1);\n"
"	const float c2 = ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, s0 + 1, t0);\n"
"	const float c3 = ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, s0 + 1, t0 + 1);\n"
"\n"
"	const float k0 = ids * idt;\n"
"	const float k1 = ids * dt;\n"
"	const float k2 = ds * idt;\n"
"	const float k3 = ds * dt;\n"
"\n"
"	return (k0 * c0 + k1 *c1 + k2 * c2 + k3 * c3);\n"
"}\n"
"\n"
"float3 ImageMap_GetSpectrum(__global const ImageMap *imageMap,\n"
"		const float u, const float v\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	__global const void *pixels = ImageMap_GetPixelsAddress(\n"
"		imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"	const ImageMapStorageType storageType = imageMap->storageType;\n"
"	const uint channelCount = imageMap->channelCount;\n"
"	const uint width = imageMap->width;\n"
"	const uint height = imageMap->height;\n"
"	const ImageWrapType wrapType = imageMap->wrapType;\n"
"\n"
"	const float s = u * width - .5f;\n"
"	const float t = v * height - .5f;\n"
"\n"
"	const int s0 = Floor2Int(s);\n"
"	const int t0 = Floor2Int(t);\n"
"\n"
"	const float ds = s - s0;\n"
"	const float dt = t - t0;\n"
"\n"
"	const float ids = 1.f - ds;\n"
"	const float idt = 1.f - dt;\n"
"\n"
"	const float3 c0 = ImageMap_GetTexel_Spectrum(storageType, pixels, width, height, channelCount, wrapType, s0, t0);\n"
"	const float3 c1 = ImageMap_GetTexel_Spectrum(storageType, pixels, width, height, channelCount, wrapType, s0, t0 + 1);\n"
"	const float3 c2 = ImageMap_GetTexel_Spectrum(storageType, pixels, width, height, channelCount, wrapType, s0 + 1, t0);\n"
"	const float3 c3 = ImageMap_GetTexel_Spectrum(storageType, pixels, width, height, channelCount, wrapType, s0 + 1, t0 + 1);\n"
"\n"
"	const float k0 = ids * idt;\n"
"	const float k1 = ids * dt;\n"
"	const float k2 = ds * idt;\n"
"	const float k3 = ds * dt;\n"
"\n"
"	return (k0 * c0 + k1 *c1 + k2 * c2 + k3 * c3);\n"
"}\n"
"\n"
"float2 ImageMap_GetDuv(__global const ImageMap *imageMap,\n"
"		const float u, const float v\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	__global const void *pixels = ImageMap_GetPixelsAddress(\n"
"		imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"	const ImageMapStorageType storageType = imageMap->storageType;\n"
"	const uint channelCount = imageMap->channelCount;\n"
"	const uint width = imageMap->width;\n"
"	const uint height = imageMap->height;\n"
"	const ImageWrapType wrapType = imageMap->wrapType;\n"
"\n"
"	const float s = u * width;\n"
"	const float t = v * height;\n"
"\n"
"	const int is = Floor2Int(s);\n"
"	const int it = Floor2Int(t);\n"
"\n"
"	const float as = s - is;\n"
"	const float at = t - it;\n"
"\n"
"	int s0, s1;\n"
"	if (as < .5f) {\n"
"		s0 = is - 1;\n"
"		s1 = is;\n"
"	} else {\n"
"		s0 = is;\n"
"		s1 = is + 1;\n"
"	}\n"
"	int t0, t1;\n"
"	if (at < .5f) {\n"
"		t0 = it - 1;\n"
"		t1 = it;\n"
"	} else {\n"
"		t0 = it;\n"
"		t1 = it + 1;\n"
"	}\n"
"\n"
"	float2 duv;\n"
"	duv.x = mix(ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, s1, it) - ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, s0, it),\n"
"		ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, s1, it + 1) - ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, s0, it + 1), at) * width;\n"
"	duv.y = mix(ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, is, t1) - ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, is, t0),\n"
"		ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, is + 1, t1) - ImageMap_GetTexel_Float(storageType, pixels, width, height, channelCount, wrapType, is + 1, t0), as) * height;\n"
"\n"
"	return duv;\n"
"}\n"
"\n"
"#endif\n"
; } }
