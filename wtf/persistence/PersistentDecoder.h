/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <wtf/EnumTraits.h>
#include <wtf/SHA1.h>
#include <wtf/persistence/PersistentCoder.h>

namespace WTF {
namespace Persistence {

class Decoder {
    WTF_MAKE_FAST_ALLOCATED;
public:
    WTF_EXPORT_PRIVATE Decoder(const uint8_t* buffer, size_t bufferSize);
    WTF_EXPORT_PRIVATE ~Decoder();

    size_t length() const { return m_bufferEnd - m_buffer; }
    size_t currentOffset() const { return m_bufferPosition - m_buffer; }

    WTF_EXPORT_PRIVATE WARN_UNUSED_RETURN bool verifyChecksum();

    WTF_EXPORT_PRIVATE WARN_UNUSED_RETURN bool decodeFixedLengthData(uint8_t*, size_t);

    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<bool>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<uint8_t>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<uint16_t>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<uint32_t>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<uint64_t>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<int16_t>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<int32_t>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<int64_t>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<float>&);
    WTF_EXPORT_PRIVATE Decoder& operator>>(Optional<double>&);

    template<typename T, std::enable_if_t<!std::is_arithmetic<typename std::remove_const<T>>::value && !std::is_enum<T>::value>* = nullptr>
    Decoder& operator>>(Optional<T>& result)
    {
        result = Coder<T>::decode(*this);
        return *this;
    }

    template<typename E, std::enable_if_t<std::is_enum<E>::value>* = nullptr>
    Decoder& operator>>(Optional<E>& result)
    {
        static_assert(sizeof(E) <= 8, "Enum type T must not be larger than 64 bits!");
        Optional<uint64_t> value;
        *this >> value;
        if (!value)
            return *this;
        if (!isValidEnum<E>(*value))
            return *this;
        result = static_cast<E>(*value);
        return *this;
    }

    template<typename T> WARN_UNUSED_RETURN
    bool bufferIsLargeEnoughToContain(size_t numElements) const
    {
        static_assert(std::is_arithmetic<T>::value, "Type T must have a fixed, known encoded size!");

        if (numElements > std::numeric_limits<size_t>::max() / sizeof(T))
            return false;

        return bufferIsLargeEnoughToContain(numElements * sizeof(T));
    }

    static constexpr bool isIPCDecoder = false;

private:
    WTF_EXPORT_PRIVATE WARN_UNUSED_RETURN bool bufferIsLargeEnoughToContain(size_t) const;
    template<typename Type> Decoder& decodeNumber(Optional<Type>&);

    const uint8_t* m_buffer;
    const uint8_t* m_bufferPosition;
    const uint8_t* m_bufferEnd;

    SHA1 m_sha1;
};

} 
}

