#pragma once
 
#include "vhl_device.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace vhl {
 
    class VhlDescriptorSetLayout 
    {
    public:
        class Builder 
        {
        public:
            Builder(VhlDevice& vhlDevice) : m_VhlDevice{vhlDevice} {}
        
            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<VhlDescriptorSetLayout> build() const;
        
        private:
            VhlDevice& m_VhlDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings{};
        };
    
        VhlDescriptorSetLayout(
            VhlDevice& vhlDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VhlDescriptorSetLayout();
        VhlDescriptorSetLayout(const VhlDescriptorSetLayout &) = delete;
        VhlDescriptorSetLayout &operator=(const VhlDescriptorSetLayout &) = delete;
        
        VkDescriptorSetLayout getDescriptorSetLayout() const { return m_DescriptorSetLayout; }
        
    private:
        VhlDevice& m_VhlDevice;
        VkDescriptorSetLayout m_DescriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;
        
        friend class VhlDescriptorWriter;
    };
    
    class VhlDescriptorPool 
    {
    public:
        class Builder 
        {
        public:
            Builder(VhlDevice& vhlDevice) : m_VhlDevice{vhlDevice} {}
        
            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VhlDescriptorPool> build() const;
        
        private:
            VhlDevice& m_VhlDevice;
            std::vector<VkDescriptorPoolSize> m_PoolSizes{};
            uint32_t m_MaxSets = 1000;
            VkDescriptorPoolCreateFlags m_PoolFlags = 0;
        };
        
        VhlDescriptorPool(
            VhlDevice& vhlDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~VhlDescriptorPool();
        VhlDescriptorPool(const VhlDescriptorPool&) = delete;
        VhlDescriptorPool &operator=(const VhlDescriptorPool&) = delete;
        
        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
        
        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
        
        void resetPool();
        
    private:
        VhlDevice& m_VhlDevice;
        VkDescriptorPool m_DescriptorPool;
        
        friend class VhlDescriptorWriter;
    };
    
    class VhlDescriptorWriter 
    {
    public:
        VhlDescriptorWriter(VhlDescriptorSetLayout& setLayout, VhlDescriptorPool& pool);
        
        VhlDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VhlDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
        
        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);
        
    private:
        VhlDescriptorSetLayout& m_SetLayout;
        VhlDescriptorPool& m_Pool;
        std::vector<VkWriteDescriptorSet> m_Writes;
    };
 
}  // namespace vhl