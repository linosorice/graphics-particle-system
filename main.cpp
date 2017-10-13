#ifdef _MSC_VER
#pragma warning(disable:4305)
#pragma warning(disable:4244)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <gl/glut.h>

#include "particle_system.h"

int phase = 0;

GLfloat light_diffuse_0[] = {1, 1, 1, 1};
GLfloat light_positio_0[] = {1, 1, 1, 1};

// size of the window
int window_size_x = 800, window_size_y = 480;

// used for the trackball implementation
const double m_ROTSCALE = 90.0;
const double m_ZOOMSCALE = 0.008;
float fit_factor = 1.f;
Vector3 trackBallMapping(int x, int y);    // Utility routine to convert mouse locations to a virtual hemisphere
Vector3 lastPoint;                         // Keep track of the last mouse location
enum MovementType { ROTATE, ZOOM, NONE };  // Keep track of the current mode of interaction (which mouse button)
MovementType Movement;                     //    Left-mouse => ROTATE, Right-mouse => ZOOM
Vector3 mouse2D, mouse3D;

GLint FPS = 60;		

float angular_speed = 1;

ParticleSystem ps;

// implementation of printf with GLUT
void glPrint(float* c, float x, float y, float z, const char *fmt, ...);

// handles key down event
void keyboardDown(unsigned char key, int x, int y) 
{
	switch(key) 
	{
	case 'a': phase=1; break;
	case 'Q':
	case 'q':
	case  27:   // ESC
		exit(0);
	}

	glutPostRedisplay();
}

// by pressing left and right cursors, ...
void specialDown(int key, int x, int y)
{
	switch(key) 
	{
	case GLUT_KEY_LEFT:
		break;

	case GLUT_KEY_RIGHT:
		break;
	}

	glutPostRedisplay();
}


void reshape(int width, int height) 
{
	window_size_x = width;
	window_size_y = height;

	// Determine the new aspect ratio
	GLdouble gldAspect = (GLdouble) width/ (GLdouble) height;

	// Reset the projection matrix with the new aspect ratio.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, gldAspect, 0.01, 60.0);
	glTranslatef( 0.0, 0.0, -40.0 );

	// Set the viewport to take up the entire window.
	glViewport(0, 0, width, height);
}

// handles when a mouse button is pressed / released
void mouseClick(int button, int state, int x, int y) 
{
	mouse2D = Vector3(x, window_size_y - y, 0);

	if (state == GLUT_UP)
	{
		// Turn-off rotations and zoom.
		Movement = NONE;
		glutPostRedisplay();
		return;
	}

	switch (button)
	{
	case (GLUT_LEFT_BUTTON):

		// Turn on user interactive rotations.
		// As the user moves the mouse, the scene will rotate.
		Movement = ROTATE;

		// Map the mouse position to a logical sphere location.
		// Keep it in the class variable lastPoint.
		lastPoint = trackBallMapping( x, y );

		break;

	case (GLUT_MIDDLE_BUTTON):

		// Turn on user interactive zooming.
		// As the user moves the mouse, the scene will zoom in or out
		//   depending on the x-direction of travel.
		Movement = ZOOM;

		// Set the last point, so future mouse movements can determine
		//   the distance moved.
		lastPoint.x = (double) x;
		lastPoint.y = (double) y;

		break;

	case (GLUT_RIGHT_BUTTON):
		Movement = NONE;	
		break;
	}

	glutPostRedisplay();
}

void mousePassiveMotion(int x, int y) 
{
	GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX1, posY1, posZ1;
    GLdouble posX2, posY2, posZ2;
 
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
 
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
 
    gluUnProject( winX, winY, 0, modelview, projection, viewport, &posX1, &posY1, &posZ1);
    gluUnProject( winX, winY, 1, modelview, projection, viewport, &posX2, &posY2, &posZ2);

	ps.SetAttractor(Vector3((posX1+posX2)/2, (posY1+posY2)/2, (posZ1+posZ2)/2));

}

// handle any necessary mouse movements through the trackball
void mouseMotion(int x, int y) 
{
	Vector3 direction;
	double pixel_diff;
	double rot_angle, zoom_factor;
	Vector3 curPoint;

	switch (Movement) 
	{
	case ROTATE :  // Left-mouse button is being held down
		{
			curPoint = trackBallMapping( x, y );  // Map the mouse position to a logical sphere location.
			direction = curPoint - lastPoint;
			double velocity = direction.Length(); 
			if( velocity > 0.0001 )
			{
				// Rotate about the axis that is perpendicular to the great circle connecting the mouse movements.
				Vector3 rotAxis;
				rotAxis = lastPoint ^ curPoint ;
				rotAxis.Normalize();
				rot_angle = velocity * m_ROTSCALE;

				// We need to apply the rotation as the last transformation.
				//   1. Get the current matrix and save it.
				//   2. Set the matrix to the identity matrix (clear it).
				//   3. Apply the trackball rotation.
				//   4. Pre-multiply it by the saved matrix.
				static GLdouble m[4][4];
				glGetFloatv( GL_MODELVIEW_MATRIX, (GLfloat *) m );
				glLoadIdentity();
				glRotatef( rot_angle, rotAxis.x, rotAxis.y, rotAxis.z );
				glMultMatrixf( (GLfloat *) m );

				//  If we want to see it, we need to force the system to redraw the scene.
				glutPostRedisplay();
			}
			break;
		}
	case ZOOM :  // Right-mouse button is being held down
		//
		// Zoom into or away from the scene based upon how far the mouse moved in the x-direction.
		//   This implementation does this by scaling the eye-space.
		//   This should be the first operation performed by the GL_PROJECTION matrix.
		//   1. Calculate the signed distance
		//       a. movement to the left is negative (zoom out).
		//       b. movement to the right is positive (zoom in).
		//   2. Calculate a scale factor for the scene s = 1 + a*dx
		//   3. Call glScalef to have the scale be the first transformation.
		// 
		pixel_diff = y - lastPoint.y; 
		zoom_factor = 1.0 + pixel_diff * m_ZOOMSCALE;
		glScalef( zoom_factor, zoom_factor, zoom_factor );

		// Set the current point, so the lastPoint will be saved properly below.
		curPoint.x = (double) x;  curPoint.y = (double) y;  (double) curPoint.z = 0;

		//  If we want to see it, we need to force the system to redraw the scene.
		glutPostRedisplay();
		break;
	}

	// Save the location of the current point for the next movement. 
	lastPoint = curPoint;	// in spherical coordinates
	mouse2D = Vector3(x, window_size_y - y, 0);	// in window coordinates
}

