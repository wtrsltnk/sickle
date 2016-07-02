#ifndef MAPLOADER_H
#define MAPLOADER_H

#include "mapdocument.h"
#include "hl1types.h"
#include "tokenizer.h"

class MapLoader
{
    std::set<std::string> _textureNames;
    DataFileLocator& _locator;
    DataFileLoader& _loader;
    int _mapVersion;
    bool LoadEntity(Tokenizer& tok, MapDocument* document);
    bool LoadBrush(Tokenizer& tok, MapEntity* entity);
    static MapBrushFace* CreateFace(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
    bool LoadTextures(const std::vector<TexturePack*>& wads, MapDocument* document);

public:
    MapLoader(DataFileLocator& locator, DataFileLoader& loader);

    bool Load(const std::string& filename, MapDocument* document);

};

#endif // MAPLOADER_H
