/*
 * glutviewer.cpp
 *
 *  Created on: September 16, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <string>
#include <vector>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <iostream>
#include "cpu6502.h"
#include "clk.h"
#include "bus.h"
#include "aedbus.h"

static GLuint texName;
static int imageWidth = 0;
static int imageHeight = 0;
static int windowWidth = 512;
static int windowHeight = 512;
static std::vector<uint32_t> imageData(512*512, 0xff00ff00);

class Clock : public CLK {
    public:
        Clock() : _count(0) { }
        ~Clock() = default;
        void add_cpu_cycles(size_t cycles) { _count += cycles; }
        size_t getCount() const { return _count; }
        void reset() { _count = 0; }
    private:
        size_t _count;
};

static CPU6502<Clock, AedBus>* cpu;
static AedBus* bus;

static void checkGLError(const char* msg) {
    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "GL Error (" << msg << ") : " << gluErrorString(err) << std::endl;
    }
}

void
displayString(float x, float y, char *string)
{
    int len, i;
    glRasterPos2f(x, y);
    len = (int) strlen(string);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
        #ifdef DEBUG
        if (GLenum err = glGetError() != GL_NO_ERROR) {
            printf( "Error calling glutBitmapChar():%s\n", gluErrorString(err));
        }
        #endif
    }
}

static void updateTexture()
{
    glBindTexture(GL_TEXTURE_2D, texName);
    checkGLError("before glTexSubImage2D");
    glTexSubImage2D(GL_TEXTURE_2D, 0,0,0, imageWidth, imageHeight, GL_RGBA,
            GL_UNSIGNED_BYTE, &imageData[0]);
    checkGLError("glTexSubImage2D");
}

static void init(int width, int height)
{
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glDisable(GL_DEPTH_TEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    imageWidth = width;
    imageHeight = height;
    imageData.resize(imageWidth * imageHeight);
    checkGLError("before glTexImage2d");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
            &imageData[0]);
    checkGLError("after glTexImage2d");
}

static void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,1.0f,0.0f,1.0f,0.0f,100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    windowWidth = w;
    windowHeight = h;
}

static void display(void)
{
    checkGLError("before display()");
    glPushMatrix();
        // Compute (u,v) and (x,y) scaling factors to center the image on the screen
        glClearColor(0.0, 0.0, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (true) {
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
            glBindTexture(GL_TEXTURE_2D, texName);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
                glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 1.0, 0.0);
                glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
                glTexCoord2f(1.0, 0.0); glVertex3f(1.0, 0.0, 0.0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        } else {
            glPixelZoom(1.0, -1.0);
            glRasterPos3f(0, 1, 0);
            glDrawPixels(imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, &imageData[0]);
        }
    glPopMatrix();

    glutSwapBuffers();
    checkGLError("after display()");
}

static void keyboard(unsigned char key, int x, int y)
{
    bus->send(key);
}

static void mouse(int button, int state, int x, int y)
{
}

static void motion(int x, int y)
{
}

static void passiveMotion(int x, int y)
{
}

static void idle() {
    // Amortize by doing more CPU clocks per idle call
    for (int i = 0; i < 100; i++) {
        cpu->cycle();
    }
    // TODO: automate this with a signal handler. Should operate at 60Hz.
    if (bus->doVideo()) {
        cpu->nmi();
        cpu->irq();
        // copy pixels to texture
        const std::vector<uint8_t>& display = bus->getVideoMemory();
        for (int y = 0; y < bus->getDisplayHeight(); y++) {
            size_t row = (bus->getDisplayHeight() - y - 1) * bus->getDisplayWidth();
            for (int x = 0; x < bus->getDisplayWidth(); x++) {
                uint8_t video = display[row + x]; // TODO: use CLUT
                imageData[row + x]= 0xff000000 | (video << 16) | (video << 8) | video;
            }
        }
        updateTexture();
        glutPostRedisplay();
    }
    if (bus->doSerial()) {
        cpu->irq();
    }
}

void mouseWheel(int button, int dir, int x, int y)
{
    //glutPostRedisplay();
}

int main(int argc, char **argv)
{
    Clock clock;
    bus = new AedBus;
    cpu = new CPU6502<Clock, AedBus>(CPU6502<Clock, AedBus>(clock, *bus));
    cpu->reset();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    //glutInitDisplayString("rgba stencil double samples=64 hidpi");
    glutInitWindowSize(bus->getDisplayWidth(), bus->getDisplayHeight());
    glutIdleFunc(idle);
    glutCreateWindow(argv[0]);
    checkGLError("before gentex");
    glGenTextures(1, &texName);
    checkGLError("before init");
    init(bus->getDisplayWidth(), bus->getDisplayHeight());
    checkGLError("after init");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    //glutMouseWheelFunc(mouseWheel);
    glutPassiveMotionFunc(passiveMotion);
    glutMainLoop();
    return 0;
}

