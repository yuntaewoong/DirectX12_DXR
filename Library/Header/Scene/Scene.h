#pragma once

#include "Common/Common.h"
#include "Render\Mesh.h"
#include "Model\Model.h"
#include "Light\PointLight.h"
#include "Texture\Material.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"

namespace pbrt
{
    struct Object;
};


namespace library
{
    /*
        Renderer가 그리게 될 Scene
    */
    class Scene
    {
    public:
        Scene(); //기본 생성자 => empty scene
        Scene(_In_ const std::filesystem::path& filePath); //경로가 주어진 생성자 => pre defined scene
        Scene(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene& operator=(Scene&& other) = delete;
        ~Scene() = default;

        HRESULT Initialize(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
        );
        void AddMesh(_In_ const std::shared_ptr<Mesh>& pMesh);
        void AddModel(_In_ const std::shared_ptr<Model>& pModel);
        const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const;
        void AddLight(_In_ const std::shared_ptr<PointLight>& pLight);
        void AddMaterial(_In_ const std::shared_ptr<Material>& pMaterial);
        void Update(_In_ FLOAT deltaTime);
        ComPtr<ID3D12Resource>& GetPointLightsConstantBuffer();
    private:
        void traverse(std::shared_ptr<pbrt::Object> object);
        HRESULT createLightConstantBuffer(_In_ const ComPtr<ID3D12Device>& pDevice);
        void updateLightConstantBuffer();
    private:
        std::filesystem::path m_filePath;
        std::vector<std::shared_ptr<Mesh>> m_meshes;
        std::vector<std::shared_ptr<Model>> m_models;
        std::vector<std::shared_ptr<PointLight>> m_lights;  
        std::vector<std::shared_ptr<Material>> m_materials;
        ComPtr<ID3D12Resource> m_pointLightsConstantBuffer;
        void* m_pointLightMappedData;
    };
}