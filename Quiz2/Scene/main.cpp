/**
 * main.cpp – Desktop (GLFW) and WebGL (Emscripten/SDL2) entry point.
 * Android uses NativeTemplate.cpp + JNI instead.
 *
 * Chapter 3 – Vertex Array Object demo:
 *   Renders a rotating coloured cube (VAO + VBO sub-buffer + IBO)
 *   and a spinning ground grid (VAO + procedurally generated lines).
 *   Both objects demonstrate that a VAO captures all attribute pointer
 *   state at init time, so Render() only needs glBindVertexArray().
 *   Touch (or mouse click) pauses / resumes the cube rotation.
 */

#include "Platform.h"
#include "Renderer.h"

// ==========================================================================
// WebGL / Emscripten
// ==========================================================================
#ifdef PLATFORM_EMSCRIPTEN

// --------------------------------------------------------------------------
// GLFW backend (default – build with -DUSE_GLFW -s USE_GLFW=3)
// --------------------------------------------------------------------------
#ifdef USE_GLFW

static GLFWwindow* g_window = nullptr;

static void fbsize_cb(GLFWwindow* /*win*/, int w, int h)
{ Renderer::Instance().resize(w, h); }

static void mouseButtonCB(GLFWwindow* win, int btn, int action, int)
{
    if (btn != GLFW_MOUSE_BUTTON_LEFT) return;
    double mx, my; glfwGetCursorPos(win, &mx, &my);
    if (action == GLFW_PRESS)
        Renderer::Instance().TouchEventDown((float)mx, (float)my);
    else
        Renderer::Instance().TouchEventRelease((float)mx, (float)my);
}

// for cursor movement callback
static void cursorPosCB(GLFWwindow* win, double mx, double my)
{
    // only when the left mouse btn is held down
    if(glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        Renderer::Instance().TouchEventMove((float)mx, (float)my);
    }
}

static void main_loop()
{
    if (glfwWindowShouldClose(g_window)) {
        emscripten_cancel_main_loop();
        glfwDestroyWindow(g_window);
        glfwTerminate();
        return;
    }
    glfwPollEvents();
    Renderer::Instance().render();
    glfwSwapBuffers(g_window);
}

int main()
{
    LOGI("Vertex Array Object - WebGL / Emscripten (GLFW)");
    if (!glfwInit()) { LOGE("glfwInit failed"); return -1; }
    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    g_window = glfwCreateWindow(800, 600, "Vertex Array Object", nullptr, nullptr);
    if (!g_window) { LOGE("glfwCreateWindow failed"); glfwTerminate(); return -1; }
    glfwMakeContextCurrent(g_window);
    glfwSetFramebufferSizeCallback(g_window, fbsize_cb);
    glfwSetMouseButtonCallback(g_window,     mouseButtonCB);
    glfwSetCursorPosCallback(g_window, cursorPosCB);

    Renderer::Instance().initializeRenderer();
    int w, h; glfwGetFramebufferSize(g_window, &w, &h);
    Renderer::Instance().resize(w, h);
    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}

// --------------------------------------------------------------------------
// SDL2 backend (opt-in – build without -DUSE_GLFW, use -s USE_SDL=2)
// --------------------------------------------------------------------------
#else // !USE_GLFW

static SDL_Window*   g_window = nullptr;
static SDL_GLContext g_glctx  = nullptr;

static void main_loop()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            emscripten_cancel_main_loop(); break;
        case SDL_MOUSEBUTTONDOWN:
            Renderer::Instance().TouchEventDown(
                (float)ev.button.x, (float)ev.button.y); break;
        case SDL_MOUSEBUTTONUP:
            Renderer::Instance().TouchEventRelease(
                (float)ev.button.x, (float)ev.button.y); break;
        case SDL_MOUSEMOTION: // for the swiping action
            if(ev.motion.state && SDL_BUTTON_LMASK){ // only check when left mouse is held
                Renderer::Instance().TouchEventMove(
                    (float)ev.button.x, (float)ev.button.y);
                }
            break;
        default: break;
        }
    }
    Renderer::Instance().render();
    SDL_GL_SwapWindow(g_window);
}

int main()
{
    LOGI("Vertex Array Object - WebGL / Emscripten (SDL2)");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { LOGE("SDL_Init: %s", SDL_GetError()); return -1; }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    g_window = SDL_CreateWindow("Vertex Array Object",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!g_window) { LOGE("SDL_CreateWindow: %s", SDL_GetError()); return -1; }
    g_glctx = SDL_GL_CreateContext(g_window);
    if (!g_glctx) { LOGE("SDL_GL_CreateContext: %s", SDL_GetError()); return -1; }

    Renderer::Instance().initializeRenderer();
    Renderer::Instance().resize(800, 600);
    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}

#endif // USE_GLFW

// ==========================================================================
// Desktop – Windows / GLFW
// ==========================================================================
#elif defined(PLATFORM_WINDOWS)

#include <iostream>

static void framebufferSizeCB(GLFWwindow*, int w, int h)
{ Renderer::Instance().resize(w, h); }

static void mouseButtonCB(GLFWwindow* win, int btn, int action, int)
{
    if (btn != GLFW_MOUSE_BUTTON_LEFT) return;
    double mx, my; glfwGetCursorPos(win, &mx, &my);
    if (action == GLFW_PRESS){
        Renderer::Instance().TouchEventDown((float)mx, (float)my);
    }
    else{
        Renderer::Instance().TouchEventRelease((float)mx, (float)my);
    }
        
}

// for cursor movement callback
static void cursorPosCB(GLFWwindow* win, double mx, double my)
{
    // only when the left mouse btn is held down
    if(glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        Renderer::Instance().TouchEventMove((float)mx, (float)my);
    }
}

int main()
{
    std::cout << "Vertex Array Object - Desktop (GLFW)\n"
              << "Spinning cube (VAO) + rotating grid (VAO). Click to pause cube. ESC to quit.\n";

    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return -1; }
    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Vertex Array Object", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "glewInit failed\n"; return -1; }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCB);
    glfwSetMouseButtonCallback(window, mouseButtonCB);
    glfwSetCursorPosCallback(window, cursorPosCB);

    Renderer::Instance().initializeRenderer();
    int w, h; glfwGetFramebufferSize(window, &w, &h);
    Renderer::Instance().resize(w, h);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        Renderer::Instance().render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
#endif
