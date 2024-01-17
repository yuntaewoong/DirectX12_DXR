#pragma once
#include "Common/Common.h"
#include "Render\Mesh.h"
#include "Material\Material.h"

struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiAnimation;
struct aiBone;
struct aiNode;
struct aiNodeAnim;

namespace Assimp
{
    class Importer;
}

namespace library
{
    class Model
    {
    public:
        Model() = delete;
        Model(
            _In_ const std::filesystem::path& filePath,
            _In_ XMVECTOR location,
            _In_ XMVECTOR rotation,
            _In_ XMVECTOR scale,
            _In_ BOOL reverseNormalOnLoad = FALSE
        );
        Model(const Model& other) = delete;
        Model(Model&& other) = delete;
        Model& operator=(const Model& other) = delete;
        Model& operator=(Model&& other) = delete;
        ~Model() = default;

        HRESULT Initialize(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
        );
        void Update(_In_ FLOAT deltaTime);
        void ForceMaterial(_In_ const std::shared_ptr<Material>& material);
        const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const;
        const std::vector<std::shared_ptr<Material>>& GetMaterials() const;
    private:
        void setMeshes2Materials(_In_ const aiScene* pScene);
        HRESULT initAllMeshes(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const aiScene* pScene
        );
        HRESULT initFromScene(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
            _In_ const aiScene* pScene,
            _In_ const std::filesystem::path& filePath
        );
        HRESULT initMaterials(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
            _In_ const aiScene* pScene,
            _In_ const std::filesystem::path& filePath
        );
        HRESULT initSingleMesh(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ UINT uMeshIndex,
            _In_ const aiMesh* pMesh
        );
        HRESULT loadDiffuseTexture(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
            _In_ const std::filesystem::path& parentDirectory,
            _In_ const aiMaterial* pMaterial,
            _In_ UINT uIndex
        );
        HRESULT loadSpecularTexture(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
            _In_ const std::filesystem::path& parentDirectory,
            _In_ const aiMaterial* pMaterial,
            _In_ UINT uIndex
        );
        HRESULT loadNormalTexture(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
            _In_ const std::filesystem::path& parentDirectory,
            _In_ const aiMaterial* pMaterial,
            _In_ UINT uIndex
        );
        HRESULT loadTextures(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
            _In_ const std::filesystem::path& parentDirectory,
            _In_ const aiMaterial* pMaterial,
            _In_ UINT uIndex
        );
    private:
        static std::unique_ptr<Assimp::Importer> sm_pImporter;
    private:
        std::filesystem::path m_filePath;
        std::vector<std::shared_ptr<Mesh>> m_meshes;
        std::vector<std::shared_ptr<Material>> m_materials;
        const aiScene* m_pScene;
        XMVECTOR m_location;
        XMVECTOR m_rotation;
        XMVECTOR m_scale;

        BOOL m_bReverseNoramlOnLoad;
    };
}