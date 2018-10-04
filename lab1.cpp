//
//modified by: Anahi Vargas
//date: 09/06/2018
//
//3350 Spring 2018 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
// .general animation framework
// .animation loop
// .object definition and movement
// .collision detection
// .mouse/keyboard interaction
// .object constructor
// .coding style
// .defined constants
// .use of static variables
// .dynamic memory allocation
// .simple opengl components
// .git
//
//elements we will add to program...
//   .Game constructor
//   .multiple particles
//   .gravity
//   .collision detection
//   .more objects
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

const int MAX_PARTICLES = 5000;
const float GRAVITY = 0.1;

//some structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

class Global {
public:
	int xres, yres;
	Shape box;
	Particle particle[MAX_PARTICLES];
	int n;
	Global() {
		xres = 800;
		yres = 600;
		//define a box shape
		box.width = 100;
		box.height = 10;
		box.center.x = 120 + 5*65;
		box.center.y = 500 - 5*60;
		n = 0;
	}
} g;

class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	X11_wrapper() {
		GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
		int w = g.xres, h = g.yres;
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			cout << "\n\tcannot connect to X server\n" << endl;
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL) {
			cout << "\n\tno appropriate visual found\n" << endl;
			exit(EXIT_FAILURE);
		} 
		Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		XSetWindowAttributes swa;
		swa.colormap = cmap;
		swa.event_mask =
			ExposureMask | KeyPressMask | KeyReleaseMask |
			ButtonPress | ButtonReleaseMask |
			PointerMotionMask |
			StructureNotifyMask | SubstructureNotifyMask;
		win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
			InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
		set_title();
		glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
	}
	void set_title() {
		//Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, "3350 Lab1");
	}
	bool getXPending() {
		//See if there are pending events.
		return XPending(dpy);
	}
	XEvent getXNextEvent() {
		//Get a pending event.
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void swapBuffers() {
		glXSwapBuffers(dpy, win);
	}
} x11;

//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();


//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
	srand(time(NULL));
	init_opengl();
	//Main animation loop
	int done = 0;
	while (!done) {
		//Process external events.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			check_mouse(&e);
			done = check_keys(&e);
		}
		movement();
		render();
		x11.swapBuffers();
	}
    cleanup_fonts();
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(int x, int y)
{
	if (g.n >= MAX_PARTICLES)
		return;
	cout << "makeParticle() " << x << " " << y << endl;
	//position of particle
	Particle *p = &g.particle[g.n];
	p->s.center.x = 40;
	p->s.center.y = 510;
	p->velocity.y = (float)rand()/RAND_MAX * 2.0 - 4.0;
	p->velocity.x = (float)rand()/RAND_MAX * 2.0;
	++g.n;
}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = g.yres - e->xbutton.y;
			makeParticle(e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			int y = g.yres - e->xbutton.y;

			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
		}
	}
}

int check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
				//Key 1 was pressed
				break;
			case XK_a:
				//Key A was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void movement()
{
	if (g.n <= 0)
		return;
	for (int i = 0; i<g.n; i++){
    	Particle *p = &g.particle[i];
    	p->s.center.x += p->velocity.x;
    	p->s.center.y += p->velocity.y;
    	p->velocity.y -= GRAVITY;

	    //check for collision with shapes...
    	Shape *s = &g.box;

        if (p->s.center.y < (s->center.y + s->height) &&
	    	p->s.center.x > s->center.x - s->width &&
	    	p->s.center.x < s->center.x + s->width)
    	{
            p->velocity.y *= -1.0;
            p->velocity.y *=0.8;
        }

    	//check for off-screen
    	if (p->s.center.y < 0.0) {
	    	cout << "off screen" << endl;
	    	g.particle[i] = g.particle[g.n-1];
	    	--g.n;
        }
    }
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	float w, h;
	//Draw shapes...
    Shape *s;
	s = &g.box;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	
	//draw a box
   
    //Creates five boxes
    for (int i = 0; i < 5; i++) {
        glVertex2i(-w - i * 80, -h + i * 70);
        glVertex2i(-w - i * 80,  h + i * 70);
        glVertex2i( w - i * 80,  h + i * 70);
        glVertex2i( w - i * 80, -h + i * 70);

        if (i == 0) 
            glColor3ub(128, 193, 255);
        
        else if (i == 1) 
            glColor3ub (77, 169, 255);
        
        else if (i == 2) 
            glColor3ub (26, 144, 255);
        
        else if (i == 3) 
            glColor3ub (0, 119, 255);
        
        else if (i == 4) 
            glColor3ub (179, 218, 255);
    }
	glEnd();
	glPopMatrix();
	
	//Draw the particle here
	for (int i=0; i<g.n; i++) {
	    glPushMatrix();
	    glColor3ub(207,241,255);
	    Vec *c = &g.particle[i].s.center;
	    w = 2;
	    h = 2;
	    glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
	    glEnd();
	    glPopMatrix();
	}
    void drawText(int x, int y);
    drawText(30, g.yres-100);
}
	//Draw your 2D text here
void drawText (int x, int y)
{
    Rect r;
    int a = 0;
    int b = 0;
    for (int i = 0; i < 5; i++) {
        r.bot = y  - a;
        r.left = x + b;
        r.center = 0;
        if (i == 0) {
            ggprint8b(&r, 40, 0x0044ffff, "Requirements");
        } else if (i == 1) {
            ggprint8b(&r, 40, 0x0044ffff, "Design");
        } else if (i == 2) {
            ggprint8b(&r, 40, 0x0044ffff, "Implementation");
        } else if (i == 3){    
            ggprint8b(&r, 40, 0x0044ffff, "Testing");
        } else {
            ggprint8b(&r, 40, 0x0044ffff, "Maintenance");
        }
        a += 70;
        b += 90;
    }
}   
