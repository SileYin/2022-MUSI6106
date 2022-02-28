#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstring>

/*! \brief implement a circular buffer of type T
*/
template <class T> 
class CRingBuffer
{
public:
    explicit CRingBuffer(int iBufferLengthInSamples) :
        m_iBuffLength(iBufferLengthInSamples),
        m_iReadIdx(0),
        m_iWriteIdx(0),
        m_ptBuff(0)
    {
        assert(iBufferLengthInSamples > 0);

        m_ptBuff = new T[m_iBuffLength];
        for (int i = 0; i < iBufferLengthInSamples; ++i)
        {
            m_ptBuff[i] = 0;
        }
    }

    virtual ~CRingBuffer()
    {
        delete [] m_ptBuff;
        m_ptBuff = 0;
        // free memory
    }

    /*! add a new value of type T to write index and increment write index
    \param tNewValue the new value
    \return void
    */
    void putPostInc (T tNewValue)
    {
        put(tNewValue);
        m_iWriteIdx++;
        if (m_iWriteIdx == m_iBuffLength) m_iWriteIdx = 0;
    }

    /*! add a new value of type T to write index
    \param tNewValue the new value
    \return void
    */
    void put(T tNewValue)
    {
        m_ptBuff[m_iWriteIdx] = tNewValue;
    }
    
    /*! return the value at the current read index and increment the read pointer
    \return float the value from the read index
    */
    T getPostInc()
    {
        T curSample = get();
        m_iReadIdx++;
        if (m_iReadIdx == m_iBuffLength) m_iReadIdx = 0;
        return static_cast<T>(curSample);
    }

    /*! return the value at the current read index
    \return float the value from the read index
    */
    /*
    T get() const
    {
        return static_cast<T>(m_ptBuff[m_iReadIdx]);
    }
    */

    /*! return the value at the current read index
    \param fOffset: read at offset from read index
    \return float the value from the read index
    */
    T get(float fOffset = 0) const
    {
        int l_iIdx = (m_iReadIdx + static_cast<int>(floor(fOffset))) % m_iBuffLength;
        float l_fOffset = fOffset - floor(fOffset);
        assert(l_fOffset >= 0 && l_fOffset < 1);
        return static_cast<T>(m_ptBuff[l_iIdx] + l_fOffset * (m_ptBuff[(l_iIdx + 1) % m_iBuffLength] - m_ptBuff[l_iIdx]));
    }
    
    /*! set buffer content and indices to 0
    \return void
    */
    void reset()
    {
        m_iReadIdx = 0;
        m_iWriteIdx = 0;
        for (int i = 0; i < m_iBuffLength; ++i)
        {
            m_ptBuff[i] = 0;
        }
    }

    /*! return the current index for writing/put
    \return int
    */
    int getWriteIdx() const
    {
        return m_iWriteIdx;
    }

    /*! move the write index to a new position
    \param iNewWriteIdx: new position
    \return void
    */
    void setWriteIdx(int iNewWriteIdx)
    {
        m_iWriteIdx = iNewWriteIdx;
        while (m_iWriteIdx > m_iBuffLength)
        {
            m_iWriteIdx -= m_iBuffLength;
        }
        while (m_iWriteIdx < 0)
        {
            m_iWriteIdx += m_iBuffLength;
        }
    }

    /*! return the current index for reading/get
    \return int
    */
    int getReadIdx() const
    {
        return m_iReadIdx;
    }

    /*! move the read index to a new position
    \param iNewReadIdx: new position
    \return void
    */
    void setReadIdx(int iNewReadIdx)
    {
        m_iReadIdx = iNewReadIdx;
        while (m_iReadIdx > m_iBuffLength)
        {
            m_iReadIdx -= m_iBuffLength;
        }
        while (m_iReadIdx < 0)
        {
            m_iReadIdx += m_iBuffLength;
        }
    }

    /*! returns the number of values currently buffered (note: 0 could also mean the buffer is full!)
    \return int
    */
    int getNumValuesInBuffer() const
    {
        if (m_iReadIdx <= m_iWriteIdx)
        {
            return (m_iWriteIdx - m_iReadIdx);
        }
        else
        {
            return (m_iWriteIdx + m_iBuffLength - m_iReadIdx);
        }
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength() const
    {
        return m_iBuffLength;
    }
private:

    CRingBuffer();
    CRingBuffer(const CRingBuffer& that);
    T* m_ptBuff;

    int m_iReadIdx;
    int m_iWriteIdx;
    int m_iBuffLength;              //!< length of the internal buffer
};
#endif // __RingBuffer_hdr__
