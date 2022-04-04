
#include "FastConv.h"

CFastConv::CFastConv(void) :
    bIsInitialized(0)
{
}

CFastConv::~CFastConv( void )
{
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/)
{
    if (bIsInitialized)
        return Error_t::kFunctionIllegalCallError;

    if (eCompMode == ConvCompMode_t::kTimeDomain)
    {
        pImpl = new CConvTimeDomain(pfImpulseResponse, iLengthOfIr, iBlockLength);
        bIsInitialized = true;
    }
    else if (eCompMode == ConvCompMode_t::kTimeDomain)
    {
        return Error_t::kFunctionIllegalCallError;
    }
    else
    {
        return Error_t::kFunctionInvalidArgsError;
    }
    return Error_t::kNoError;
}

Error_t CFastConv::reset()
{
    if (!bIsInitialized)
        return Error_t::kFunctionIllegalCallError;
    delete pImpl;
    return Error_t::kNoError;
}

Error_t CFastConv::process (float* pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers )
{
    return Error_t::kNoError;
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer)
{
    return Error_t::kNoError;
}


class CConvBase
{
protected:
    CConvBase(float* pfImpulseResponse, int iLengthOfIr, int iBlockLength) :
        m_iLengthOfIr(iLengthOfIr),
        m_iBlockLength(iBlockLength)
    {
    }
    virtual ~CConvBase() 
    {
    }
    virtual void process(float* pfOutputBuffer, const float* pfInputBuffer, int iLengthOfBuffers) = 0;
    virtual void flushBuffer(float* pfOutputBuffer) = 0;

    int m_iLengthOfIr;
    int m_iBlockLength;
};

class CConvTimeDomain: public CConvBase
{
public:
    CConvTimeDomain(float* pfImpulseResponse, int iLengthOfIr, int iBlockLength) :
        CConvBase(pfImpulseResponse, iLengthOfIr, iBlockLength)
    {
        m_pfImpulseResponse = new float[m_iLengthOfIr];
        std::memcpy(pfImpulseResponse, m_pfImpulseResponse, sizeof(float) * m_iLengthOfIr);
        m_pcRingBuff = new CRingBuffer<float>(m_iLengthOfIr);
    }

    virtual ~CConvTimeDomain()
    {
        delete[] m_pfImpulseResponse;
        m_pfImpulseResponse = 0;
        delete m_pcRingBuff;
        m_pcRingBuff = 0;
    }

    void process(float* pfOutputBuffer, const float* pfInputBuffer, int iLengthOfBuffers)
    {
        std::memset(pfOutputBuffer, 0, sizeof(float) * iLengthOfBuffers);
        for (int i = 0; i < iLengthOfBuffers; i++)
        {
            m_pcRingBuff->setReadIdx(m_pcRingBuff->getWriteIdx() + 1);
            m_pcRingBuff->putPostInc(pfInputBuffer[i]);
            for (int j = m_iLengthOfIr - 1; j >= 0; j--)
            {
                pfOutputBuffer[i] += m_pfImpulseResponse[j] * m_pcRingBuff->getPostInc();
            }
            m_pcRingBuff->getPostInc();
            assert(m_pcRingBuff->getReadIdx() == m_pcRingBuff->getWriteIdx());
        }
    }

    void flushBuffer(float* pfOutputBuffer)
    {
        float* pfFlushInputBuffer = new float[m_iLengthOfIr];
        std::memset(pfFlushInputBuffer, 0, sizeof(float) * m_iLengthOfIr);
    }

private:
    float* m_pfImpulseResponse = 0;

    CRingBuffer<float>* m_pcRingBuff = 0;

};

class CConvFreqDomain
{

};