#pragma once

#include "Common/Common.h"

namespace library
{
    /*
        ShaderIdentifier¿Í localRootArgumentÀÇ Wrapper
    */
    class ShaderRecord final
    {
    public:
        ShaderRecord();
        ShaderRecord(_In_ void* pShaderIdentifier, _In_ UINT nShaderIdentifierSize);
        ShaderRecord(_In_ void* pShaderIdentifier, _In_ UINT nShaderIdentifierSize, _In_ void* pLocalRootArguments, _In_ UINT nLocalRootArgumentsSize);
        ~ShaderRecord() = default;

        void CopyTo(void* dest) const;
    private:
        struct PointerWithSize {
            PointerWithSize() : 
                ptr(nullptr),
                size(0) 
            {}
            PointerWithSize(_In_ void* _ptr,_In_ UINT _size) :
                ptr(_ptr), 
                size(_size) 
            {}
            void* ptr;
            UINT size;
        };
        PointerWithSize shaderIdentifier;
        PointerWithSize localRootArguments;
    };
}