
#include "FastConv.h"




class CConvBase
{
public:
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

protected:
    int m_iLengthOfIr;
    int m_iBlockLength;
};

class CConvTimeDomain : public CConvBase
{
public:
    CConvTimeDomain(float* pfImpulseResponse, int iLengthOfIr, int iBlockLength) :
        CConvBase(pfImpulseResponse, iLengthOfIr, iBlockLength)
    {
        m_pfImpulseResponse = new float[m_iLengthOfIr];
        std::memcpy(m_pfImpulseResponse, pfImpulseResponse, sizeof(float) * m_iLengthOfIr);
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
            assert(m_pcRingBuff->getReadIdx() == m_pcRingBuff->getWriteIdx());
        }
    }

    void flushBuffer(float* pfOutputBuffer)
    {
        float* pfFlushInputBuffer = new float[m_iLengthOfIr - 1]{ 0 };
        process(pfOutputBuffer, pfFlushInputBuffer, m_iLengthOfIr - 1);
        delete[] pfFlushInputBuffer;
    }

private:
    float* m_pfImpulseResponse = 0;

    CRingBuffer<float>* m_pcRingBuff = 0;

};

class CConvFreqDomain: public CConvBase
{
public:
    CConvFreqDomain(float* pfImpulseResponse, int iLengthOfIr, int iBlockLength) :
        CConvBase(pfImpulseResponse, iLengthOfIr, iBlockLength),
        m_iBlockLength(iBlockLength),
        m_iWriteBlockIdx(0),
        m_iWriteIdx(0)
    {
        m_iBlockNum = static_cast<int>(std::ceil(static_cast<float>(m_iLengthOfIr) / static_cast<float>(m_iBlockLength)));
        m_iNextBlockIdx = (m_iWriteBlockIdx + 1) % m_iBlockNum;
        m_iReadBlockIdx = m_iBlockNum - 1;

        CFft::createInstance(pcFFT);
        pcFFT->initInstance(2 * m_iBlockLength, 1, CFft::kWindowHann, CFft::kNoWindow);

        pfComplexTemp = new CFft::complex_t[2 * m_iBlockLength];

        pfFFTRealTemp = new float[m_iBlockLength + 1]; //Why am I doing this? Looks like FFT split real imag would corrupt the heap memory when block length is very short.
        pfFFTImagTemp = new float[m_iBlockLength + 1];
        pfCurrentBlockFFTReal = new float[m_iBlockLength + 1];
        pfCurrentBlockFFTImag = new float[m_iBlockLength + 1];
        pfIFFTTemp = new float[2 * m_iBlockLength]{0};

        m_ppfIRFreqDomainReal = new float* [m_iBlockNum];
        m_ppfIRFreqDomainImag = new float* [m_iBlockNum];
        m_ppfInputBlockBuffer = new float* [m_iBlockNum];
        m_ppfProcessedBlockBuffer = new float* [m_iBlockNum];
        for (int i = 0; i < m_iBlockNum; i++)
        {
            m_ppfIRFreqDomainReal[i] = new float[m_iBlockLength + 1]{ 0 };
            m_ppfIRFreqDomainImag[i] = new float[m_iBlockLength + 1]{ 0 };
            m_ppfInputBlockBuffer[i] = new float[2 * m_iBlockLength]{ 0 };
            m_ppfProcessedBlockBuffer[i] = new float[m_iBlockLength] {0};
            for (int j = 0; j < m_iBlockLength; j++)
            {
                if (i * iBlockLength + j < m_iLengthOfIr)
                    pfIFFTTemp[j] = pfImpulseResponse[i * iBlockLength + j];
                else
                    pfIFFTTemp[j] = 0;
            }

            for (int j = m_iBlockLength; j < 2 * m_iBlockLength; j++)
                pfIFFTTemp[j] = 0;

            pcFFT->doFft(pfComplexTemp, pfIFFTTemp);
            pcFFT->splitRealImag(m_ppfIRFreqDomainReal[i], m_ppfIRFreqDomainImag[i], pfComplexTemp);
        }
    }

    virtual ~CConvFreqDomain()
    {
        for (int i = 0; i < m_iBlockNum; i++)
        {
            delete[] m_ppfInputBlockBuffer[i];
            delete[] m_ppfProcessedBlockBuffer[i];
            delete[] m_ppfIRFreqDomainReal[i];
            delete[] m_ppfIRFreqDomainImag[i];
        }
        delete[] m_ppfIRFreqDomainReal;
        delete[] m_ppfIRFreqDomainImag;
        delete[] m_ppfInputBlockBuffer;
        delete[] m_ppfProcessedBlockBuffer;

        m_ppfIRFreqDomainReal = 0;
        m_ppfIRFreqDomainImag = 0;
        m_ppfInputBlockBuffer = 0;
        m_ppfProcessedBlockBuffer = 0;

        delete[] pfIFFTTemp;
        delete[] pfFFTRealTemp;
        delete[] pfFFTImagTemp;
        delete[] pfCurrentBlockFFTReal;
        delete[] pfCurrentBlockFFTImag;
        pfIFFTTemp = 0;
        pfFFTRealTemp = 0;
        pfFFTImagTemp = 0;


        delete[] pfComplexTemp;
        pfComplexTemp = 0;

        CFft::destroyInstance(pcFFT);
        pcFFT = 0;
    }

