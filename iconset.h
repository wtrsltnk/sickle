#ifndef ICONSET_H
#define ICONSET_H

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glextl.h>

#include "shader.h"
#include "font.h"

struct Icon {
    int w, h;
    GLuint tex;
};

class Iconset {
    stbtt_fontinfo font;
    stbtt_bakedchar *cdata;
    static std::map<int, Icon> loadedIcons;
    Icon InitIcon(int c);

public:
    Iconset();
    virtual ~ Iconset();

    bool Init(const std::string & fontname, float size);
    void PrintIcon(const glm::mat4& ortho, int c, const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

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

#ifdef ICONSET_H_IMPL

#define imagesize 1024

Iconset::Iconset():tex(0), cdata(nullptr)
{ }

Iconset::~Iconset()
{
    if (this->cdata != nullptr)
    {
        delete[]this->cdata;
        this->cdata = nullptr;
    }
}

std::map<int, Icon> Iconset::loadedIcons;

bool Iconset::Init(const std::string & fontname, float size)
{
    Font::SetupShader();

    this->fontsize = size;

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

    stbtt_InitFont(&this->font, buffer, stbtt_GetFontOffsetForIndex(buffer,0));

    return true;
}

Icon Iconset::InitIcon(int c)
{
    auto found = Iconset::loadedIcons.find(c);
    if (found != Iconset::loadedIcons.end())
    {
        return found->second;
    }

    Icon res;
    int i,j,s = 20;
    auto h = stbtt_ScaleForPixelHeight(&this->font, this->fontsize);
    auto bitmap = stbtt_GetCodepointBitmap(&font, 0, h, c, &(res.w), &(res.h), 0,0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);          // UNPACK_ALIGNMENT, lightmaps are byte per pixel
    glGenTextures(1, &(res.tex));
    glBindTexture(GL_TEXTURE_2D, res.tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, res.w, res.h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
    stbtt_FreeBitmap(bitmap, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);          // Reset UNPACK_ALIGNMENT

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //* change monochrome bitmap into rgb and write as bmp file
    auto b = new unsigned char[res.w * res.h * 3];
    for (int i = 0; i < res.w * res.h; i++)
    {
        b[i * 3] = b[i * 3 + 1] = b[i * 3 + 2] = bitmap[i];
    }
    stbi_write_bmp(std::string("icon.bmp").c_str(), res.w, res.h, 3, b);
    delete[]b;
    // */

    Iconset::loadedIcons.insert(std::make_pair(c, res));

    return res;
}

void Iconset::PrintIcon(const glm::mat4& ortho, int c, const glm::vec4& color)
{
    Icon icon = InitIcon(c);

    std::vector < Vertex > res;
    Vertex a[] = {
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        { float(icon.w), 0.0f, 0.0f, 1.0f, 0.0f},
        { float(icon.w), float(icon.h), 0.0f, 1.0f, 1.0f},
        { 0.0f, float(icon.h), 0.0f, 0.0f, 1.0f}
    };

    res.push_back(a[0]);
    res.push_back(a[1]);
    res.push_back(a[2]);

    res.push_back(a[0]);
    res.push_back(a[2]);
    res.push_back(a[3]);

    glUseProgram(Font::program);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, icon.tex);

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

#endif				// ICONSET_H_IMPL

#endif				// ICONSET_H
