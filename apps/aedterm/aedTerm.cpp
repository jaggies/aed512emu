/*
 * glutviewer.cpp
 *
 *  Created on: September 16, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <cerrno>
#include <cstring>
#include <poll.h>
#include <fcntl.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <iostream>
#include "cpu6502.h"
#include "mos6502.h"
#include "dis6502.h"
#include "cpu.h"
#include "clk.h"
#include "bus.h"
#include "aedbus.h"
#include "config.h"
#include "aedsequence.h"

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
static struct pollfd filedesc[5] = {{ 0, POLLIN | POLLPRI, 0 }};
static int nfds = 0;

// Small numbers can be used for more accuracy, but lesser performance.
static size_t CPU_MHZ = 2000000;
static size_t CYCLES_PER_CALL = 5;

static void drawCircle() {
    AedSequence seq;
    seq.mov(256, 256)
    .setlut(0, 0x00, 0x00, 0x80)
    .setlut(1, 0xff, 0xff, 0xff)
    .circle(0x70)
    .send([](uint8_t value) { bus->send(value); });
    return;
}

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
    bus->getFrame(imageData, &imageWidth, &imageHeight);

    // Copy to texture.
    glBindTexture(GL_TEXTURE_2D, texName);
    checkGLError("before glTexSubImage2D");
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_RGBA,
    GL_UNSIGNED_BYTE, &imageData[0]);
    checkGLError("glTexSubImage2D");
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
     drawCircle();
}

static void motion(int x, int y)
{
    std::cerr << __func__ << "(" << x << ", " << y << ")\n";
}

static void passiveMotion(int x, int y)
{
    bus->setJoystick(x, y);
}

static void idle() {
    if (debugger) {
        static std::string cmd;
        if (poll(&filedesc[0], nfds, 1) > 0 && (filedesc[0].revents & POLLIN)) { // Only read stdin
            char c;
            if (read(0, &c, 1) > 0) {
                cmd += c;
                if (c == '\n') {
                    stringstream tokenizer(cmd);
                    std::vector<std::string> tokens;
                    do {
                        string token;
                        tokenizer >> token;
                        if (token.size() > 0) {
                            tokens.push_back(token);
                        }
                    } while (!tokenizer.eof());

                    if (tokens.size() > 0) {
                        switch(tokens[0][0]) {
                            case 'a':
                                if (tokens.size() < 3) {
                                    std::cout << "Usage a <address> <content>" << std::endl;
                                } else {
                                    int addr = std::stoi(tokens[1], NULL, 16);
                                    int data = std::stoi(tokens[2], NULL, 16);
                                    cpu->write_mem(addr, data);
                                }
                            break;
                            case 'd':
                                if (tokens.size() < 2) {
                                    std::cout << "Usage d <addr> <count>" << std::endl;
                                } else {
                                    int n = tokens.size() > 2 ? std::stoi(tokens[2], NULL, 10) : 1;
                                    int base = std::stoi(tokens[1], NULL, 16);
                                    for (int addr = base; addr < base + n; addr++) {
                                        uint8_t data = cpu->read_mem(addr);
                                        std::cout << "\t0x" << addr << " 0x" << (int) data;
                                        if (isprint(data)) {
                                            std::cout << " '" << data << "'";
                                        }
                                        std::cout << std::endl;
                                    }
                                }
                                break;
                            case 'l':
                                if (tokens.size() < 2) {
                                    dasm(cpu->get_pc(), std::cout, 10);
                                } else {
                                    int pc = std::stoi(tokens[1], NULL, 16);
                                    dasm(pc, std::cout, 10);
                                }
                            break;
                            case 'r':
                                cout << std::hex;
                                cpu->dump(std::cout);
                            break;
                            case 's':
                                if (tokens.size() < 2) {
                                    cpu->cycle();
                                } else {
                                    int count = std::stoi(tokens[1], NULL, 10);
                                    cpu->cycle(count);
                                }
                                bus->handleEvents(clk->getCpuTime());
                                showline(std::cout);
                            break;
                            case 'b':
                                if (tokens.size() < 2) {
                                    std::cout << "Breakpoints:" << std::endl;
                                    for(const uint32_t& addr : cpu->getBreakpoints()) {
                                        std::cout << "\t0x" << addr << std::endl;
                                    }
                                } else {
                                    int pc = std::stoi(tokens[1], NULL, 16);
                                    cpu->addBreak(pc);
                                    std::cout << "Breakpoint set at 0x" << pc << std::endl;
                                }
                            break;
                            case 'w':
                                if (tokens.size() < 2) {
                                    std::cout << "Watchpoints:" << std::endl;
                                    for(const uint32_t& addr : cpu->getWatchpoints()) {
                                        std::cout << "\t0x" << addr << std::endl;
                                    }
                                } else {
                                    int addr = std::stoi(tokens[1], NULL, 16);
                                    cpu->addWatch(addr);
                                    std::cout << "Watchpoint set at 0x" << addr << std::endl;
                                }
                            break;
                            case 'c':
                                debugger = false;
                            break;
                            case 'q':
                                exit(0);
                            break;
                            case 'R':
                                std::cerr << "Resetting Machine" << std::endl;
                                cpu->reset();
                                bus->reset();
                                debugger = false;
                            break;
                            case 'W':
                                if (tokens.size() < 2) {
                                    std::cout << "Usage: W <imagefilename>\n";
                                } else {
                                    std::cerr << "Saving file to " << tokens[1] << std::endl;
                                    bus->saveFrame(tokens[1]);
                                }
                            break;
                            case '?':
                            case 'h':
                                std::cout <<
                                        "(a)ssign <addr> <data>\tSets byte at addr to data\n"
                                        "(d)ump <addr>\tDumps display memory at addr]\n"
                                        "(l)ist <addr>\tDisassembles address\n"
                                        "(r)egisters\tDisplay CPU registers\n"
                                        "(s)tep <n>\tStep n instructions\n"
                                        "(b)reak <addr>\tSets a breakpoint at address\n"
                                        "(w)atch <addr>\tSets a watch point at address\n"
                                        "(c)ontinue\n"
                                        "(R)eset\n"
                                        "(W)rite image\n"
                                        "(q)uit\n";
                            break;
                        }
                    }
                    if (debugger) { // ignore if 'c' is issued above
                        showprompt();
                    }
                    cmd = "";
                }
            }
        }
    } else { // running
        cpu->cycle(CYCLES_PER_CALL);
        if (poll(&filedesc[0], nfds, 0) > 0) {
            for (int n = 0; n < nfds; n++) {
                char c;
                if (filedesc[n].revents & POLLIN) {
                    if (read(filedesc[n].fd, &c, 1) > 0) {
                        bus->send(c);
                    }
                }
            }
        }
        bus->handleEvents(clk->getCpuTime());
    }
}

void mouseWheel(int button, int dir, int x, int y)
{
    //glutPostRedisplay();
}

void signalHandler(int) {
    if (!debugger) {
        debugger = true;
        std::cout << std::endl << "Entering debugger" << std::endl;
        showline(std::cout);
    } else {
        std::cout << std::endl;
    }
    showprompt();
}

void handleException(CPU::ExceptionType ex, int pc) {
    std::cout << std::hex;
    switch (ex) {
        case CPU::ILLEGAL_INSTRUCTION:
            std::cout << "Illegal instruction at PC = " << pc << std::endl;
            dasm(pc, std::cout, 1);
        break;
        case CPU::WATCH_POINT:
            std::cout << "Watchpoint at PC = " << pc << std::endl;
            dasm(pc, std::cout, 1);
        break;
        case CPU::BREAK_POINT:
            std::cout << "Break point at PC = " << pc << std::endl;
            dasm(pc, std::cout, 1);
        break;
        default:
            std::cout << "Unknown exception at PC = " << pc << ", ex=" << ex << std::endl;
            dasm(pc, std::cout, 1);
    }
    debugger = true;
    showprompt();
}

int main(int argc, char **argv)
{
    clk = new Clock(CPU_MHZ); // TODO: what frequency?
    bus = new AedBus([]() { ::cpu->irq(); }, []() { ::cpu->nmi(); ::glutPostRedisplay(); });
    cpu = new USE_CPU(
            [](int addr) { return ::bus->read(addr); },
            [](int addr, uint8_t value) { ::bus->write(addr, value); },
            [](int cycles) { ::clk->add_cpu_cycles(cycles); ::bus->setCpuTime(clk->getCpuTime()); },
            [](CPU::ExceptionType ex, int pc) { ::handleException(ex, pc); });

    // Add various file descriptors
    filedesc[nfds] = {0, POLLIN, 0 }; nfds++;

    std::cerr << "Opening additional file descs\n";
    if (int fd = open("input", O_RDONLY | O_NONBLOCK)) {
        if (fd > 0) {
            std::cerr << "Adding input file at descriptor " << fd << std::endl;
            filedesc[nfds] = {fd, POLLIN | POLLPRI, 0 }; nfds++;
        }
    }

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

