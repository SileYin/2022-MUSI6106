#ifndef __Lfo_Hdr__
#define __Lfo_Hdr__

#include "RingBuffer.h"
#include <cmath>

class CWavetableLFO
{
public:
    /*! Creates intance for the wavetable LFO
    \param iWavetableLength wavetable length of the LFO
    \param iSampleRate sample rate for the LFO
    \param fFrequency frequency for the LFO in Hz
    \param fAmplitude amplitude for the LFO
    */
    CWavetableLFO(int iWavetableLength, int iSampleRate, float fFrequency, float fAmplitude):
        m_iSampleRate(iSampleRate),
        m_dReadIdx(0)
    {
        double const l_dPi = 3.14159265358979323846;
        assert(iWavetableLength > 0);
        pCWavetableRingBuff = new CRingBuffer<float>(iWavetableLength);
        for (int i = 0; i < iWavetableLength; i++)
        {
            pCWavetableRingBuff->putPostInc(sin(2 * l_dPi * i / iWavetableLength));
        }
        m_fAmplitude = fAmplitude; 
        m_dReadInc = static_cast<double>(pCWavetableRingBuff->getLength()) / static_cast<double>(m_iSampleRate) * fFrequency;
    }

    ~CWavetableLFO()
    {
        delete pCWavetableRingBuff;
        pCWavetableRingBuff = 0;
    }

    /*! Set the phase for the read head
    \param fPhaseInDegrees phase for the read head in degrees
    */
    void setReadPhase(float fPhaseInDegrees)
    {
        while (fPhaseInDegrees >= 360)
            fPhaseInDegrees -= 360;
        while (fPhaseInDegrees < 360)
            fPhaseInDegrees += 360;
        m_dReadIdx = fPhaseInDegrees / 360 * static_cast<double>(pCWavetableRingBuff->getLength());
    }

    /*! Get the wavetable length of the LFO 
    \return int
    */
    int getWavetableLength() const
    {
        return pCWavetableRingBuff->getLength();
    }

    /* Set the frequency of the LFO, read head fall back to origin at default
    \param fFrequency frequency for the LFO in Hz
    \param fPhaseInDegrees phase for the read head in degrees
    */
    void setLFOFrequency(float fFrequency, float fPhaseInDegrees = (0.f))
    {
        while (fPhaseInDegrees >= 360)
            fPhaseInDegrees -= 360;
        while (fPhaseInDegrees < 360)
            fPhaseInDegrees += 360;
        m_dReadInc = static_cast<double>(pCWavetableRingBuff->getLength()) / static_cast<double>(m_iSampleRate) * fFrequency;
        setReadPhase(fPhaseInDegrees);
    }
    
    /*! Get the current frequency of the LFO 
    \return float
    */
    float getLFOFrequency() const
    {
        return m_dReadInc * static_cast<float>(m_iSampleRate) / static_cast<float>(pCWavetableRingBuff->getLength());
    }
    
    /* Set the amplitude for the LFO
    \param fAmplitude amplitude for the LFO
    */
    void setLFOAmplitude(float fAmplitude)
    {
        m_fAmplitude = fAmplitude;
    }

    /*! Get the current amplitude of the LFO
    \return float
    */
    float getLFOAmplitude() const
    {
        return m_fAmplitude;
    }

    /* Get LFO output value, post by increment
    \return float
    */
    float getPostInc()
    {
        float l_fCurrentValue;
        l_fCurrentValue = m_fAmplitude * pCWavetableRingBuff->get(m_dReadIdx);
        m_dReadIdx += m_dReadInc;
        while (m_dReadIdx > pCWavetableRingBuff->getLength())
        {
            m_dReadIdx = m_dReadIdx - pCWavetableRingBuff->getLength();
        }
        return l_fCurrentValue;
    }
private:
    CRingBuffer<float>* pCWavetableRingBuff = 0; //<! Using ringbuffer as wavetable for easier fractional index value read
    int m_iSampleRate; //<! working sample rate for the LFO
    double m_dReadIdx; //<! read head of the LFO, using double for better precision
    float m_fAmplitude; //<! amplitude for the LFO
    double m_dReadInc; //<! read increment value for the LFO, using double for better precision
};



#endif // !__Lfo_Hdr__
