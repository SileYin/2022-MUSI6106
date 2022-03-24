#include "CombFilter.h"

CCombFilterBase::CCombFilterBase(int iMaxDelayInSamples, int iNumChannels)
	:m_fGain(1)
{
	m_iMaxDelayLength = iMaxDelayInSamples;
	m_iDelayLength = m_iMaxDelayLength;
	m_iNumChannels = iNumChannels;
	pCRingBuff = new CRingBuffer<float>*[m_iNumChannels];
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		pCRingBuff[i] = new CRingBuffer<float>(m_iMaxDelayLength + 1);
	}
}


CCombFilterBase::~CCombFilterBase()
{
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		delete pCRingBuff[i];
	}
	delete [] pCRingBuff;
	pCRingBuff = 0;
}

Error_t CCombFilterBase::setDelayLength(int iDelayInSample)
{
	if (iDelayInSample > m_iMaxDelayLength || iDelayInSample < 0)
	{
		return Error_t::kNumErrors;
	}
	m_iDelayLength = iDelayInSample;
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		pCRingBuff[i]->reset();
		pCRingBuff[i]->setReadIdx(m_iMaxDelayLength + 1 - m_iDelayLength);
	}
	return Error_t::kNoError;
}

Error_t CCombFIR::combFilter(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames)
{
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		for (int j = 0; j < iNumberOfFrames; ++j)
		{
			pCRingBuff[i]->putPostInc(ppfInputBuffer[i][j]);
			ppfOutputBuffer[i][j] = ppfInputBuffer[i][j] + m_fGain * pCRingBuff[i]->getPostInc();
		}
	}
	return Error_t::kNoError;
}

Error_t CCombIIR::combFilter(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames)
{
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		for (int j = 0; j < iNumberOfFrames; ++j)
		{
			ppfOutputBuffer[i][j] = ppfInputBuffer[i][j] + m_fGain * pCRingBuff[i]->getPostInc();
			pCRingBuff[i]->putPostInc(ppfOutputBuffer[i][j]);
		}
	}
	return Error_t::kNoError;
}
