#ifndef _PARTICLE__
#define _PARTICLE__

#include "vector3.h"

#define SIZE_TRAIL 30

class Particle
{
public:

	Vector3 pos;		// current position
	Vector3 vel;		// velocity
	Vector3 restPos;	// rest (initial) position
	Vector3 oldPos;		// previous position
	Vector3 trail[SIZE_TRAIL];
	int start;

	Vector3 acc;		// acceleration

	Particle()
	{
		oldPos = restPos = pos = Vector3(0, 0, 0);
		Init();
	}
	
	Particle(float x, float y, float z)
	{
		oldPos = restPos = pos = Vector3(x, y, z);
		Init();
	}

	Particle(const Vector3 & _p)
	{
		oldPos = restPos = pos = _p;
		Init();
	}

	void Init()
	{
		acc = Vector3(0, 0, 0);
		vel = Vector3(0, 0, 0);
		start = 0;
	}

	void Update(const float & time_step)
	{
		trail[start] = pos;
		start--;
		if (start < 0)
			start = SIZE_TRAIL-1;
		Verlet(time_step);
	}

	
	// integration step with Verlet
	void Verlet(const float & time_step)
	{
		Vector3  temp = pos;

		pos += vel * time_step + acc * time_step * time_step ;
		vel = (temp - oldPos) / time_step;

		oldPos = temp;
	}		
};

# endif // _PARTICLE__