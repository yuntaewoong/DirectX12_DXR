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

        //���� heap type�� upload�� �� ���·� vertex buffer�� gpu�޸𸮿� �����ϴµ�, �̴� ���� ���� ���
        //GPU�� �����Ҷ����� �������� �Ͼ�ٰ� ���������� �ּ��� ����
        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
        hr = pDevice->CreateCommittedResource(//�� ũ�� == ������ ũ��� ����, �ڿ� �Ҵ�
            &heapProperties,                    //�� Ÿ��
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,                      //�ڿ� ũ������
            D3D12_RESOURCE_STATE_GENERIC_READ,  //���� ����
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)       //CPU�޸𸮿��� ���ٰ����� ComPtr��ü
        );

        UINT8* pVertexDataBegin = nullptr;    // gpu�޸𸮿� mapping �� cpu�޸�(virtual memory�� �ü�� ���� �����ϴµ�)
        CD3DX12_RANGE readRange(0, 0);        // 0~0���� ������ CPU�޸𸮷� gpu������ �б� ���� ����, nullptr�Է��ϸ� gpu������ �б� ����
        hr = m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));//����
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(pVertexDataBegin, triangleVertices, vertexBufferSize);//gpu �޸� ����
        m_vertexBuffer->Unmap(0, nullptr);//���� ����
        return hr;
	}
	HRESULT Renderable::createIndexBuffer(_In_ ID3D12Device* pDevice)
	{
        HRESULT hr = S_OK;
        const Index* indices = GetIndices();
        const UINT indexBufferSize = sizeof(Index) * GetNumIndices();

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);//heap type�� upload
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
        hr = pDevice->CreateCommittedResource(//�� ũ�� == ������ ũ��� ����, �ڿ� �Ҵ�
            &heapProperties,                    //�� Ÿ��
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,                      //�ڿ� ũ������
            D3D12_RESOURCE_STATE_GENERIC_READ,  //���� ����
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)       //CPU�޸𸮿��� ���ٰ����� ComPtr��ü
        );
        if (FAILED(hr))
        {
            return hr;
        }
        UINT8* pIndexDataBegin = nullptr;    // gpu�޸𸮿� mapping �� cpu�޸�(virtual memory�� �ü�� ���� �����ϴµ�)
        CD3DX12_RANGE readRange(0, 0);        // 0~0���� ������ CPU�޸𸮷� gpu������ �б� ���� ����, nullptr�Է��ϸ� gpu������ �б� ����
        hr = m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));//����
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(pIndexDataBegin, indices, indexBufferSize);//gpu �޸� ����
        m_indexBuffer->Unmap(0, nullptr);//���� ����
        return hr;
	}
}