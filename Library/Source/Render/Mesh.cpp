#include "pch.h"
#include "Render\Mesh.h"

namespace library
{
    Mesh::Mesh(
        _In_ XMVECTOR location,
        _In_ XMVECTOR rotation,
        _In_ XMVECTOR scale,
        _In_ XMFLOAT4 color
    ) :
		m_vertexBuffer(nullptr),
		m_indexBuffer(nullptr),
        m_world(XMMatrixIdentity()),
        m_color(color),
        m_material(),
        m_vertices(std::vector<Vertex>()),
        m_indices(std::vector<Index>())
        
	{
        m_world = m_world *
            XMMatrixRotationRollPitchYawFromVector(rotation) *
            XMMatrixScalingFromVector(scale) *
            XMMatrixTranslationFromVector(location);
    }
    HRESULT Mesh::Initialize(const ComPtr<ID3D12Device>& pDevice)
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
    void Mesh::Update(FLOAT deltaTime)
    {
    }
    ComPtr<ID3D12Resource>& Mesh::GetVertexBuffer()
    {
        return m_vertexBuffer;
    }
    ComPtr<ID3D12Resource>& Mesh::GetIndexBuffer()
    {
        return m_indexBuffer;
    }
    const std::vector<Vertex>& Mesh::GetVertices() const
    {
        return m_vertices;
    }
    const std::vector<Index>& Mesh::GetIndices() const
    {
        return m_indices;
    }
    XMMATRIX Mesh::GetWorldMatrix() const
    {
        return m_world;
    }
    XMFLOAT4 Mesh::GetColor() const
    {
        return m_color;
    }
    const std::shared_ptr<Material>& Mesh::GetMaterial() const
    {
        return m_material;
    }
    void Mesh::SetMaterial(_In_ const std::shared_ptr<Material>& pMaterial)
    {
        m_material = pMaterial;
    }
    void Mesh::AddVertex(Vertex vertex)
    {
        m_vertices.push_back(vertex);
    }
    void Mesh::AddIndex(Index index)
    {
        m_indices.push_back(index);
    }
	HRESULT Mesh::createVertexBuffer(_In_ const ComPtr<ID3D12Device>& pDevice)
	{
        HRESULT hr = S_OK;
        const Vertex* triangleVertices = GetVertices().data();
        const UINT vertexBufferSize = sizeof(Vertex) * static_cast<UINT>(GetVertices().size());

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
	HRESULT Mesh::createIndexBuffer(_In_ const ComPtr<ID3D12Device>& pDevice)
	{
        HRESULT hr = S_OK;
        const Index* indices = GetIndices().data();
        const UINT indexBufferSize = sizeof(Index) * static_cast<UINT>(GetIndices().size());

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