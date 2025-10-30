/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#ifndef BASE_COLOR_H
#define BASE_COLOR_H

#include "vmath.h"

/*
	Title: Color handling
*/

/*
	Function: HueToRgb
		Converts Hue to RGB
*/
inline float HueToRgb(float v1, float v2, float h)
{
	if(h < 0.0f)
		h += 1;
	if(h > 1.0f)
		h -= 1;
	if((6.0f * h) < 1.0f)
		return v1 + (v2 - v1) * 6.0f * h;
	if((2.0f * h) < 1.0f)
		return v2;
	if((3.0f * h) < 2.0f)
		return v1 + (v2 - v1) * ((2.0f / 3.0f) - h) * 6.0f;
	return v1;
}

/*
	Function: HslToRgb
		Converts HSL to RGB
*/
inline vec3 HslToRgb(vec3 HSL)
{
	if(HSL.s == 0.0f)
		return vec3(HSL.l, HSL.l, HSL.l);

	float v2 = HSL.l < 0.5f ? HSL.l * (1.0f + HSL.s) : (HSL.l + HSL.s) - (HSL.s * HSL.l);
	float v1 = 2.0f * HSL.l - v2;
	return vec3(HueToRgb(v1, v2, HSL.h + (1.0f / 3.0f)), HueToRgb(v1, v2, HSL.h), HueToRgb(v1, v2, HSL.h - (1.0f / 3.0f)));
}

/*
	Function: VarToRgb
		Converts Var to RGB
*/
inline vec3 VarToRgb(int v)
{
	return HslToRgb(vec3(((v >> 16) & 0xff) / 255.0f, ((v >> 8) & 0xff) / 255.0f, (v & 0xff) / 255.0f));
}

/*
	Function: VarToRgba
		Converts Var to RGBA
*/
inline vec4 VarToRgba(int v)
{
	vec3 r = VarToRgb(v);
	float Alpha = ((v >> 24) & 0xff) / 255.0f;
	return vec4(r.r, r.g, r.b, Alpha);
}
/*
	Function: VarToRgbaWithAlpha
		Converts Var to RGBA
*/
inline vec4 VarToRgbaWithAlpha(int v, float Alpha)
{
	vec3 r = VarToRgb(v);
	return vec4(r.r, r.g, r.b, Alpha);
}
/*
	Function: HexToRgba
		Converts Hex to RGBA

	Remarks: Hex should be RGBA8
*/
inline vec4 HexToRgba(int hex)
{
	vec4 c;
	c.r = ((hex >> 24) & 0xFF) / 255.0f;
	c.g = ((hex >> 16) & 0xFF) / 255.0f;
	c.b = ((hex >> 8) & 0xFF) / 255.0f;
	c.a = (hex & 0xFF) / 255.0f;
	return c;
}

/*
	Function: HsvToRgb
		Converts HSV to RGB
*/
inline vec3 HsvToRgb(vec3 hsv)
{
	int h = int(hsv.x * 6.0f);
	float f = hsv.x * 6.0f - h;
	float p = hsv.z * (1.0f - hsv.y);
	float q = hsv.z * (1.0f - hsv.y * f);
	float t = hsv.z * (1.0f - hsv.y * (1.0f - f));

	vec3 rgb = vec3(0.0f, 0.0f, 0.0f);

	switch(h % 6)
	{
	case 0:
		rgb.r = hsv.z;
		rgb.g = t;
		rgb.b = p;
		break;

	case 1:
		rgb.r = q;
		rgb.g = hsv.z;
		rgb.b = p;
		break;

	case 2:
		rgb.r = p;
		rgb.g = hsv.z;
		rgb.b = t;
		break;

	case 3:
		rgb.r = p;
		rgb.g = q;
		rgb.b = hsv.z;
		break;

	case 4:
		rgb.r = t;
		rgb.g = p;
		rgb.b = hsv.z;
		break;

	case 5:
		rgb.r = hsv.z;
		rgb.g = p;
		rgb.b = q;
		break;
	}

	return rgb;
}

/*
	Function: RgbToHsv
		Converts RGB to HSV
*/
inline vec3 RgbToHsv(vec3 rgb)
{
	float h_min = minimum(minimum(rgb.r, rgb.g), rgb.b);
	float h_max = maximum(maximum(rgb.r, rgb.g), rgb.b);

	// hue
	float hue = 0.0f;

	if(h_max == h_min)
		hue = 0.0f;
	else if(h_max == rgb.r)
		hue = (rgb.g - rgb.b) / (h_max - h_min);
	else if(h_max == rgb.g)
		hue = 2.0f + (rgb.b - rgb.r) / (h_max - h_min);
	else
		hue = 4.0f + (rgb.r - rgb.g) / (h_max - h_min);

	hue /= 6.0f;

	if(hue < 0.0f)
		hue += 1.0f;

	// saturation
	float s = 0.0f;
	if(h_max != 0.0f)
		s = (h_max - h_min) / h_max;

	// value
	float v = h_max;

	return vec3(hue, s, v);
}

inline float RgbToLabH(float val)
{
	return val > 0.008856f ? powf(val, 0.333333f) : (7.787f * val + 0.137931f);
}

/*
	Function: RgbToLab
		Converts RGB to Lab
*/
inline vec3 RgbToLab(vec3 rgb)
{
	vec3 adapt(0.950467f, 1, 1.088969f);
	vec3 xyz(
		0.412424f * rgb.r + 0.357579f * rgb.g + 0.180464f * rgb.b,
		0.212656f * rgb.r + 0.715158f * rgb.g + 0.0721856f * rgb.b,
		0.0193324f * rgb.r + 0.119193f * rgb.g + 0.950444f * rgb.b);

	return vec3(
		116 * RgbToLabH(xyz.y / adapt.y) - 16,
		500 * (RgbToLabH(xyz.x / adapt.x) - RgbToLabH(xyz.y / adapt.y)),
		200 * (RgbToLabH(xyz.y / adapt.y) - RgbToLabH(xyz.z / adapt.z)));
}

#endif
