#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <OpenGL/GL.h>
#include <iostream>
#include "sdl.hpp"

using std::cerr;
using std::endl;

struct SdlWindow::_SdlHandle {
    SDL_Window * hwnd;
    SDL_GLContext gl_ctx;
    _SdlHandle(SDL_Window * window)
        : hwnd(window)
        , gl_ctx(0) { }

    ~_SdlHandle() {
        if (gl_ctx)
            SDL_GL_DeleteContext(gl_ctx);
        SDL_DestroyWindow(hwnd);
    }
};

bool SdlWindow::isGlInitialized() {
    return (_handle->gl_ctx != 0);
}

SdlWindow::SdlWindow(const char * title, int w, int h)
    : requiresExpose(false) {

    if (!SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            cerr << "Failed to initialize SDL: " << SDL_GetError() << endl;
            return;
        }
    }

    _handle = std::make_shared<_SdlHandle>(SDL_CreateWindow(title,
                                                SDL_WINDOWPOS_UNDEFINED,
                                                SDL_WINDOWPOS_UNDEFINED,
                                                w,
                                                h,
                                                SDL_WINDOW_OPENGL));
}

bool SdlWindow::createGlContext() {
    if (!_handle) {
        cerr << "Can't initialize an OpenGL context without a valid window" << endl;
        return false;
    }
    if (_handle->gl_ctx) {
        // destroy existing opengl context
        SDL_GL_DeleteContext(_handle->gl_ctx);
        _handle->gl_ctx = 0;
    }
    // on OSX systems, only core profiles are available for OpenGL 3+, which
    // removes the fixed-function pipeline
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    // technically, SDL already defaults to double buffering and a depth buffer
    // all we need is an alpha channel

    //SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 1);
    //SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 1);

    SDL_GLContext context = SDL_GL_CreateContext(_handle->hwnd);
    if (!context) {
        cerr << "Failed to create an OpenGL 2.1 context: %s" << SDL_GetError() << endl;
        return false;
    }
    _handle->gl_ctx = context;

    SDL_GL_SetSwapInterval(1);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "Failed to initialize GLEW: %s" << glewGetErrorString(err) << endl;
        return false;
    }
    return true;
}

SdlWindow::~SdlWindow() {
}

bool SdlWindow::windowEvent(SDL_WindowEvent& ew) {
    switch(ew.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            cerr << "Window:reshape event" << endl;
            if (onReshape)
                onReshape(ew.data1, ew.data2);
            return false;
        case SDL_WINDOWEVENT_EXPOSED:
            cerr << "Window:expose event" << endl;
            if (onExpose)
                onExpose();
            return true;
        default:
            return false;
    }
}

void SdlWindow::motionEvent(SDL_MouseMotionEvent& em) {
    EventInfo info;
    info.event = AUX_MOUSELOC;
    info.data[AUX_MOUSEX] = em.x;
    info.data[AUX_MOUSEY] = em.y;
    info.data[2] = SDL_GetModState();
    if (em.state & SDL_BUTTON_LMASK) {
        info.data[AUX_MOUSESTATUS] = AUX_LEFTBUTTON;
        if (onMouseMove[SDL_BUTTON_LEFT]) {
            onMouseMove[SDL_BUTTON_LEFT](&info);
        }
    } else if (em.state & SDL_BUTTON_RMASK) {
        info.data[AUX_MOUSESTATUS] = AUX_RIGHTBUTTON;
        if (onMouseMove[SDL_BUTTON_RIGHT]) {
            onMouseMove[SDL_BUTTON_RIGHT](&info); 
        }
    } else if (em.state & SDL_BUTTON_MMASK) {
        info.data[AUX_MOUSESTATUS] = AUX_MIDDLEBUTTON;
        if (onMouseMove[SDL_BUTTON_MIDDLE]) {
            onMouseMove[SDL_BUTTON_MIDDLE](&info); 
        }
    }
}

void SdlWindow::mouseEventDown(SDL_MouseButtonEvent& eb) {
    if (onMouseDown[eb.button]) {
        EventInfo info;
        info.event = AUX_MOUSEDOWN;
        info.data[AUX_MOUSEX] = eb.x;
        info.data[AUX_MOUSEY] = eb.y;
        info.data[2] = SDL_GetModState();
        info.data[AUX_MOUSESTATUS] = eb.button;
        onMouseDown[eb.button](&info); 
    }
}

void SdlWindow::mouseEventUp(SDL_MouseButtonEvent& eb) {
    if (onMouseUp[eb.button]) {
        EventInfo info;
        info.event = AUX_MOUSEUP;
        info.data[AUX_MOUSEX] = eb.x;
        info.data[AUX_MOUSEY] = eb.y;
        info.data[2] = SDL_GetModState();
        info.data[AUX_MOUSESTATUS] = eb.button;
        onMouseUp[eb.button](&info);
    }
}

void SdlWindow::keyEvent(SDL_Keysym& ks) {
    if (ks.mod & KMOD_SHIFT) {
        //check if separate caps handler exists
        if (onKeyDown[toupper(ks.sym)]) {
            onKeyDown[toupper(ks.sym)](ks.mod);
            return;
        }
    }
    if (onKeyDown[ks.sym]) {
        onKeyDown[ks.sym](ks.mod);
    }
}

void SdlWindow::mainLoop() {
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_WINDOWEVENT:
                    if (windowEvent(e.window))
                        SDL_GL_SwapWindow(_handle->hwnd);
                    break;
                case SDL_KEYDOWN:
                    cerr << "Key down event" << endl;
                    keyEvent(e.key.keysym);
                    break;
                case SDL_MOUSEMOTION:
                    cerr << "Mouse move event" << endl;
                    motionEvent(e.motion);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    cerr << "Mouse down event" << endl;
                    mouseEventDown(e.button);
                    break;
                case SDL_MOUSEBUTTONUP:
                    cerr << "Mosue up event" << endl;
                    mouseEventUp(e.button);
                    break;
            }
        }
        if (onIdle)
            onIdle();
        if (requiresExpose) {
            onExpose();
            SDL_GL_SwapWindow(_handle->hwnd);
            requiresExpose = false;
        }
    }
}

void SdlWindow::getWindowSize(int& w, int& h) {
    if (_handle)
        SDL_GetWindowSize(_handle->hwnd, &w, &h);
}

void SdlWindow::getDpi(int& w, int& h) {
    if (_handle) {
        int disp = SDL_GetWindowDisplayIndex(_handle->hwnd);
        float f_w, f_h;
        SDL_GetDisplayDPI(disp, NULL, &f_w, &f_h);
        w = f_w;
        h = f_h;
    }
}

void SdlWindow::setWindowTitle(std::string& title) {
    setWindowTitle(title.c_str());
}

void SdlWindow::setWindowTitle(const char * title) {
    if (_handle)
        SDL_SetWindowTitle(_handle->hwnd, title);
}

void SdlWindow::setWindowSize(int w, int h) {
    if (_handle)
        SDL_SetWindowSize(_handle->hwnd, w, h);
}

void SdlWindow::setWindowPos(int x, int y) {
    if (_handle)
        SDL_SetWindowPosition(_handle->hwnd, x, y);
}

void SdlWindow::signalKeyDown(SDL_Keycode k, SDL_Keymod m) {
    SDL_Event event;
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = k;
    event.key.keysym.mod = m;
    SDL_PushEvent(&event);
}
/*
void SdlWindow::signalExpose() {
   SDL_Event event;
   event.type = SDL_WINDOWEVENT;
   event.window.event = SDL_WINDOWEVENT_EXPOSED;
   SDL_PushEvent(&event);
}
*/

