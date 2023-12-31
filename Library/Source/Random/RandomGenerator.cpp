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

        size_t cbSize = (RANDOM_SEQUENCE_LENGTH + RANDOM_SEQUENCE_LENGTH) * sizeof(FLOAT) * 2;//왜인지는 모르겠지만 Constant버퍼는 256의 배수여야함
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
        hr = m_randomConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData));//m_mappedBuffer에 Constant Buffer에 대한 주소 저장
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
    {//c++ mt19937 생성기를 이용해 uniform분포에 가까운 의사 난수생성(0,0~1.0범위)
        static std::random_device rd;//random_device: 프로그램 실행마다 다른시드값을 주기 위해 사용
	    static std::mt19937 gen(rd());//mt19937 : 2^19937 -1 주기의 의사난수 생성기
	    static std::uniform_real_distribution<FLOAT> dist(0.0f, 1.0f);//0~1범위에서 uniform분포를 가지게 변환
	    return dist(gen);
    }
    void RandomGenerator::UpdateGPUConstantBuffer()
    {
        RandomConstantBuffer cbRandom = RandomConstantBuffer();
        for (UINT i = 0; i < RANDOM_SEQUENCE_LENGTH/4; i++)
        {//GPU에서 사용할 난수테이블 생성
            cbRandom.randFloats0[i] = XMVECTOR({ randFloat(),randFloat(),randFloat(),randFloat() });
            cbRandom.randFloats1[i] = XMVECTOR({ randFloat(),randFloat(),randFloat(),randFloat() });
        }
        memcpy(m_mappedData, &cbRandom, sizeof(cbRandom));
    }
}