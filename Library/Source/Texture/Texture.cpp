#include "Texture\Texture.h"
#include "ThirdPartyHeader\DDSTextureLoader.h"
#include "ThirdPartyHeader\GraphicsMemory.h"
#include "ThirdPartyHeader\WICTextureLoader.h"
#include "ThirdPartyHeader\ResourceUploadBatch.h"

namespace library
{
    Texture::Texture(_In_ const std::filesystem::path& filePath) :
        m_filePath(filePath),
        m_textureResource(nullptr)
    {}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::Initialize

      Summary:  Initializes the texture and samplers if not initialized

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers

      Modifies: [m_textureRV].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Texture::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice,_In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue)
    {
        HRESULT hr = S_OK;
        ResourceUploadBatch resourceUpload(pDevice.Get());
        resourceUpload.Begin();

        hr = CreateDDSTextureFromFile(
            pDevice.Get(),
            resourceUpload,
            m_filePath.c_str(),
            m_textureResource.GetAddressOf()
        );
        if (FAILED(hr))
        {
            return hr;
        }
        // Upload the resources to the GPU.
        std::future<void> finish = resourceUpload.End(pCommandQueue.Get());
        finish.wait();//현재 Thread의 UploadBatch작업 완료 대기

        return hr;
    }
    ComPtr<ID3D12Resource>& Texture::GetTextureResource()
    {
        return m_textureResource;
    }
}