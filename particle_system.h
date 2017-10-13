// code for the verlet-based physical simulation

#ifndef __PARTICLE_SYSTEM__
#define __PARTICLE_SYSTEM__

#define TIME_STEP 0.01f

#include "particle.h"
#include "colors.h"
#include <vector>

#define _USE_MATH_DEFINES // enable the definition of M_PI
#include <math.h>

#define COLLISIONS

using namespace std;

class ParticleSystem 
{
	vector<Particle>   _particles;		// the particles

	Vector3     m_vGravity;				// gravity force applied to the particles system
	float       m_fTimeStep;			// time step
	int			iterations;

	Vector3 attractor;

public:

	ParticleSystem()
	{
		m_vGravity = Vector3(0, -9.81f, 0);
		m_fTimeStep = TIME_STEP;	

		attractor = Vector3(0, 0, 0);
	}

	void Reset()
	{
		_particles.clear();
	}

	// accessing the fields

	void SetGravity(Vector3 g)	{		m_vGravity = g;}

	void SetTimeStep(float ts)	{		m_fTimeStep = ts;}

	// adding a particle
	Particle* AddParticle(Particle _p)
	{
		_particles.push_back(_p);

		return &(_particles.back());
	}

	void SetAttractor(Vector3 g) { attractor = g; }

	void Build()
	{
		#ifdef FIREWORKS
			fireworks();
		#endif
		#ifdef ATTRACTOR
			attractor_build();
		#endif
		#ifdef COLLISIONS
			collisions();
		#endif
	}

	void collisions()
	{
		int n = 1000;
		int i;
		float r = 10;
		float theta = 0;
		Vector3 pos(0, 0, 0);
		Vector3 vel(0, 0, 0);
		Particle p(pos);

		for (i = 0; i < n; i++) {

			theta = randRange(-M_PI/20, M_PI/20);
			r = randRange(8, 12);

			p.vel.x = sin(theta)*r;
			p.vel.y = cos(theta)*r;

			AddParticle(p);
		}
	}

	void attractor_build()
	{
		float i, j;
		
		for (i = -10; i < 10; i += 0.4) {
			for (j = -10; j < 10; j += 0.4) {
				Particle p(i, j, 0);
				AddParticle(p);
			}
		}
		Particle p(attractor);
		AddParticle(p);
	}

	void fireworks()
	{
		int n = 100;
		int i;
		float r = 10;
		float theta = 0;
		Vector3 pos(0, 0, 0);
		Vector3 vel(0, 0, 0);
		Particle p(pos);

		for (i = 0; i <= n; i++) {

			theta = randRange(0, M_PI);
			r = randRange(0, 10);

			vel.x = cos(theta)*r;
			vel.y = sin(theta)*r;

			p.vel = vel;
			AddParticle(p);
		}
	}

	float randRange(float min, float max) {
		return rand() * (max - min) / RAND_MAX + min;
	}


	// integration step
	void Update()
	{
		AccumulateForces();			// assigns the forces to the particles		

		#ifdef ATTRACTOR
		/*if (iterations == 300) {
			int a = randRange(-5, 5);
			int b = randRange(-5, 5);
			int c = randRange(-5, 5);

			attractor = Vector3(a, b, c);
			iterations = 0;
		} else {
			iterations++;
		}*/
		#endif

		vector<Particle>::iterator pIt;
		for(pIt = _particles.begin(); pIt != _particles.end(); pIt++) 
			pIt->Update(m_fTimeStep);

		#ifdef COLLISIONS
		CollisionHandling();
		#endif
	}	

	// draws the particles system
	// each particle is represented by a red point
	void Draw()
	{
		// draw round points
		glPointSize(4.f);
		glEnable(GL_POINT_SMOOTH);
		glAlphaFunc(GL_GREATER,0.5f); 
		glEnable(GL_ALPHA_TEST); 
		glEnable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);

		// draws the particles

		vector<Particle>::iterator pIt;
		#ifdef COLLISIONS
		int i,l;
		#endif
		for(pIt = _particles.begin(); pIt != _particles.end(); pIt++) {
			glBegin(GL_POINTS);
			glColor3f(1.f, 0.f, 0.f);
			Vector3& pos = pIt->pos;
			glVertex3f(pos.x, pos.y, pos.z);
			glEnd();

			#ifdef COLLISIONS
			glBegin(GL_LINE_STRIP);
			glColor3f(0,1,1);
			l = pIt->start;
			for (i = 0; i < SIZE_TRAIL; i++) {
				l++;
				if (l > SIZE_TRAIL-1)
					l = 0;
				glVertex3f(pIt->trail[l].x, pIt->trail[l].y, pIt->trail[l].z);
			}
			glEnd();
			#endif
		}
		glEnable(GL_LIGHTING);

