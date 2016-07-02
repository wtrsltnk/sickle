#include "program.h"
#include <iostream>
#include <GL/glextl.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define LOG_H_IMPL
#include "log.h"

#define SHADER_H_IMPL
#include "shader.h"

#define FONT_H_IMPL
#include "font.h"

#define ICONSET_H_IMPL
#include "iconset.h"

static bool run = true;
Font monaco;
Iconset materialicons;

std::string vshader = std::string("attribute vec4 a_vertex;"
                                  "uniform vec4 u_color;"
                                  "uniform mat4 u_proj;"
                                  "uniform mat4 u_pos;"
                                  "uniform mat4 u_size;"
                                  "varying vec4 v_color;"
                                  "void main()"
                                  "{"
                                  "    v_color = u_color;"
                                  "    gl_Position = u_proj * u_pos * u_size * a_vertex; }");

std::string fshader = std::string("precision mediump float;"
                                  "varying vec4 v_color;"
                                  "void main()"
                                  "{ gl_FragColor = v_color; }");

static GLuint iProgram = 0;
static GLint a_vertex = 0;
static GLint u_color = 0;
static GLint u_proj = 0;
static GLint u_pos = 0;
static GLint u_size = 0;

static GLfloat vertices[] = {
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
};

namespace im
{
#define MOUSEBUTTON01 0x0001
#define MOUSEBUTTON02 0x0002
#define MOUSEBUTTON03 0x0004

    struct sUIID {
        long long name;
        int index;
        sUIID(const char* a, int b = 0){ name = (long long)a; index = b; }
    };

    bool operator < (const struct sUIID a, const struct sUIID b)
    {
        if (a.name < b.name) return true;
        else if (a.name == b.name && a.index < b.index) return true;
        return false;
    }

    bool operator == (const struct sUIID a, const struct sUIID b)
    {
        return (a < b == b < a);
    }

    struct sUIstate {
        int windowx;
        int windowy;

        int mousex;
        int mousey;
        int mousebuttonsdown;

        sUIID hot;
        sUIID active;
    };

    static sUIstate uistate =  { 0, 0, 0, 0, 0, sUIID(0), sUIID(0) };

    static std::string emptyString;

    class Control
    {
    protected:
        Control(sUIID id) : id(id), text(emptyString) { }
        Control(const char* a, int b = 0) : id(sUIID(a, b)), rect(glm::vec4(0.0f, 0.0f, 100.0f, 25.0f)), text(emptyString) { }
        virtual ~Control() { }

    public:
        sUIID id;
        glm::vec4 rect;
        std::string& text;

        virtual void Render() const = 0;

        Control& Position(int x, int y)
        {
            rect.x = x;
            rect.y = y;

            return *this;
        }

        Control& Text(const std::string& text)
        {
            this->text = text;
            return *this;
        }
    };

    class Button : public Control
    {
    public:
        Button(sUIID id) : Control(id) { }
        Button(const char* a, int b = 0) : Control(a, b) { }
        virtual ~Button() { }

        virtual void Render() const
        {
            glUseProgram(iProgram);

            glEnableVertexAttribArray(GLuint(a_vertex));
            glVertexAttribPointer(GLuint(a_vertex), 3, GL_FLOAT, GL_FALSE, 0, vertices);

            auto ortho = glm::ortho(0.0f, float (uistate.windowx), float (uistate.windowy), 0.0f, -10.0f, 10.0f);
            auto t = glm::translate(glm::mat4(1.0f), glm::vec3(rect.x, rect.y, 0.0f));
            auto s = glm::scale(glm::mat4(1.0f), glm::vec3(rect.z, rect.w, 0.0f));

            glUniformMatrix4fv(u_proj, 1, false, glm::value_ptr(ortho));
            glUniformMatrix4fv(u_pos, 1, false, glm::value_ptr(t));
            glUniformMatrix4fv(u_size, 1, false, glm::value_ptr(s));

            if (id == uistate.active)
                glUniform4fv(u_color, 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
            else if (id == uistate.hot)
                glUniform4fv(u_color, 1, glm::value_ptr(glm::vec4(1.0f, 0.8f, 0.5f, 1.0f)));
            else
                glUniform4fv(u_color, 1, glm::value_ptr(glm::vec4(1.0f)));

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            auto bounds = monaco.TextBounds(text);
            auto textwidth = bounds.z - bounds.x;
            auto ft = glm::translate(ortho, glm::vec3(rect.x + (rect.z - textwidth)/2.0f, rect.y + monaco.fontsize, 0.0f));
            monaco.PrintText(ft, text, glm::vec4(0.0f));
        }
    };

    bool Click(const Control& control)
    {
        if (uistate.mousex >= control.rect.x && uistate.mousex <= (control.rect.x+control.rect.z)
                && uistate.mousey >= control.rect.y && uistate.mousey <= (control.rect.y+control.rect.w))
        {
            uistate.hot = control.id;
            if (uistate.active == sUIID(0) && uistate.mousebuttonsdown)
                uistate.active = control.id;
        }

        control.Render();

        if (uistate.mousebuttonsdown == 0 && uistate.hot == control.id && uistate.active == control.id)
            return true;

        return false;
    }

    void StartFrame()
    {
        uistate.hot = sUIID(0);
    }

    void EndFrame()
    {
        if (uistate.mousebuttonsdown == 0)
        {
            uistate.active = sUIID(0);
        }
    }

