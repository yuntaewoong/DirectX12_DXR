#include "pch.h"
#include "ShaderTable\ShaderRecord.h"
namespace library
{
	ShaderRecord::ShaderRecord()
	{}
	ShaderRecord::ShaderRecord(_In_ void* pShaderIdentifier, _In_ UINT nShaderIdentifierSize) :
		shaderIdentifier(pShaderIdentifier, nShaderIdentifierSize)
	{}
	ShaderRecord::ShaderRecord(_In_ void* pShaderIdentifier, _In_ UINT nShaderIdentifierSize, _In_ void* pLocalRootArguments, _In_ UINT nLocalRootArgumentsSize) :
		shaderIdentifier(pShaderIdentifier, nShaderIdentifierSize),
		localRootArguments(pLocalRootArguments, nLocalRootArgumentsSize)
	{}
	void ShaderRecord::CopyTo(void* dest) const
	{
		uint8_t* byteDest = static_cast<uint8_t*>(dest);
		memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
		if (localRootArguments.ptr)
		{
			memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
		}
	} 
}

