#include "vhl_descriptors.hpp"
 
// std
#include <cassert>
#include <stdexcept>
 
namespace vhl {
 
// *************** Descriptor Set Layout Builder *********************
 
    VhlDescriptorSetLayout::Builder& VhlDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) 
    {
        assert(m_Bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        m_Bindings[binding] = layoutBinding;
        return *this;
    }
    
    std::unique_ptr<VhlDescriptorSetLayout> VhlDescriptorSetLayout::Builder::build() const 
    {
        return std::make_unique<VhlDescriptorSetLayout>(m_VhlDevice, m_Bindings);
    }
    
    // *************** Descriptor Set Layout *********************
    
    VhlDescriptorSetLayout::VhlDescriptorSetLayout(
        VhlDevice& vhlDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : m_VhlDevice{vhlDevice}, m_Bindings{bindings} 
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (const auto& kv : bindings) 
        {
            setLayoutBindings.push_back(kv.second);
        }
    
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
        
        if (vkCreateDescriptorSetLayout(
                m_VhlDevice.device(),
                &descriptorSetLayoutInfo,
                nullptr,
                &m_DescriptorSetLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
    
    VhlDescriptorSetLayout::~VhlDescriptorSetLayout() 
    {
        vkDestroyDescriptorSetLayout(m_VhlDevice.device(), m_DescriptorSetLayout, nullptr);
    }
    
    // *************** Descriptor Pool Builder *********************
    
    VhlDescriptorPool::Builder& VhlDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) 
    {
        m_PoolSizes.push_back({descriptorType, count});
        return *this;
    }
    
    VhlDescriptorPool::Builder& VhlDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) 
    {
        m_PoolFlags = flags;
        return *this;
    }

    VhlDescriptorPool::Builder& VhlDescriptorPool::Builder::setMaxSets(uint32_t count) 
    {
        m_MaxSets = count;
        return *this;
    }
    
    std::unique_ptr<VhlDescriptorPool> VhlDescriptorPool::Builder::build() const 
    {
        return std::make_unique<VhlDescriptorPool>(m_VhlDevice, m_MaxSets, m_PoolFlags, m_PoolSizes);
    }
    
    // *************** Descriptor Pool *********************
    
    VhlDescriptorPool::VhlDescriptorPool(
        VhlDevice& vhlDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : m_VhlDevice{vhlDevice} 
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;
        
        if (vkCreateDescriptorPool(m_VhlDevice.device(), &descriptorPoolInfo, nullptr, &m_DescriptorPool) !=
            VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    
    VhlDescriptorPool::~VhlDescriptorPool() 
    {
        vkDestroyDescriptorPool(m_VhlDevice.device(), m_DescriptorPool, nullptr);
    }
    
    bool VhlDescriptorPool::allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const 
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;
        
        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(m_VhlDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) 
        {
            return false;
        }
        return true;
    }
    
    void VhlDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const 
    {
        vkFreeDescriptorSets(
            m_VhlDevice.device(),
            m_DescriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }
    
    void VhlDescriptorPool::resetPool() 
    {
        vkResetDescriptorPool(m_VhlDevice.device(), m_DescriptorPool, 0);
    }
    
    // *************** Descriptor Writer *********************
    
    VhlDescriptorWriter::VhlDescriptorWriter(VhlDescriptorSetLayout& setLayout, VhlDescriptorPool& pool)
        : m_SetLayout{setLayout}, m_Pool{pool} {}
    
    VhlDescriptorWriter &VhlDescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo *bufferInfo) 
    {
        assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");
        
        auto& bindingDescription = m_SetLayout.m_Bindings[binding];
        
        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");
        
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;
        
        m_Writes.push_back(write);
        return *this;
    }
    
    VhlDescriptorWriter& VhlDescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo) 
    {
        assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");
        
        auto& bindingDescription = m_SetLayout.m_Bindings[binding];
        
        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");
        
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;
        
        m_Writes.push_back(write);
        return *this;
    }
        
    bool VhlDescriptorWriter::build(VkDescriptorSet& set) 
    {
        bool success = m_Pool.allocateDescriptor(m_SetLayout.getDescriptorSetLayout(), set);
        if (!success) 
        {
            return false;
        }
        overwrite(set);
        return true;
    }
        
    void VhlDescriptorWriter::overwrite(VkDescriptorSet& set) 
    {
        for (auto& write : m_Writes) 
        {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(m_Pool.m_VhlDevice.device(), static_cast<uint32_t>(m_Writes.size()), m_Writes.data(), 0, nullptr);
    }
 
}  // namespace vhl