    bool DoButton(sUIID id, const std::string& text, const glm::vec4& rect)
    {
        if (uistate.mousex >= rect.x && uistate.mousex <= (rect.x+rect.z)
                && uistate.mousey >= rect.y && uistate.mousey <= (rect.y+rect.w))
        {
            uistate.hot = id;
            if (uistate.active == sUIID(0) && uistate.mousebuttonsdown)
                uistate.active = id;
        }

        glUseProgram(iProgram);

        glEnableVertexAttribArray(GLuint(a_vertex));
        glVertexAttribPointer(GLuint(a_vertex), 3, GL_FLOAT, GL_FALSE, 0, vertices);

        auto ortho = glm::ortho(0.0f, float(uistate.windowx), float(uistate.windowy), 0.0f, -10.0f, 10.0f);
        auto t = glm::translate(glm::mat4(1.0f), glm::vec3(rect.x, rect.y, 0.0f));
        auto s = glm::scale(glm::mat4(1.0f), glm::vec3(rect.z, rect.w, 0.0f));

        glUniformMatrix4fv(u_proj, 1, false, glm::value_ptr(ortho));
        glUniformMatrix4fv(u_pos, 1, false, glm::value_ptr(t));
        glUniformMatrix4fv(u_size, 1, false, glm::value_ptr(s));

        if (id == uistate.active)
            glUniform4fv(u_color, 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
        else if (id == uistate.hot)
            glUniform4fv(u_color, 1, glm::value_ptr(glm::vec4(1.0f, 0.8f, 0.5f, 1.0f)));
        else
            glUniform4fv(u_color, 1, glm::value_ptr(glm::vec4(1.0f)));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        auto bounds = monaco.TextBounds(text);
        auto textwidth = bounds.z - bounds.x;
        auto ft = glm::translate(ortho, glm::vec3(rect.x + (rect.z - textwidth)/2.0f, rect.y + monaco.fontsize, 0.0f));
        monaco.PrintText(ft, text, glm::vec4(0.0f));

        if (uistate.mousebuttonsdown == 0 && uistate.hot == id && uistate.active == id)
            return true;

        return false;
    }

}

//
// OPENGL/SDL INTERFACE
//
bool glinit()
{
    glClearColor(0.3f, 0.4f, 0.5f, .0f);

    iProgram = shader_LoadProgram(vshader, fshader);
    a_vertex = glGetAttribLocation(iProgram, "a_vertex");
    u_color = glGetUniformLocation(iProgram, "u_color");
    u_proj = glGetUniformLocation(iProgram, "u_proj");
    u_pos = glGetUniformLocation(iProgram, "u_pos");
    u_size = glGetUniformLocation(iProgram, "u_size");

    if (monaco.Init("../sickle/monaco.ttf", 20.0f) == false)
    {
        std::cout << "Failed to load monaco"<< std::endl;
    }

    if (materialicons.Init("../sickle/MaterialIcons-Regular.ttf", 150.0f) == false)
    {
        std::cout << "Failed to load MaterialIcons"<< std::endl;
    }

    return true;
}

void glresize(int width, int height)
{
    glViewport(0, 0, width, height);
}

void glloop()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    im::StartFrame();

    if (im::Click(im::Button("newtest").Text("Bladie bla").Position(25, 25)))
    {
        std::cout << "im::Click @ " << im::uistate.mousex << "," << im::uistate.mousey << std::endl;
    }

    if (im::DoButton(im::sUIID("test"), "Test", glm::vec4(25.0f, 65.0f, 180.0f, 25.0f)))
    {
        std::cout << "clicked test " << im::uistate.mousex << "," << im::uistate.mousey << std::endl;
    }
    if (im::DoButton(im::sUIID("teste"), "Testsdfasd", glm::vec4(25.0f, 100.0f, 180.0f, 25.0f)))
    {
        std::cout << "clicked teste" << im::uistate.mousex << "," << im::uistate.mousey << std::endl;
    }
    im::EndFrame();

    auto ortho = glm::ortho(0.0f, float (im::uistate.windowx), float (im::uistate.windowy), 0.0f, -10.0f, 10.0f);
    auto t = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 0.0f));
    materialicons.PrintIcon(ortho*t, 0xe001);
    t = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 300.0f, 0.0f));
    materialicons.PrintIcon(ortho*t, 0xe070);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        Log("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == NULL)
    {
        Log("Could not create context: %s\n", SDL_GetError());
        return 1;
    }

    int res = SDL_GL_MakeCurrent(window, context);
    if (res != 0)
    {
        Log("Could not make context current: %s\n", SDL_GetError());
        return 1;
    }

    glExtLoadAll((PFNGLGETPROC*)&SDL_GL_GetProcAddress);

    run = glinit();

    SDL_SetWindowSize(window, 1280, 720);
    while (run)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    im::uistate.windowx = (event.window.data1 <= 0 ? 1 : event.window.data1);
                    im::uistate.windowy = (event.window.data2 <= 0 ? 1 : event.window.data2);

                    glresize(im::uistate.windowx, im::uistate.windowy);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    im::uistate.mousebuttonsdown |= MOUSEBUTTON01;
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                {
                    im::uistate.mousebuttonsdown |= MOUSEBUTTON02;
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    im::uistate.mousebuttonsdown |= MOUSEBUTTON03;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    im::uistate.mousebuttonsdown ^= MOUSEBUTTON01;
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                {
                    im::uistate.mousebuttonsdown ^= MOUSEBUTTON02;
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    im::uistate.mousebuttonsdown ^= MOUSEBUTTON03;
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                im::uistate.mousex = event.button.x;
                im::uistate.mousey = event.button.y;
            }
        }
        glloop();
        SDL_GL_SwapWindow(window);

        const Uint8 *state = SDL_GetKeyboardState(nullptr);
        run = !state[SDL_SCANCODE_ESCAPE];
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
