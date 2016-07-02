#ifndef _MAPDOCUMENT_H_
#define _MAPDOCUMENT_H_

#include "hl1types.h"
#include "texturepack.h"
#include "texture.h"

#include <vector>
#include <string>
#include <map>
#include <set>
#include <ostream>

class MapId
{
    friend std::ostream& operator << ( std::ostream& os, MapId const& value );

    const char* category;
    int index;
public:
    MapId(const char* c);
    MapId(const MapId& id);
    virtual ~MapId();

    bool operator < (const MapId& id) const;
    bool operator == (const MapId& id) const;
    bool operator != (const MapId& id) const;
    MapId& operator = (const MapId& id);

};

std::ostream& operator << (std::ostream& os, const MapId& value);
std::ostream& operator << (std::ostream& os, const glm::vec2& v);
std::ostream& operator << (std::ostream& os, const glm::vec3& v);
std::ostream& operator << (std::ostream& os, const glm::vec4& v);

class MapObject
{
    MapId id;
protected:
    MapObject(const MapId& id);

public:
    virtual ~MapObject();

    const MapId& Id() const;
};

class MapBrushFace : public MapObject
{
    float distance;
    glm::vec3 normal;
    std::string texture;

    struct sBrushFaceTextureDefinition_v100
    {
        float x_off;     // Texture x-offset (must be multiple of 16)
        float y_off;     // Texture y-offset (must be multiple of 16)
        float rotate; // floating point value indicating texture rotation
        float x_scale;   // scales x-dimension of texture (negative value to flip)
        float y_scale;   // scales y-dimension of texture (negative value to flip)

    } material_v100;

    struct sBrushFaceTextureDefinition_v220
    {
        glm::vec3 uaxis;
        float ushift;
        glm::vec3 vaxis;
        float vshift;
        float rotate;
        float xscale;
        float yscale;

    } material_v220;

public:
    MapBrushFace();

    float PlaneDistance() const;
    const glm::vec3& PlaneNormal() const;
    const std::string& TextureName() const;

    void SetPlane(float distance, const glm::vec3& normal);
    void SetPlane(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
    void SetTexture(const std::string& texture);
    void SetTextureDefinitions_v100(float x_off, float y_off, float rotate, float x_scale, float y_scale);
    void SetTextureDefinitions_v220(const glm::vec3& uaxis, float ushift, const glm::vec3& vaxis, float vshift, float rotate, float xscale, float yscale);

};

typedef std::map<MapId, MapBrushFace*> MapFaceList;

class MapBrush : public MapObject
{
    MapFaceList _brushFaces;

public:
    MapBrush();

    MapFaceList Faces();

    MapBrushFace* AddFace();

};

typedef std::map<MapId, MapBrush*> MapBrushList;

class MapEntity : public MapObject
{
    std::string _className;
    KeyValueList _keyValues;
    MapBrushList _brushes;

public:
    MapEntity();

    const std::string& ClassName() const;
    const std::string& Value(const std::string& key) const;
    MapBrushList& Brushes();

    MapBrush* AddBrush();
    void SetValue(const std::string& key, const std::string& value);

};

typedef std::map<MapId, MapEntity*> MapEntityList;
typedef std::map<std::string, Texture*> TextureList;

class MapDocument
{
    MapEntityList _entities;
    TextureList _textures;

public:
    MapDocument();
    virtual ~MapDocument();

    MapEntityList Entities();
    TextureList Textures();

    MapEntity* AddEntity();
    Texture* AddTexture(const std::string& texture);

    MapEntity* FindEntityByClassname(const std::string& classname);

};

#endif // _MAPDOCUMENT_H_
