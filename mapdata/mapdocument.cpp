#include "mapdocument.h"

static int mapObjectCounter = 0;

MapId::MapId(const char* c)
    : category(c), index(mapObjectCounter++)
{ }

MapId::MapId(const MapId& id)
    : category(id.category), index(id.index)
{ }

MapId::~MapId()
{ }

bool MapId::operator < (const MapId& id) const
{
    if (this->category < id.category) return true;
    else if (this->category == id.category && this->index < id.index) return true;
    return false;
}

bool MapId::operator == (const MapId& id) const
{
    return ((*this) < id == id < (*this));
}

bool MapId::operator != (const MapId& id) const
{
    return !((*this) == id);
}

MapId& MapId::operator = (const MapId& id)
{
    this->category = id.category;
    this->index = id.index;

    return *this;
}

std::ostream& operator << ( std::ostream& os, MapId const& value )
{
    os <<  value.category << ":" << value.index;
    return os;
}

std::ostream& operator << (std::ostream& os, glm::vec2 const& v)
{
    os << "[" << v[0] << ", " << v[1] << "]";
    return os;
}

std::ostream& operator << (std::ostream& os, glm::vec3 const& v)
{
    os << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
    return os;
}

std::ostream& operator << (std::ostream& os, glm::vec4 const& v)
{
    os << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
    return os;
}

MapObject::MapObject(const MapId& id)
    : id(id)
{ }

MapObject::~MapObject()
{ }

const MapId& MapObject::Id() const
{
    return this->id;
}

MapBrushFace::MapBrushFace()
    : MapObject(MapId("BRUSHFACE"))
{ }

float MapBrushFace::PlaneDistance() const
{
    return this->distance;
}

const glm::vec3& MapBrushFace::PlaneNormal() const
{
    return this->normal;
}

const std::string& MapBrushFace::TextureName() const
{
    return this->texture;
}

void MapBrushFace::SetPlane(float distance, const glm::vec3& normal)
{
    this->distance = distance;
    this->normal = normal;
}

void MapBrushFace::SetPlane(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
{
    glm::vec3 normal = glm::normalize(glm::cross(v1-v2, v3-v2));

    this->distance = glm::dot(v2, normal);
    this->normal = normal;
}

void MapBrushFace::SetTexture(const std::string& texture)
{
    this->texture = texture;
}

void MapBrushFace::SetTextureDefinitions_v100(float x_off, float y_off, float rotate, float x_scale, float y_scale)
{
    this->material_v100.x_off = x_off;
    this->material_v100.y_off = y_off;
    this->material_v100.rotate = rotate;
    this->material_v100.x_scale = x_scale;
    this->material_v100.y_scale = y_scale;
}

void MapBrushFace::SetTextureDefinitions_v220(const glm::vec3& uaxis, float ushift, const glm::vec3& vaxis, float vshift, float rotate, float xscale, float yscale)
{
    this->material_v220.uaxis = uaxis;
    this->material_v220.ushift = ushift;
    this->material_v220.vaxis = vaxis;
    this->material_v220.vshift = vshift;
    this->material_v220.rotate = rotate;
    this->material_v220.xscale = xscale;
    this->material_v220.yscale = yscale;
}

MapBrush::MapBrush()
    : MapObject(MapId("BRUSH"))
{ }

MapFaceList MapBrush::Faces()
{
    return this->_brushFaces;
}

MapBrushFace* MapBrush::AddFace()
{
    auto f = new MapBrushFace();

    this->_brushFaces.insert(std::make_pair(f->Id(), f));

    return f;
}

MapEntity::MapEntity()
    : MapObject(MapId("ENTITY"))
{ }

const std::string& MapEntity::ClassName() const
{
    return this->_className;
}

const std::string& MapEntity::Value(const std::string& key) const
{
    return this->_keyValues.at(key);
}

MapBrushList& MapEntity::Brushes()
{
    return this->_brushes;
}

MapBrush* MapEntity::AddBrush()
{
    auto b = new MapBrush();

    this->_brushes.insert(std::make_pair(b->Id(), b));

    return b;
}

void MapEntity::SetValue(const std::string& key, const std::string& value)
{
    if (key == "classname")
        this->_className = value;
    else
        this->_keyValues.insert(std::make_pair(key, value));
}

MapDocument::MapDocument()
{ }

MapDocument::~MapDocument()
{ }

MapEntityList MapDocument::Entities()
{
    return this->_entities;
}

TextureList MapDocument::Textures()
{
    return this->_textures;
}

MapEntity* MapDocument::AddEntity()
{
    auto e = new MapEntity();

    this->_entities.insert(std::make_pair(e->Id(), e));

    return e;
}

Texture* MapDocument::AddTexture(const std::string& texture)
{
    Texture* t = new Texture();

    t->SetName(texture);
    this->_textures.insert(std::make_pair(texture, t));

    return t;
}

MapEntity* MapDocument::FindEntityByClassname(const std::string& classname)
{
    for (auto i = this->_entities.begin(); i != this->_entities.end(); ++i)
    {
        MapEntity* e = i->second;
        if (e->ClassName() == classname)
            return e;
    }

    return nullptr;
}
