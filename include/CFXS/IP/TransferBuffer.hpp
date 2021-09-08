// [CFXS] //
// Main data structure for sending, receiving and processing network data
#pragma once

#include <cstdint>
#include <CFXS/Base/RefCounter.hpp>
#include <CFXS/IP/_LoggerConfig.hpp>

namespace CFXS::IP {

    class TransferBuffer : public RefCounter<uint16_t, TransferBuffer> {
        friend class RefCounter<uint16_t, TransferBuffer>;

    public:
        TransferBuffer() = default;

        /// \returns next buffer in chain, nullptr if end of chain
        TransferBuffer* GetNextBuffer() {
            return m_NextBuffer;
        }

        /// Set next buffer in chain and increment its ref count
        /// \param nextBuffer add next buffer to chain
        void SetNextBuffer(TransferBuffer* nextBuffer) {
            CFXS_ASSERT(!GetNextBuffer(), "Next buffer already set");
            m_NextBuffer = nextBuffer;
        }

        /// \returns true if data is allocated
        inline bool IsDataAllocated() const {
            return m_IsDataAllocated;
        }

    private:
        /// Called when DecRefCount sets ref count to 0
        void RefCountZero() {
            NetworkLogger_Base::Log("TransferBuffer ref count is 0");
        }

    private:
        uint16_t m_Size              = 0;
        void* m_Data                 = nullptr;
        TransferBuffer* m_NextBuffer = nullptr;
        union {
            struct {
                bool m_IsDataAllocated : 1;
            };
            uint32_t m_Flags = 0;
        };
    };

} // namespace CFXS::IP