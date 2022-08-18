#include "pch.h"
#include "Texture\Texture.h"
#include "ThirdPartyHeader\DDSTextureLoader.h"
#include "ThirdPartyHeader\WICTextureLoader.h"
#include "ThirdPartyHeader\ResourceUploadBatch.h"

namespace library
{
    Texture::Texture(_In_ const std::filesystem::path& filePath) :
        m_filePath(filePath),
        m_textureResource(nullptr),
        m_descriptorHandle()
    {}
    HRESULT Texture::Initialize(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
    )
    {
        HRESULT hr = S_OK;
        ResourceUploadBatch resourceUpload(pDevice.Get());
        resourceUpload.Begin();//upload명령 기록시작

        hr = CreateWICTextureFromFile(
            pDevice.Get(),
            resourceUpload,
            m_filePath.c_str(),
            m_textureResource.GetAddressOf()
        );
        if (FAILED(hr))
        {
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
        }
        std::future<void> finish = resourceUpload.End(pCommandQueue.Get());//업로드 명령 기록끝, GPU 업로드 작업 시작
        finish.wait();//현재 Thread의 UploadBatch작업 완료 대기

        
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
            .Format = m_textureResource->GetDesc().Format,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D = {
                .MipLevels = 1
            }
        };

        hr = cbvSrvUavDescriptorHeap.CreateSRV(//Descriptor Heap에 SRV생성
            pDevice,
            m_textureResource,
            srvDesc,
            &m_descriptorHandle
        );
        if (FAILED(hr))
        {
            return hr;
        }


        return hr;
    }
    D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetDescriptorHandle() const
    {
        return m_descriptorHandle;
    }
}