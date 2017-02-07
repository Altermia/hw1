//modified by: James Kim
//purpose: HW
//date: 1/27/17
//
//cs3350 Spring 2017 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 4000
#define GRAVITY 0.15
#define rnd() (float)rand() / (float)RAND_MAX
#define MAX_BOXES 5

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

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

struct Game {
    Shape box[MAX_BOXES];
    Particle particle[MAX_PARTICLES];
    Shape circle;
    int n;
    int bubbler;
    int mouse[2];
    Game() {n = 0; bubbler = 0;}
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);

int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    game.n=0;

    //declare a box shape
    for (int i = 0; i < MAX_BOXES; i++){
	game.box[i].width = 100;
	game.box[i].height = 10;
	game.box[i].center.x = 150 + i*75;
	game.box[i].center.y = 500 - i*60;
    }

    game.circle.radius = 250;
    game.circle.center.x = 700;
    game.circle.center.y = 0;

    //start animation
    while (!done) {
	while (XPending(dpy)) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    check_mouse(&e, &game);
	    done = check_keys(&e, &game);
	}
	movement(&game);
	render(&game);
	glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void)
{
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void)
{
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
	std::cout << "\n\tcannot connect to X server\n" << std::endl;
	exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
	std::cout << "\n\tno appropriate visual found\n" << std::endl;
	exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	ButtonPress | ButtonReleaseMask | PointerMotionMask |
	StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
	    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(Game *game, int x, int y)
{
    if (game->n >= MAX_PARTICLES)
	return;
    std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y =  rnd() * 0.5 - 0.25;
    p->velocity.x =  rnd() * 0.5 - 0.25;
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    //static int n = 0;

    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    int y = WINDOW_HEIGHT - e->xbutton.y;
	    for (int i = 0; i < 10; i++) {
		makeParticle(game, e->xbutton.x, y);
	    }
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    return;
	}
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
	savex = e->xbutton.x;
	savey = e->xbutton.y;
	int y = WINDOW_HEIGHT - e->xbutton.y;
	if (game->bubbler == 0) {
	    game->mouse[0] = savex;
	    game->mouse[1] = y;
	}
	for (int i = 0; i < 5; i++) {
	    makeParticle(game, e->xbutton.x, y);
	}
	//if (++n < 10)
	//	return;
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
	int key = XLookupKeysym(&e->xkey, 0);
	if (key == XK_b) {
	    game->bubbler ^= 1;				//^= exclusive or assignment
	}
	if (key == XK_Escape) {
	    return 1;
	}
	//You may check other keys here.



    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if (game->n <= 0)
	return;

    if (game->bubbler != 0) {
	//the bubbler is on
	makeParticle(game, game->mouse[0], game->mouse[1]);
    }
    for (int j = 0; j < game->n; j++) {
	p = &game->particle[j];
	p->velocity.y -= GRAVITY;
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;

	//check for collision with shapes...
	Shape *s;
	for (int i = 0; i < MAX_BOXES; i++) {
	    s = &game->box[i];
	    if(p->s.center.x <= s->center.x + s->width &&
		    p->s.center.x >= s->center.x - s->width) {
		if (p->s.center.y < s->center.y + s->height &&
			p->s.center.y <= s->center.y + s->height) {
		    p->s.center.y = s->center.y + s->height - .05;
		    p->velocity.y = -p->velocity.y * (0.8f + GRAVITY);
		    p->velocity.x = 0.5f;		
		}
	    }
	}

	if(p->s.center.x <= WINDOW_WIDTH &&
		p->s.center.x >= 0 &&
		p->s.center.y == 50){
	    p->s.center.y = 50;
	    p->velocity.x = -p->velocity.y * (.8f + GRAVITY);
	    p->velocity.x = 0.2f * (.8f + rnd());
	}



	//check for off-screen
	if (p->s.center.y < 0.0) {
	    std::cout << "off screen" << std::endl;	
	    game->particle[j] = game->particle[--game->n];
	}
    }
}

void render(Game *game)
{
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...

    //draw box
    Shape *s[MAX_BOXES];
    Rect r;
    char ts[5][32] = {"Requirements", "Design", "Coding", "Testing", "Maintenance"};
    for (int i = 0; i < MAX_BOXES; i++) {
	glColor3ub(90,140,90);
	s[i] = &game->box[i];
	glPushMatrix();
	glTranslatef(s[i]->center.x, s[i]->center.y, s[i]->center.z);
	w = s[i]->width;
	h = s[i]->height;
	glBegin(GL_QUADS);
	glVertex2i(-w,-h);
	glVertex2i(-w, h);
	glVertex2i( w, h);
	glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
	
	r.bot = s[i]->center.y - 10;
	r.left = s[i]->center.x;
	r.center = s[i]->center.y;
	ggprint13(&r, 20,-50, "%s", ts[i]);
    }

    Shape *c;
    c = &game->circle;
    int i, x, y;
    double radius = game->circle.radius;
    glColor3ub(68,101,189);
    double twicePi = 2.0 * 3.142;
    x = c->center.x, y = c->center.y;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x,y);
    for (i = 0; i <= 50; i++) {
	glVertex2f (
		(x + (radius * cos(i * twicePi / 50))), (y + (radius * sin(i * twicePi / 50)))
		);
    }
    glEnd();

    //draw all particles here
    for (int i = 0; i < game->n; i++) {
	glPushMatrix();
	glColor3ub(86,135,255);
	Vec *c = &game->particle[i].s.center;
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
}
