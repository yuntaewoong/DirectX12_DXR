#include "pch.h"
#include "Random\RandomGenerator.h"
#include <random>

namespace library
{
	RandomGenerator::RandomGenerator() :
		m_randomConstantBuffer(nullptr),
        m_mappedData(nullptr)
	{}
	ComPtr<ID3D12Resource>& RandomGenerator::GetConstantBuffer()
	{
		return m_randomConstantBuffer;
	}
	HRESULT RandomGenerator::Initialize(ID3D12Device* pDevice)
	{
		HRESULT hr = S_OK;
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        size_t cbSize = (RANDOM_SEQUENCE_LENGTH + RANDOM_SEQUENCE_LENGTH) * sizeof(FLOAT) * 2;//�������� �𸣰����� Constant���۴� 256�� ���������
        const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
        hr = pDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_randomConstantBuffer)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        CD3DX12_RANGE readRange(0, 0);  
        hr = m_randomConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData));//m_mappedBuffer�� Constant Buffer�� ���� �ּ� ����
        if (FAILED(hr))
        {
            return hr;
        }
        UpdateGPUConstantBuffer();
        return hr;
	}
	void RandomGenerator::Update(FLOAT deltaTime)
	{
        UpdateGPUConstantBuffer();
	}
    FLOAT RandomGenerator::randFloat()
    {//c++ mt19937 �����⸦ �̿��� uniform������ ����� �ǻ� ��������(0,0~1.0����)
        static std::random_device rd;//random_device: ���α׷� ���ึ�� �ٸ��õ尪�� �ֱ� ���� ���
	    static std::mt19937 gen(rd());//mt19937 : 2^19937 -1 �ֱ��� �ǻ糭�� ������
	    static std::uniform_real_distribution<FLOAT> dist(0.0f, 1.0f);//0~1�������� uniform������ ������ ��ȯ
	    return dist(gen);
    }
    void RandomGenerator::UpdateGPUConstantBuffer()
    {
        RandomConstantBuffer cbRandom = RandomConstantBuffer();
        for (UINT i = 0; i < RANDOM_SEQUENCE_LENGTH/4; i++)
        {//GPU���� ����� �������̺� ����
            cbRandom.randFloats0[i] = XMVECTOR({ randFloat(),randFloat(),randFloat(),randFloat() });
            cbRandom.randFloats1[i] = XMVECTOR({ randFloat(),randFloat(),randFloat(),randFloat() });
        }
        memcpy(m_mappedData, &cbRandom, sizeof(cbRandom));
    }
}