    void process(float* pfOutputBuffer, const float* pfInputBuffer, int iLengthOfBuffers)
    {

        for (int i = 0; i < iLengthOfBuffers; i++)
        {

            assert(m_iWriteIdx + m_iBlockLength < 2 * m_iBlockLength);
            assert(m_iReadBlockIdx < m_iBlockNum);
            assert(m_iWriteBlockIdx < m_iBlockNum);
            assert(m_iNextBlockIdx < m_iBlockNum);

            m_ppfInputBlockBuffer[m_iWriteBlockIdx][m_iWriteIdx + m_iBlockLength] = pfInputBuffer[i];
            m_ppfInputBlockBuffer[m_iNextBlockIdx][m_iWriteIdx] = pfInputBuffer[i];

            pfOutputBuffer[i] = m_ppfProcessedBlockBuffer[m_iReadBlockIdx][m_iWriteIdx];

            m_iWriteIdx++;

            if (m_iWriteIdx == m_iBlockLength)
            {
                m_iWriteIdx = 0;
                for (int j = 0; j < m_iBlockLength; j++)
                {
                    m_ppfProcessedBlockBuffer[m_iReadBlockIdx][j] = 0;
                }
                pcFFT->doFft(pfComplexTemp, m_ppfInputBlockBuffer[m_iWriteBlockIdx]);
                pcFFT->splitRealImag(pfCurrentBlockFFTReal, pfCurrentBlockFFTImag, pfComplexTemp);
                for (int j = 0; j < m_iBlockNum; j++)
                { 
                    complexMultiplication(pfFFTRealTemp, pfFFTImagTemp, 
                        pfCurrentBlockFFTReal, pfCurrentBlockFFTImag, 
                        m_ppfIRFreqDomainReal[j], m_ppfIRFreqDomainImag[j]);
                    pcFFT->mergeRealImag(pfComplexTemp, pfFFTRealTemp, pfFFTImagTemp);
                    pcFFT->doInvFft(pfIFFTTemp, pfComplexTemp);
                    for (int k = 0; k < m_iBlockLength; k++)
                    {
                        m_ppfProcessedBlockBuffer[(m_iWriteBlockIdx + j) % m_iBlockNum][k] += pfIFFTTemp[k + m_iBlockLength];
                    }
                }

                m_iReadBlockIdx = m_iWriteBlockIdx;
                m_iWriteBlockIdx = m_iNextBlockIdx;
                m_iNextBlockIdx = (m_iWriteBlockIdx + 1) % m_iBlockNum;
            }
        }
    }

    void flushBuffer(float* pfOutputBuffer)
    {
        float* pfFlushInputBuffer = new float[m_iBlockLength + m_iLengthOfIr - 1]{ 0 };
        process(pfOutputBuffer, pfFlushInputBuffer, m_iBlockLength + m_iLengthOfIr - 1);
        delete[] pfFlushInputBuffer;
    }

private:

    void complexMultiplication(float* pfOutReal, float* pfOutImag, const float* pfSignalReal, const float* pfSignalImag, const float* pfIRReal, const float* pfIRImag)
    {
        float tempRe, tempIm;
        for (int i = 0; i <= m_iBlockLength; i++)
        {
            pfOutReal[i] = (pfSignalReal[i] * pfIRReal[i] - pfSignalImag[i] * pfIRImag[i]) * 2 * m_iBlockLength;
            pfOutImag[i] = (pfSignalReal[i] * pfIRImag[i] + pfSignalImag[i] * pfIRReal[i]) * 2 * m_iBlockLength;
        }
    }


    int m_iBlockNum;
    int m_iBlockLength;

    int m_iReadBlockIdx;
    int m_iWriteBlockIdx;
    int m_iNextBlockIdx;
    int m_iWriteIdx;

    float** m_ppfIRFreqDomainReal = 0;
    float** m_ppfIRFreqDomainImag = 0;
    float** m_ppfInputBlockBuffer = 0;
    float** m_ppfProcessedBlockBuffer = 0;

    float* pfFFTRealTemp = 0;
    float* pfFFTImagTemp = 0;

    float* pfCurrentBlockFFTReal = 0;
    float* pfCurrentBlockFFTImag = 0;

    float* pfIFFTTemp = 0;



    CFft* pcFFT = 0;
    CFft::complex_t* pfComplexTemp = 0;
};

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
    else if (eCompMode == ConvCompMode_t::kFreqDomain)
    {
        pImpl = new CConvFreqDomain(pfImpulseResponse, iLengthOfIr, iBlockLength);
        bIsInitialized = true;
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
    pImpl = 0;
    return Error_t::kNoError;
}

Error_t CFastConv::process (float* pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers )
{
    if (!bIsInitialized)
        return Error_t::kFunctionIllegalCallError;
    pImpl->process(pfOutputBuffer, pfInputBuffer, iLengthOfBuffers);
    return Error_t::kNoError;
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer)
{
    pImpl->flushBuffer(pfOutputBuffer);
    return Error_t::kNoError;
}
