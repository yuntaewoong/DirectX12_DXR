#pragma once

#include "Common/Common.h"
#include "Render\Mesh.h"
#include "Model\Model.h"
#include "Light\PointLight.h"
#include "Material\Material.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"
#include "pbrtParser/Scene.h"

namespace library
{
    /*
        Renderer가 그리게 될 Scene
    */
    class Scene
    {
    public:
        Scene(
            _In_ XMVECTOR location,
            _In_ XMVECTOR rotation,
            _In_ XMVECTOR scale
        ); //기본 생성자 => empty scene
        Scene(
            _In_ const std::filesystem::path& filePath,
            _In_ XMVECTOR location,
            _In_ XMVECTOR rotation,
            _In_ XMVECTOR scale
        ); //경로가 주어진 생성자 => pre defined scene
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
        const std::vector<std::shared_ptr<PointLight>>& GetPointLights() const;
        void AddLight(_In_ const std::shared_ptr<PointLight>& pLight);
        void AddMaterial(_In_ const std::shared_ptr<Material>& pMaterial);
        void Update(_In_ FLOAT deltaTime);
    private:
        void loadPBRTWorld(_In_ const std::shared_ptr<const pbrt::Object> object);
        void loadPBRTMaterial(_In_ const std::shared_ptr<pbrt::Material> material);
        void loadPBRTTriangleMesh(_In_ const std::shared_ptr<const pbrt::TriangleMesh> mesh);
        void loadPBRTTriangleMeshVertices(
            _In_ const std::vector<pbrt::vec3f>& vertexPositions,
            _In_ const std::vector<pbrt::vec2f>& vertexUV,
            _In_ const std::vector<pbrt::vec3f>& vertexNormals
        );
        void loadPBRTTriangleMeshIndices(
            _In_ const std::vector<pbrt::vec3i>& indices
        );
    private:
        std::filesystem::path m_filePath;
        std::vector<std::shared_ptr<Mesh>> m_meshes;
        std::vector<std::shared_ptr<Model>> m_models;
        std::vector<std::shared_ptr<PointLight>> m_pointLights;  
        std::vector<std::shared_ptr<Material>> m_materials;
        BOOL m_bInitialized;

        XMVECTOR m_location;
        XMVECTOR m_rotation;
        XMVECTOR m_scale;
    };
}