// draw the coordinate axes
void DrawAxes(double length)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glPushMatrix();
	glScalef(length, length, length);

	glLineWidth(2.f);
	glBegin(GL_LINES);

	// x red
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(1.f, 0.f, 0.f);

	// y green
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 1.f, 0.f);

	// z blue
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 1.f);

	glEnd();
	glLineWidth(1.f);

	glPopMatrix();
}

// draw information on the screen
void DrawInfo()
{
	glDisable(GL_LIGHTING);
	// -- start Ortographic Mode
	glMatrixMode(GL_PROJECTION);					//Select the projection matrix
	glPushMatrix();									//Store the projection matrix
	glLoadIdentity();								//Reset the projection matrix
	glOrtho(0, window_size_x, window_size_y, 0, -1, 1);		//Set up an ortho screen

	glMatrixMode(GL_MODELVIEW);						//Select the modelview matrix
	glPushMatrix();									//Store the projection matrix

	glLoadIdentity();								//Reset the projection matrix

	float c[3] = {1, 1, 1};
	float y = 25;

	glPrint(c, 10, y, 0, "Hello World"); y += 20;

	glPopMatrix();									//Restore the old projection matrix
	glMatrixMode(GL_PROJECTION);					//Select the projection matrix
	glPopMatrix();									//Restore the old projection matrix
	glMatrixMode(GL_MODELVIEW);
	// -- end Ortographic mode
}


// draw the scene
void draw() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	DrawAxes(1);
	glDisable(GL_COLOR_MATERIAL);

	// draw here

	ps.Draw();



	//DrawInfo();

	glutSwapBuffers();
}


void idle() { }


void initGL(int width, int height) 
{
	glLightfv (GL_LIGHT0, GL_DIFFUSE,	light_diffuse_0);
	glLightfv (GL_LIGHT0, GL_POSITION,	light_positio_0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	reshape(width, height);

	glClearColor(0.126f, 0.126f, 0.228f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);

	ps.Build();
}


void animation(int t)
{
	switch (phase) {
		case 0: break;
		case 1:
			ps.Update();
			glutPostRedisplay();
			break;
	}

	glutTimerFunc((int) 1000/FPS, animation, 0);
}


int main(int argc, char** argv) 
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(window_size_x, window_size_y);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("CG 2012/13 - Assignment #3");

	glutKeyboardFunc(keyboardDown);
	glutSpecialFunc(specialDown);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(mousePassiveMotion);	
	glutReshapeFunc(reshape);
	glutDisplayFunc(draw);  
	glutIdleFunc(idle);
	glutTimerFunc((int) 1000/FPS, animation, 0);
	glutIgnoreKeyRepeat(false); // process keys held down

	initGL(window_size_x, window_size_y);

	glutMainLoop();
	return 0;
}

//
// Utility routine to calculate the 3D position of a 
// projected unit vector onto the xy-plane. Given any
// point on the xy-plane, we can think of it as the projection
// from a sphere down onto the plane. The inverse is what we
// are after.
//
Vector3 trackBallMapping(int x, int y)
{
	Vector3 v;
	double d;

	v.x = (2.0 * x - window_size_x) / window_size_x;
	v.y = (window_size_y - 2.0 * y) / window_size_y;
	v.z = 0.0;
	d = v.Length();
	d = (d < 1.0) ? d : 1.0;  // If d is > 1, then clamp it at one.
	v.z = sqrtf( 1.001 - d * d );  // project the line segment up to the surface of the sphere.

	v.Normalize();  // We forced d to be less than one, not v, so need to normalize somewhere.


	return v;
}



// Custom GL "Print" Routine
// needs glut
void glPrint(float* c, float x, float y, float z, const char *fmt, ...)
{
	if (fmt == NULL)	// If There's No Text
		return;			// Do Nothing

	char text[256];		// Holds Our String
	va_list ap;			// Pointer To List Of Arguments

	va_start(ap, fmt);								// Parses The String For Variables
	vsprintf(text,/* 256 * sizeof(char),*/ fmt, ap);	// And Converts Symbols To Actual Numbers
	va_end(ap);										// Results Are Stored In Text

	size_t len = strlen(text);

	if (c != NULL)
		glColor3fv(c);

	glRasterPos3f(x, y, z);
	for(size_t i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);

}


