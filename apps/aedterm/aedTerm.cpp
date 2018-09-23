/*
 * glutviewer.cpp
 *
 *  Created on: September 16, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <cerrno>
#include <poll.h>
#include <fcntl.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <iostream>
#include "cpu6502.h"
#include "mos6502.h"
#include "dis6502.h"
#include "clk.h"
#include "bus.h"
#include "aedbus.h"
#include "config.h"

static GLuint texName;
static int imageWidth = 0;
static int imageHeight = 0;
static int windowWidth = 512;
static int windowHeight = 512;
static std::vector<uint32_t> imageData;

typedef CLK Clock;

static CPU* cpu;
static AedBus* bus;
static Clock* clk;
static bool debugger = false;
static bool doUpdate = false;
static struct pollfd fds[] = {{ 0, POLLIN, 0 }};
static bool debug = false;
static const std::string DEFAULT_IMAGE_PATH = "aed.ppm";

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

void showprompt() {
    std::cout << "> " << std::flush;
}

static void dasm(int pc, std::ostream& os, int nlines) {
    int count;
    std::string line;
    while (nlines--) {
        std::tie(count, line) = disassemble_6502(&pc,
                [](int offset) { return ::bus->read(offset); });
        os << line << std::endl;
    }
}

static void showline(std::ostream& os) {
    os << std::hex;
    dasm(cpu->get_pc(), os, 1);
    cpu->dump(os);
}

static void maybeUpdateTexture() {
    if (doUpdate) {
        doUpdate = false;

        bus->getFrame(imageData, &imageWidth, &imageHeight);

        // Copy to texture.
        glBindTexture(GL_TEXTURE_2D, texName);
        checkGLError("before glTexSubImage2D");
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_RGBA,
        GL_UNSIGNED_BYTE, &imageData[0]);
        checkGLError("glTexSubImage2D");
    }
}

static void init(int initialWidth, int initialHeight)
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
    imageWidth = initialWidth;
    imageHeight = initialHeight;
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
    maybeUpdateTexture();

    checkGLError("before display()");
    glPushMatrix();
        // Compute (u,v) and (x,y) scaling factors to center the image on the screen
        glClearColor(0.0, 0.0, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (false) {
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
            glPixelZoom((float) windowWidth / imageWidth, (float) windowHeight / imageHeight);
            glRasterPos3f(0, 0, 0);
            glDrawPixels(imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, &imageData[0]);
        }
    glPopMatrix();

    glutSwapBuffers();
    checkGLError("after display()");
}

static uint8_t keymap[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 92, 9, 10, 91, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
        64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
        96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
        110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
        123, 124, 125, 126, 127
};

static void keyboard(unsigned char key, int x, int y)
{
    if (key < 128) {
        bus->key(keymap[key]);
    } else {
        std::cerr << "UNHANDLED KEY: " << (int) key << std::endl;
    }
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
    // TODO: automate this with a signal handler. Should operate at 60Hz.
    if (bus->doVideo(clk->getCpuTime())) {
        static uint64_t last = clk->getCpuTime();
        uint64_t now = clk->getCpuTime();
        if (debug) {
            std::cerr << "vsync " << (float) (now - last) / 1000.0f << "ms" << "\r";
        }
        last = now;
        cpu->nmi();
        if (doUpdate == false) {
            doUpdate = true;
            glutPostRedisplay();
        }
    }
    if (bus->doSerial()) {
        cpu->irq();
    }

    if (debugger) {
        static std::string cmd;
        while (poll(fds, sizeof(fds) / sizeof(fds[0]), 1) > 0) {
            char c;
            if (read(0, &c, 1) > 0) {
                cmd += c;
                if (c == '\n') {
                    switch(cmd[0]) {
                        case 'l':
                            dasm(cpu->get_pc(), std::cout, 10);
                        break;
                        case 'r':
                            cpu->dump(std::cout);
                        break;
                        case 's':
                            cpu->cycle();
                            showline(std::cout);
                        break;
                        case 'c':
                            debugger = false;
                        break;
                        case 'q':
                            exit(0);
                        break;
                        case 'R':
                            std::cerr << "Resetting CPU" << std::endl;
                            cpu->reset();
                            bus->reset();
                            debugger = false;
                        break;
                        case 'W':
                            std::cerr << "Saving file to " << DEFAULT_IMAGE_PATH << std::endl;
                            bus->saveFrame(DEFAULT_IMAGE_PATH);
                        break;
                        case '?':
                        case 'h':
                            std::cout << "(l)ist\n(r)egisters\n(s)tep\n(c)ontinue\n(R)eset\n(W)rite image\n(q)uit\n";
                        break;
                    }
                    if (debugger) { // ignore if 'c' is issued above
                        showprompt();
                    }
                    cmd = "";
                }
            }
        }
    } else { // running
        // Amortize by doing more CPU clocks per idle call
        for (int i = 0; !debugger && i < 1000; i++) {
            cpu->cycle();
        }
        if (poll(fds, sizeof(fds) / sizeof(fds[0]), 1) > 0) {
            char c;
            if (read(0, &c, 1) > 0) {
                bus->send(c);
            }
        }
    }
}

void mouseWheel(int button, int dir, int x, int y)
{
    //glutPostRedisplay();
}

void signalHandler(int) {
    if (debugger) {
        // 2nd <ctrl><c> exits
        std::cout << "Bye!\n";
        exit(0);
    } else {
        debugger = true;
        std::cout << std::endl << "Entering debugger" << std::endl;
        showline(std::cout);
        showprompt();
    }
}

int main(int argc, char **argv)
{
    Clock clock(1000000); // TODO: what frequency?
    AedBus aedbus;
    bus = &aedbus;
    clk = &clock;
    cpu = new USE_CPU([](int addr) { return ::bus->read(addr); },
                        [](int addr, uint8_t value) { ::bus->write(addr, value); },
                        [&clock](int cycles) { clock.add_cpu_cycles(cycles); },
                        [](CPU::ExceptionType ex) {
                            std::cout << "CPU exception " << ex << std::endl;
                            ::debugger = true;
                            ::showprompt(); });

    signal(SIGINT, signalHandler);
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