		#ifdef COLLISIONS
		DrawCollisionPlaneYZ(3);
		DrawCollisionPlaneYZ(-3);
		DrawCollisionPlaneXZ(3);
		DrawSphere(Vector3(0, -6, 0), 6);
		#endif

		#ifdef ATTRACTOR
			glBegin(GL_POINTS);
			glColor3f(0.f, 1.f, 1.f);
			glVertex3f(attractor.x, attractor.y, attractor.z);
			glEnd();
		#endif
	}

private:

	// this function assigns all the forces applied to each particle
	void AccumulateForces()
	{
		// all the particles are influenced by gravity
		vector<Particle>::iterator pIt;
		float d;
		for(pIt = _particles.begin(); pIt != _particles.end(); pIt++){
			#ifdef ATTRACTOR
			Vector3 v = attractor - pIt->pos;
			v = v.Normalize() * 10;
			pIt->acc = v;
			#else
			pIt->acc = m_vGravity;
			#endif
		}	
	}

	void CollisionHandling()
	{
		CollisionPlaneYZ(3);
		CollisionPlaneYZ(-3);
		CollisionPlaneXZ(3);

		CollisionSphere(Vector3(0, -6, 0), 6);
	}

	void CollisionPlaneYZ(float x)
	{
		vector<Particle>::iterator pIt;
		for(pIt = _particles.begin(); pIt != _particles.end(); pIt++){
			if (((pIt->pos.x < x) && (x < 0)) || ((pIt->pos.x > x) && (x > 0))){
				pIt->oldPos.x = pIt->pos.x;
				pIt->pos.x = x;
			}
		}
	}

	void DrawCollisionPlaneYZ(float x)
	{
		glBegin(GL_TRIANGLE_STRIP);    
		glVertex3f(x, -x, 1);
		glVertex3f(x, x, 1);
		glVertex3f(x, -x, -1);
		glVertex3f(x, x, -1);
		glEnd();
	}

	void CollisionPlaneXZ(float y)
	{
		vector<Particle>::iterator pIt;
		for(pIt = _particles.begin(); pIt != _particles.end(); pIt++) {
			if (((pIt->pos.y < y) && (y < 0)) || ((pIt->pos.y > y) && (y > 0))){
				pIt->oldPos.y = pIt->pos.y;
				pIt->pos.y = y;
			}
		}
	}

	void DrawCollisionPlaneXZ(float y)
	{
		glBegin(GL_TRIANGLE_STRIP);    
		glVertex3f(-y, y, -1);
		glVertex3f(y, y, -1);
		glVertex3f(-y, y, 1);
		glVertex3f(y, y, 1);
		glEnd();
	}

	void CollisionSphere(const Vector3 & c, float r)
	{
		//|p - c| < r --> p dentro la sfera
		//c + (p-c)/(|p-c|) * r --> vettore dal centro della sfera al punto (approssimato)

		Vector3 q;
		Vector3 d;

		vector<Particle>::iterator pIt;
		for(pIt = _particles.begin(); pIt != _particles.end(); pIt++) {
			d = pIt->pos - c;
			if (d.Length() < r) {
				
				//calcolo posizione approssimata
				q = pIt->pos - c;
				
				//pIt->oldPos = pIt->pos; //ALTRA SOLUZIONE PER IL BOUNCE
				
				q.Normalize();
				q = q * r;
				pIt -> pos = c + q;

				(pIt -> oldPos).y = pIt->oldPos.y - ((pIt->oldPos.y - pIt->pos.y) * 2);//inverto il vettore
			}
		}
	}

	
	// there are n_lat lines of latitude (horizontal) and n_lon lines of longitude (vertical)
	void DrawSphere(const Vector3 & c, float radius)
	{
		glPushMatrix();
		glTranslatef(c.x, c.y, c.z);
		glutSolidSphere(radius, 50, 50);
		glPopMatrix();
	}
};

#endif // __PARTICLE_SYSTEM__