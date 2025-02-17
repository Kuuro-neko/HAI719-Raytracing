// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include "src/Vec3.h"
#include "src/Camera.h"
#include "src/Scene.h"
#include <GL/glut.h>

#include "src/matrixUtilities.h"

using namespace std;

#include "src/imageLoader.h"

#include "src/Material.h"

#include <time.h> 
#include "src/Functions.h"

#include <thread>
#include <random>

#include "src/Constants.h"

// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 850; // 480
static unsigned int SCREENHEIGHT = 480; // 480
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static unsigned int FPS = 0;
static bool fullScreen = false;

std::vector<Scene> scenes;
unsigned int selected_scene;
unsigned int nsamples = DEFAULT_NSAMPLES;

MatrixUtilities matrixUtilities;

std::vector< std::pair< Vec3 , Vec3 > > rays;

void printUsage () {
    cerr << endl
         << "gMini: a minimal OpenGL/GLUT application" << endl
         << "for 3D graphics." << endl
         << "Author : Tamy Boubekeur (http://www.labri.fr/~boubek)" << endl << endl
         << "Usage : ./gmini [<file.off>]" << endl
         << "Keyboard commands" << endl
         << "------------------" << endl
         << " ?: Print help" << endl
         << " w: Toggle Wireframe Mode" << endl
         << " r: Ray trace the scene" << endl
         << " u: Recompute the random scenes" << endl
         << " f: Toggle full screen mode" << endl
         << " S/s: Increase/decrease the number of samples per pixel" << endl
         << " +/-: Change scene" << endl
         << " <drag>+<left button>: rotate model" << endl
         << " <drag>+<right button>: move model" << endl
         << " <drag>+<middle button>: zoom" << endl
         << " q, <esc>: Quit" << endl << endl;
}

void usage () {
    printUsage ();
    exit (EXIT_FAILURE);
}


// ------------------------------------
void initLight () {
    GLfloat light_position[4] = {0.0, 1.5, 0.0, 1.0};
    GLfloat color[4] = { 1.0, 1.0, 1.0, 1.0};
    GLfloat ambient[4] = { 1.0, 1.0, 1.0, 1.0};

    glLightfv (GL_LIGHT1, GL_POSITION, light_position);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color);
    glLightfv (GL_LIGHT1, GL_SPECULAR, color);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
}

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    //glCullFace (GL_BACK);
    glDisable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
}


// ------------------------------------
// Replace the code of this 
// functions for cleaning memory, 
// closing sockets, etc.
// ------------------------------------

void clear () {

}

// ------------------------------------
// Replace the code of this 
// functions for alternative rendering.
// ------------------------------------


void draw () {
    glEnable(GL_LIGHTING);
    scenes[selected_scene].draw();

    // draw rays : (for debug)
    //  std::cout << rays.size() << std::endl;
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(6);
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    for( unsigned int r = 0 ; r < rays.size() ; ++r ) {
        glVertex3f( rays[r].first[0],rays[r].first[1],rays[r].first[2] );
        glVertex3f( rays[r].second[0], rays[r].second[1], rays[r].second[2] );
    }
    glEnd();
}

void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw ();
    glFlush ();
    glutSwapBuffers ();
}

void idle () {
    static float lastTime = glutGet ((GLenum)GLUT_ELAPSED_TIME);
    static unsigned int counter = 0;
    counter++;
    float currentTime = glutGet ((GLenum)GLUT_ELAPSED_TIME);
    if (currentTime - lastTime >= 1000.0f) {
        FPS = counter;
        counter = 0;
        static char winTitle [64];
        sprintf (winTitle, "Raytracer - FPS: %d - Ray samples: %d", FPS, nsamples);
        glutSetWindowTitle (winTitle);
        lastTime = currentTime;
    }
    glutPostRedisplay ();
}

thread_local std::mt19937 rng(std::random_device{}());

void trace_line(int y, int w, int h, unsigned int nsamples, std::vector<Vec3>& image) {
    Vec3 pos, dir;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int x = 0; x < w; x++) {
        for (unsigned int s = 0; s < nsamples; ++s) {
            float u = ((float)(x) + dist(rng)) / w;
            float v = ((float)(y) + dist(rng)) / h;
            matrixUtilities.screen_space_to_world_space_ray(u, v, pos, dir);
            Vec3 color = scenes[selected_scene].rayTrace(Ray(pos, dir, dist(rng)));
            image[x + y * w] += color;
        }
        image[x + y * w] /= nsamples;
        gamma_correct(image[x + y * w]);
    }
}

