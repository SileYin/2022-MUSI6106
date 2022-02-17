#ifndef __CombFilter_hdr__
#define __CombFilter_hdr__
#include "ErrorDef.h"
#include "RingBuffer.h"

class CCombFilterBase
{
public:
	CCombFilterBase(int, int);
	~CCombFilterBase();
	Error_t CCombFilterBase::setDelayLength(int iDelayInSample);
	Error_t setGain(float g)
	{
		m_fGain = g;
		return Error_t::kNoError;
	}
	int getDelayLength() const
	{
		return m_iDelayLength;
	}
	float getGainValue() const
	{
		return m_fGain;
	}
	virtual Error_t combFilter(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) = 0;
protected:
	int m_iMaxDelayLength;
	int m_iDelayLength;
	float m_fGain;
	int m_iNumChannels;
	CRingBuffer<float>** pCRingBuff = 0;

};

class CCombFIR : public CCombFilterBase
{
public:
	using CCombFilterBase::CCombFilterBase;
	Error_t combFilter(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) override;
};

class CCombIIR : public CCombFilterBase
{
public:
	using CCombFilterBase::CCombFilterBase;
	Error_t combFilter(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) override;
};

#endif