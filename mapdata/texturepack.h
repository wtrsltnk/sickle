#ifndef _TEXTUREPACK_H_
#define _TEXTUREPACK_H_

#include "hl1types.h"

#include <string>
#include <vector>
#include <fstream>

class TexturePack
{
    std::ifstream _file;
    HL1::tWADHeader _header;
    HL1::tWADLump* _lumps;
    Array<byte*> _loadedLumps;

public:
    TexturePack(const std::string& filename);
    virtual ~TexturePack();

    bool IsLoaded() const;
    int IndexOf(const std::string& name) const;
    bool LoadTexture(int index, Texture* texture);

    static std::string FindWad(const std::string& wad, const std::vector<std::string>& hints);
    static std::vector<TexturePack*> LoadPacks(const std::string& wads, const std::string& bspLocation);
    static void UnloadPacks(std::vector<TexturePack*>& wads);

};

#endif // _TEXTUREPACK_H_
