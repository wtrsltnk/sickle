#ifndef _TEXTURE_H_
#define	_TEXTURE_H_

#include <string>
#include <glm/glm.hpp>

class Texture
{
    std::string _name;
    int _width;
    int _height;
    int _bpp;
    int _format;
    bool _repeat;
    unsigned char* _data;
    unsigned int _glIndex;

public:
    Texture();
    virtual ~Texture();

    const std::string& Name() const;
    int Width() const;
    int Height() const;
    int Bpp() const;
    int DataSize() const;
    unsigned char* Data();
    unsigned int GlIndex() const;

#ifdef __GL_H__
    unsigned int UploadToGl();
    void DeleteFromGl();
#endif

    Texture* Copy() const;
    void CopyFrom(const Texture& from);
    void DefaultTexture();

    glm::uvec4 PixelAt(int x, int y) const;
    void SetPixelAt(const glm::uvec4& pixel, int x, int y);

    void Fill(const glm::vec4& color);
    void Fill(const Texture& from);
    // expandBorder is used to puts a 1-pixel border around the copied texture so no neightbour leaking is occuring on an atlas
    void FillAtPosition(const Texture& from, const glm::vec2& pos, bool expandBorder = false);

    void SetData(int w, int h, int bpp, unsigned char* data, bool repeat = true);
    void SetName(const std::string& _name);
    void SetDimentions(int _width, int _height, int _bpp = 3, unsigned int _format = -1);

    void CorrectGamma(float gamma);

};

#endif	// _TEXTURE_H_
