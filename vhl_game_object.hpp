#pragma once

#include "vhl_model.hpp"

// std
#include <memory>

namespace vhl {

    struct Transform2dComponent 
    {
        glm::vec2 translation{};  // (position offset)
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2() {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotMatrix{{c, s}, {-s, c}};

            glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
            return rotMatrix * scaleMat;
        }
    };

    class VhlGameObject {
    public:
        using id_t = unsigned int;

        static VhlGameObject createGameObject() 
        {
            static id_t currentId = 0;
            return VhlGameObject{currentId++};
        }

        VhlGameObject(const VhlGameObject&) = delete;
        VhlGameObject& operator=(const VhlGameObject&) = delete;
        VhlGameObject(VhlGameObject&&) = default;
        VhlGameObject& operator=(VhlGameObject&&) = default;

        id_t getId() { return id; }

        std::shared_ptr<VhlModel> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d{};

    private:
        VhlGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}  // namespace Vhl