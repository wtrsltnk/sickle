#include "maploader.h"

#include <iostream>

MapLoader::MapLoader(DataFileLocator& locator, DataFileLoader& loader)
    : _locator(locator), _loader(loader)
{ }

bool MapLoader::Load(const std::string& filename, MapDocument* document)
{
    if (document == nullptr) return false;

    Array<byte>& data = this->_loader(filename);

    if (data.count == 0) return false;

    Tokenizer tok(reinterpret_cast<char*>(data.data), data.count);

    while (tok.nextToken())
    {
        if (strcmp(tok.getToken(), "{") == 0)
        {
            if (this->LoadEntity(tok, document) == false)
            {
                return false;
            }
        }
    }

    std::vector<TexturePack*> wads;
    MapEntity* worldspawn = document->FindEntityByClassname("worldspawn");
    if (worldspawn != nullptr)
        wads = TexturePack::LoadPacks(worldspawn->Value("wad"), filename);
    this->LoadTextures(wads, document);
    TexturePack::UnloadPacks(wads);

    return true;
}

bool MapLoader::LoadEntity(Tokenizer& tok, MapDocument* document)
{
    auto e = document->AddEntity();

    while (tok.nextToken() && strcmp(tok.getToken(), "}") != 0)
    {
        if (strcmp(tok.getToken(), "{") == 0)
        {
            if (this->LoadBrush(tok, e) == false)
                return false;
        }
        else
        {
            std::string key(tok.getToken());
            tok.nextToken();
            std::string value(tok.getToken());

            if (key == "mapversion")
                this->_mapVersion = atoi(value.c_str());
            else
                e->SetValue(key, value);
        }
    }

    return strcmp(tok.getToken(), "}") == 0;
}

bool MapLoader::LoadBrush(Tokenizer& tok, MapEntity* entity)
{
    auto b = entity->AddBrush();

    while (tok.nextToken() && strcmp(tok.getToken(), "}") != 0)
    {
        glm::vec3 v1, v2, v3;
        if (tok.nextToken() == false) return false;	// Skip the "("
        v1[0] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        v1[1] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        v1[2] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        if (tok.nextToken() == false) return false;	// Skip the ")"

        if (tok.nextToken() == false) return false;	// Skip the "("
        v2[0] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        v2[1] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        v2[2] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        if (tok.nextToken() == false) return false;	// Skip the ")"

        if (tok.nextToken() == false) return false;	// Skip the "("
        v3[0] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        v3[1] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        v3[2] = float(atoi(tok.getToken())); if (tok.nextToken() == false) return false;
        if (tok.nextToken() == false) return false;	// Skip the ")"

        auto face = b->AddFace();
        face->SetPlane(v1, v2, v3);
        face->SetTexture(tok.getToken()); if (tok.nextToken() == false) return false;	// Texture name

        if (this->_mapVersion == 100)      // Default Quake version
        {
            float x_off = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	// x_off     - Texture x-offset (must be multiple of 16)
            float y_off = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	// y_off     - Texture y-offset (must be multiple of 16)
            float rot_angle = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	// rot_angle - floating point value indicating texture rotation
            float x_scale = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	// x_scale   - scales x-dimension of texture (negative value to flip)
            float y_scale = float(atof(tok.getToken()));
            face->SetTextureDefinitions_v100(x_off, y_off, rot_angle, x_scale, y_scale);
        }
        else if (this->_mapVersion == 220) // Valve Hammer Version
        {
            if (tok.nextToken() == false) return false;	// Skip the "["
            glm::vec3 uaxis;
            uaxis[0] = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            uaxis[1] = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            uaxis[2] = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            float ushift = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            if (tok.nextToken() == false) return false;	// Skip the "]"
            if (tok.nextToken() == false) return false;	// Skip the "["
            glm::vec3 vaxis;
            vaxis[0] = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            vaxis[1] = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            vaxis[2] = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            float vshift = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            if (tok.nextToken() == false) return false;	// Skip the "]"
            float rotate = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            float xscale = float(atof(tok.getToken())); if (tok.nextToken() == false) return false;	//
            float yscale = float(atof(tok.getToken()));
            face->SetTextureDefinitions_v220(uaxis, ushift, vaxis, vshift, rotate, xscale, yscale);
        }
        this->_textureNames.insert(face->TextureName());
    }

    return strcmp(tok.getToken(), "}") == 0;
}

bool MapLoader::LoadTextures(const std::vector<TexturePack*>& packs, MapDocument* document)
{
    for (auto n = this->_textureNames.begin(); n != this->_textureNames.end(); ++n)
    {
        auto texture = document->AddTexture(*n);

        bool found = false;
        for (auto i = packs.cbegin(); i != packs.cend(); ++i)
        {
            TexturePack* pack = *i;
            if (pack ->LoadTexture(pack ->IndexOf(*n), texture))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::cout << "Texture \"" << (*n) << "\" not found" << std::endl;
            texture->DefaultTexture();
        }
    }

    return true;
}