void ray_trace_from_camera() {
    int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);
    std::vector<Vec3> image(w * h, Vec3(0, 0, 0));
    std::vector<std::thread> threads;

    unsigned int nb_threads;
    if (MULTI_THREADED) {
        nb_threads = std::thread::hardware_concurrency();
    } else {
        nb_threads = 1;
    }
    
    camera.apply();
    matrixUtilities.updated();
    matrixUtilities.updateMatrices();
    clock_t start = clock();
    if (MONORAY) {
        int x = 220;
        int y = 270;
        // send a ray to the x and y position of the final screen, and use the resulting color on all the screen
        std::cout << "Sending only one ray to the screen position (" << x << ", " << y << ") and using the resulting color for the whole image" << std::endl;
        Vec3 pos, dir;
        matrixUtilities.screen_space_to_world_space_ray(x / (float)w, y / (float)h, pos, dir);
        Vec3 color = scenes[selected_scene].rayTrace(Ray(pos, dir, 0.f));
        gamma_correct(color);
        for (int i = 0; i < w * h; i++) {
            image[i] = color;
        }
    } else {
        if (MULTI_THREADED) {
            // multi-threading
            std::cout << "Ray tracing a " << w << " x " << h << " image using " << nb_threads << " threads and " << nsamples << " samples per pixel" << std::endl;
            for (int y = 0; y < h; y++) {
                threads.emplace_back(trace_line, y, w, h, nsamples, std::ref(image));
            }

            for (auto& t : threads) {
                t.join();
            }
        } else {
            // single-threading
            std::cout << "Ray tracing a " << w << " x " << h << " image using 1 thread and " << nsamples << " samples per pixel" << std::endl;
            for (int y = 0; y < h; y++) {
                trace_line(y, w, h, nsamples, image);
            }
        }
    }

    clock_t end = clock();
    std::cout << "  Done in " << (double)(end - start) / CLOCKS_PER_SEC / nb_threads << " seconds" << std::endl;

    // Save image
    std::string filename = "./rendu.ppm";
    ofstream f(filename.c_str(), ios::binary);
    if (f.fail()) {
        cout << "Could not open file: " << filename << endl;
        return;
    }
    f << "P3" << std::endl << w << " " << h << std::endl << 255 << std::endl;
    for (int i=0; i<w*h; i++)
        f << (int)(255.f*std::min<float>(1.f,image[i][0])) << " " << (int)(255.f*std::min<float>(1.f,image[i][1])) << " " << (int)(255.f*std::min<float>(1.f,image[i][2])) << " ";
    f << std::endl;
    f.close();
}


void key (unsigned char keyPressed, int x, int y) {
    Vec3 pos , dir;
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }
        break;
    case 'q':
    case 27:
        clear ();
        exit (0);
        break;
    case 'w':
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, polygonMode);
        if(polygonMode[0] != GL_FILL)
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        break;
    case 'S':
        if (nsamples < 5) {
            nsamples += 1;
        } else if (nsamples < 25) {
            nsamples += 5;
        } else if (nsamples < 100) {
            nsamples += 25;
        } else if (nsamples < 250) {
            nsamples += 50;
        } else if (nsamples < 1000) {
            nsamples += 250;
        } else {
            nsamples += 500;
        }
        break;
    case 's':
        if (nsamples > 1000) {
            nsamples -= 500;
        } else if (nsamples > 250) {
            nsamples -= 250;
        } else if (nsamples > 100) {
            nsamples -= 50;
        } else if (nsamples > 25) {
            nsamples -= 25;
        } else if (nsamples > 5) {
            nsamples -= 5;
        } else if (nsamples > 1) {
            nsamples -= 1;
        }
        break;
    case 'r':
        camera.apply();
        rays.clear();
        ray_trace_from_camera();
        
        break;
    case 'u':
        scenes[5].setup_random_spheres();
        break;
    case '-':
        selected_scene = (selected_scene + scenes.size() - 1) % scenes.size();
        break;
    case '+':
        selected_scene++;
        if( selected_scene >= scenes.size() ) selected_scene = 0;
        break;
    default:
        printUsage ();
        break;
    }
    idle ();
}

void mouse (int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }
    idle ();
}

void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
    scenes[2].setup_cornell_box(float(w)/float(h));
}





int main (int argc, char ** argv) {
    if (argc > 2) {
        printUsage ();
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("gMini");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);


    camera.move(0., 0., -3.1);
    matrixUtilities = MatrixUtilities();
    selected_scene=DEFAULT_SELECTED_SCENE;
    scenes.resize(11);
    scenes[0].setup_single_sphere();
    scenes[1].setup_single_square();
    scenes[2].setup_cornell_box(float(SCREENWIDTH)/float(SCREENHEIGHT));
    scenes[3].setup_mesh();
    scenes[4].setup_rt_in_a_weekend();
    scenes[5].setup_random_spheres();
    scenes[6].setup_debug_refraction();
    scenes[7].setup_flamingo();
    scenes[8].setup_raccoon();
    scenes[9].setup_flamingo_pond();
    scenes[10].setup_backrooms_pool();


    glutMainLoop ();
    return EXIT_SUCCESS;
}

