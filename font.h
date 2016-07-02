#ifndef FONT_H
#define FONT_H

#include <stdio.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glextl.h>

#include "stb_truetype.h"
#include "shader.h"

typedef struct {
    float vertex[3];
    float texcoord[2];

} Vertex;

class Font {
    unsigned int startchar;
    unsigned int endchar;
    stbtt_bakedchar *cdata;

  public:
     Font();
     virtual ~ Font();

    bool Init(const std::string & fontname, float size, int startchar = 0, int endchar = 128);
    void PrintText(const glm::mat4 & ortho, const std::string & text, const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    glm::vec4 TextBounds(const std::string& text);

    float fontsize;
    GLuint tex;

    static bool SetupShader();

    static GLuint program;
    static GLuint a_vertex;
    static GLuint a_texcoord;
    static GLuint u_projection;
    static GLuint u_tex;
    static GLuint u_global_color;
};

#ifdef FONT_H_IMPL

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const std::string fontVertexShader("#version 100\n"
                   "precision mediump float;\n"
                   "attribute vec3 a_vertex;"
                   "attribute vec2 a_texcoord;"
                   "uniform mat4 u_projection;"
                   "varying vec2 v_texcoord;"
                   "void main()"
                   "{"
                   "    gl_Position = u_projection * vec4(a_vertex.xyz, 1.0);"
                   "    v_texcoord = a_texcoord.st;"
                   "}");

const std::string fontFragmentShader("#version 100\n"
                     "precision mediump float;\n"
                     "uniform sampler2D u_tex;"
                     "uniform vec4 u_global_color;"
                     "varying vec2 v_texcoord;"
                     "void main()"
                     "{"
                     "   vec4 t = texture2D(u_tex, v_texcoord.st);"
                     "   gl_FragColor = vec4(u_global_color.xyz, t.a);"
                     "}");

bool Font::SetupShader()
{
    if (Font::program == 0)
    {
        Font::program = shader_LoadProgram(fontVertexShader, fontFragmentShader);
        glUseProgram(Font::program);

        Font::a_vertex = glGetAttribLocation(Font::program, "a_vertex");
        Font::a_texcoord = glGetAttribLocation(Font::program, "a_texcoord");
        Font::u_projection = glGetUniformLocation(Font::program, "u_projection");
        Font::u_global_color = glGetUniformLocation(Font::program, "u_global_color");

        return true;
    }

    return false;
}

GLuint Font::program = 0;
GLuint Font::a_vertex = 0;
GLuint Font::a_texcoord = 0;
GLuint Font::u_projection = 0;
GLuint Font::u_tex = 0;
GLuint Font::u_global_color = 0;

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define imagesize 1024

Font::Font():tex(0), cdata(nullptr)
{
}

Font::~Font()
{
    if (this->cdata != nullptr)
    {
        delete[]this->cdata;
        this->cdata = nullptr;
    }
}

bool Font::Init(const std::string & fontname, float size, int startchar, int endchar)
{
    Font::SetupShader();

    this->fontsize = size;
    this->startchar = startchar;
    this->endchar = endchar;

    FILE *file = fopen(fontname.c_str(), "rb");
    if (file == NULL)
    {
        Log("failed loading font\n");
        return false;
    }

    fseek(file, 0, SEEK_END);
    auto filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    auto buffer = new unsigned char[filesize + 1];
    fread(buffer, 1, filesize, file);
    fclose(file);

    this->cdata = new stbtt_bakedchar[this->endchar - this->startchar];
    auto bitmap = new unsigned char[imagesize * imagesize];
    stbtt_BakeFontBitmap(buffer, 0, this->fontsize, bitmap, imagesize, imagesize, this->startchar, this->endchar - this->startchar, this->cdata);
    delete[]buffer;

    /* change monochrome bitmap into rgb and write as bmp file
    auto b = new unsigned char[imagesize * imagesize * 3];
    for (int i = 0; i < imagesize * imagesize; i++)
    {
        b[i * 3] = b[i * 3 + 1] = b[i * 3 + 2] = bitmap[i];
    }
    stbi_write_bmp((fontname + std::string(".bmp")).c_str(), imagesize, imagesize, 3, b);
    delete[]b;
    // */

    glGenTextures(1, &(this->tex));
    glBindTexture(GL_TEXTURE_2D, this->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, imagesize, imagesize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
    delete[]bitmap;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

void Font::PrintText(const glm::mat4 & ortho, const std::string & text2print, const glm::vec4& color)
{
    std::vector < Vertex > res;
    float x = 0, y = 0;
    const char *text = text2print.c_str();
    while (*text)
    {
        if (*text >= 0 && *text < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(this->cdata, imagesize, imagesize, *text, &x, &y, &q, 1);
            Vertex a[] = {
            {q.x0, q.y0, 0.0f, q.s0, q.t0}
            , {q.x1, q.y0, 0.0f, q.s1, q.t0}
            , {q.x1, q.y1, 0.0f, q.s1, q.t1}
            , {q.x0, q.y1, 0.0f, q.s0, q.t1}
            };

            res.push_back(a[0]);
            res.push_back(a[1]);
            res.push_back(a[2]);

            res.push_back(a[0]);
            res.push_back(a[2]);
            res.push_back(a[3]);
        }
        ++text;
    }

    glUseProgram(Font::program);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, this->tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniformMatrix4fv(Font::u_projection, 1, false, glm::value_ptr(ortho));
    glUniform4fv(Font::u_global_color, 1, glm::value_ptr(color));

    glVertexAttribPointer(Font::a_vertex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) & (res[0].vertex[0]));
    glEnableVertexAttribArray(Font::a_vertex);

    glVertexAttribPointer(Font::a_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) & (res[0].texcoord[0]));
    glEnableVertexAttribArray(Font::a_texcoord);

    Font::u_tex = glGetUniformLocation(Font::program, "u_tex");
    glUniform1i(Font::u_tex, 0);

    glDrawArrays(GL_TRIANGLES, 0, res.size());
}

glm::vec4 Font::TextBounds(const std::string& text2print)
{
    float x = 0, y = 0;
    float minx = 99999, maxx = -99999, miny = 99999, maxy = -99999;

    const char *text = text2print.c_str();
    while (*text)
    {
        if (*text >= 0 && *text < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(this->cdata, imagesize, imagesize, *text, &x, &y, &q, 1);
            if (q.x0 < minx) minx = q.x0;
            if (q.x0 > maxx) maxx = q.x0;

            if (q.y0 < miny) miny = q.y0;
            if (q.y0 > maxy) maxy = q.y0;

            if (q.x1 < minx) minx = q.x1;
            if (q.x1 > maxx) maxx = q.x1;

            if (q.y1 < miny) miny = q.y1;
            if (q.y1 > maxy) maxy = q.y1;
        }
        ++text;
    }

    return glm::vec4(minx, miny, maxx, maxy);
}

#endif				// FONT_H_IMPL

#endif				// FONT_H
