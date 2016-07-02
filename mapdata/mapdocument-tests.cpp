
#include <sstream>
#include <glm/glm.hpp>

namespace Catch {
    std::string toString( glm::vec3 const& value ) {
        std::stringstream ss;
        ss << "[" << value.x << "," << value.y << "," << value.z << "]";
        return ss.str();
    }
}

#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "mapdocument.h"

TEST_CASE("Map Id")
{
    MapId brushid("BRUSH");

    SECTION("two id's from the same type are not equal")
    {
        MapId secondid("BRUSH");

        REQUIRE(brushid != secondid);
    }

    SECTION("two id's from different types are not equal")
    {
        MapId entityid("ENTITY");

        REQUIRE(brushid != entityid);
    }

    SECTION("a map id can be constructor copied")
    {
        MapId copiedid = brushid;

        REQUIRE(brushid == copiedid);
    }

    SECTION("a map id can be assigned")
    {
        MapId assignedid("ASSIGNED");

        assignedid = brushid;

        REQUIRE(brushid == assignedid);
    }
}

TEST_CASE("Map Document")
{
    auto doc = new MapDocument();

    SECTION("a new map document has zero Entities()")
    {
        REQUIRE(doc->Entities().size() == 0);
    }

    SECTION("adding an entity")
    {
        auto entity = doc->AddEntity();

        REQUIRE(entity != nullptr);
        REQUIRE(doc->Entities().size() == 1);

        SECTION("a new map entity has no Brushes()")
        {
            REQUIRE(entity->Brushes().size() == 0);
        }

        SECTION("an empty entity has no ClassName()")
        {
            REQUIRE(entity->ClassName() == "");
        }

        SECTION("setting the \"classname\" value changes the ClassName()")
        {
            entity->SetValue("classname", "Entity");
            REQUIRE(entity->ClassName() == "Entity");
        }

        SECTION("adding a brush")
        {
            auto brush = entity->AddBrush();

            REQUIRE(brush != nullptr);
            REQUIRE(entity->Brushes().size() == 1);

            SECTION("a new map brush has no BrusheFaces()")
            {
                REQUIRE(brush->Faces().size() == 0);
            }

            SECTION("adding a face")
            {
                auto face = brush->AddFace();

                REQUIRE(face != nullptr);
                REQUIRE(brush->Faces().size() == 1);

                SECTION("a new face has an empty TextureName()")
                {
                    REQUIRE(face->TextureName() == "");
                }

                SECTION("setting the plane")
                {
                    face->SetPlane(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    REQUIRE(face->PlaneNormal() == glm::vec3(0.0f, 0.0f, -1.0f));
                }

            }
        }
    }
}
