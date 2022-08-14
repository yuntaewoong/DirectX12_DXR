#pragma once

#include "Common\Common.h"

namespace library
{
    class Texture
    {
    public:
        Texture(_In_ const std::filesystem::path& filePath);
        Texture(const Texture& other) = delete;
        Texture(Texture&& other) = delete;
        Texture& operator=(const Texture& other) = delete;
        Texture& operator=(Texture&& other) = delete;
        virtual ~Texture() = default;

        HRESULT Initialize(_In_ const ComPtr<ID3D12Device>& pDevice,_In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue);

        ComPtr<ID3D12Resource>& GetTextureResource();
    protected:
        std::filesystem::path m_filePath;
        ComPtr<ID3D12Resource> m_textureResource;
    };
}