#include "Render\Renderable.h"

namespace library
{
	Renderable::Renderable(
        _In_ XMVECTOR location,
        _In_ XMVECTOR rotation,
        _In_ XMVECTOR scale
    ) :
		m_vertexBuffer(nullptr),
		m_indexBuffer(nullptr),
        m_world(XMMatrixIdentity())
	{
        m_world = m_world *
            XMMatrixRotationRollPitchYawFromVector(rotation) *
            XMMatrixScalingFromVector(scale) *
            XMMatrixTranslationFromVector(location);
        m_world = m_world;
    }
    ComPtr<ID3D12Resource>& Renderable::GetVertexBuffer()
    {
        return m_vertexBuffer;
    }
    ComPtr<ID3D12Resource>& Renderable::GetIndexBuffer()
    {
        return m_indexBuffer;
    }
    XMMATRIX Renderable::GetWorldMatrix() const
    {
        return m_world;
    }
    HRESULT Renderable::initialize(_In_ ID3D12Device* pDevice)
	{
		HRESULT hr = S_OK;
        hr = createVertexBuffer(pDevice);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createIndexBuffer(pDevice);
        if (FAILED(hr))
        {
            return hr;
        }
		return hr;
	}
	HRESULT Renderable::createVertexBuffer(_In_ ID3D12Device* pDevice)
	{
        HRESULT hr = S_OK;
        const Vertex* triangleVertices = GetVertices();
        const UINT vertexBufferSize = sizeof(Vertex) * GetNumVertices();

        //현재 heap type을 upload로 한 상태로 vertex buffer를 gpu메모리에 생성하는데, 이는 좋지 않은 방법
        //GPU가 접근할때마다 마샬링이 일어난다고 마소직원이 주석을 남김
        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
        hr = pDevice->CreateCommittedResource(//힙 크기 == 데이터 크기로 힙과, 자원 할당
            &heapProperties,                    //힙 타입
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,                      //자원 크기정보
            D3D12_RESOURCE_STATE_GENERIC_READ,  //접근 정보
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)       //CPU메모리에서 접근가능한 ComPtr개체
        );

        UINT8* pVertexDataBegin = nullptr;    // gpu메모리에 mapping 될 cpu메모리(virtual memory로 운영체제 통해 접근하는듯)
        CD3DX12_RANGE readRange(0, 0);        // 0~0으로 설정시 CPU메모리로 gpu데이터 읽기 불허 가능, nullptr입력하면 gpu데이터 읽기 가능
        hr = m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));//매핑
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(pVertexDataBegin, triangleVertices, vertexBufferSize);//gpu 메모리 전송
        m_vertexBuffer->Unmap(0, nullptr);//매핑 해제
        return hr;
	}
	HRESULT Renderable::createIndexBuffer(_In_ ID3D12Device* pDevice)
	{
        HRESULT hr = S_OK;
        const Index* indices = GetIndices();
        const UINT indexBufferSize = sizeof(Index) * GetNumIndices();

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);//heap type은 upload
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
        hr = pDevice->CreateCommittedResource(//힙 크기 == 데이터 크기로 힙과, 자원 할당
            &heapProperties,                    //힙 타입
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,                      //자원 크기정보
            D3D12_RESOURCE_STATE_GENERIC_READ,  //접근 정보
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)       //CPU메모리에서 접근가능한 ComPtr개체
        );
        if (FAILED(hr))
        {
            return hr;
        }
        UINT8* pIndexDataBegin = nullptr;    // gpu메모리에 mapping 될 cpu메모리(virtual memory로 운영체제 통해 접근하는듯)
        CD3DX12_RANGE readRange(0, 0);        // 0~0으로 설정시 CPU메모리로 gpu데이터 읽기 불허 가능, nullptr입력하면 gpu데이터 읽기 가능
        hr = m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));//매핑
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(pIndexDataBegin, indices, indexBufferSize);//gpu 메모리 전송
        m_indexBuffer->Unmap(0, nullptr);//매핑 해제
        return hr;
	}
}