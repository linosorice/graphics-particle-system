#ifndef __COLORS__
#define __COLORS__

#include <assert.h>
#include "vector3.h"

const Vector3 red(1.0, 0.0, 0.0);
const Vector3 yellow(1.0, 1.0, 0.0);
const Vector3 green(0.0, 1.0, 0.0);
const Vector3 cyan(0.0, 1.0, 1.0);
const Vector3 blue(0.0, 0.0, 1.0);
const Vector3 magenta(1.0, 0.0, 1.0);
const Vector3 white(1.0, 1.0, 1.0);
const Vector3 black(0.0, 0.0, 0.0);


Vector3 mix(const Vector3 & v1, const Vector3 & v2, float f)
{
//	assert ((f >= 0) && (f <= 1));
	return (v1 * f + v2 * (1 - f));
}


Vector3 ComputeColor(float factor)
{
	static float _max = 1;

	Vector3 color;

	if (factor > _max)
		_max = factor;

	factor /= _max;

	// black -> blue -> magenta -> cyan -> green -> yellow -> red -> white
	if (factor < .17)
	{
		color = mix(black, blue, factor / .17);
	}
	else 
	{
		factor -= .16;
		if (factor < .16)
		{
			color = mix(blue, magenta, factor / .16);
		}
		else
		{
			factor -= .17;
			if (factor < .17)
				color = mix(magenta, cyan, factor / .17);
			else
			{
				factor -= .16;
				if (factor < .16)
					color = mix(cyan, green, factor / .16);
				else
				{
					factor -= .17;
					if (factor < .17)
						color = mix(green, yellow, factor / .17);
					else
					{
						factor -= .16;
						color = mix(yellow, red, factor / .16);
					}
				}
			}
		}
	}

	return color;

}

#endif