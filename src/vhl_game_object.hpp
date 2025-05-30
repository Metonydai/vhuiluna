#pragma once

#include "vhl_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace vhl {

    struct TransformComponent 
    {
        glm::vec3 translation{};  // (position offset)
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};

        // Matrix corresponds to translate * Ry * Rx * Rz * scale transformation
        // Rotation conversion uses tait-bryan angles with axis order Y(1), X(2), Z(3)
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    struct PointLightComponent
    {
        float lightIntensity = 1.0f;
    };

    class VhlGameObject 
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, VhlGameObject>;

        static VhlGameObject createGameObject() 
        {
            static id_t currentId = 0;
            return VhlGameObject{currentId++};
        }

        static VhlGameObject makePointLight(
            float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));
        

        VhlGameObject(const VhlGameObject&) = delete;
        VhlGameObject& operator=(const VhlGameObject&) = delete;
        VhlGameObject(VhlGameObject&&) = default;
        VhlGameObject& operator=(VhlGameObject&&) = default;

        id_t getId() { return id; }

        glm::vec3 color{};
        TransformComponent transform{};

        // Optional pointer components
        std::shared_ptr<VhlModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;
    private:
        VhlGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}  // namespace Vhl