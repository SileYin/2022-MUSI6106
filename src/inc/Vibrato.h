#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "ErrorDef.h"
#include "Lfo.h"
#include "RingBuffer.h"


class CVibrato
{
public:

	/*! list of parameters for the vibrato */
	enum VibratoParam_t
	{
		kParamVibratoRange,         //!< time range of vibrato, in seconds
		kParamVibratoFrequency,        //!< frequency of vibrato in Hz

		kNumVibratoParams
	};

	/*! creates a new vibrato instance
	\param pCCombFilterIf pointer to the new class
	\return Error_t
	*/
	static Error_t create(CVibrato*& CVibrato);

	/*! destroys a vibrato instance
	\param pCCombFilterIf pointer to the class to be destroyed
	\return Error_t
	*/
	static Error_t destroy(CVibrato*& CVibrato);

	/*! initializes a vibrato instance
	\param fDelayTimeInS delay of the vibrato, in seconds
	\param fSampleRateInHz sample rate in Hz
	\param iLFOWavetableLength desired LFO wavetable length
	\return Error_t
	*/
    Error_t init(float fDelayTimeInS, float fSampleRateInHz, int iLFOWavetableLength = 4096);

    /*! resets the internal variables (requires new call of init)
    \return Error_t
    */
    Error_t reset();

    /*! sets a vibrato parameter
    \param eParam what parameter (see ::VibratoParam_t)
    \param fParamValue value of the parameter
    \return Error_t
    */
    Error_t setParam(VibratoParam_t eParam, float fParamValue);

    /*! return the value of the specified parameter
    \param eParam
    \return float
    */
    float   getParam(VibratoParam_t eParam) const;

    /*! processes one block of audio
    \param ppfInputBuffer input buffer [numChannels][iNumberOfFrames]
    \param ppfOutputBuffer output buffer [numChannels][iNumberOfFrames]
    \param iNumberOfFrames buffer length (per channel)
    \return Error_t
    */
    Error_t process(float* pfInputBuffer, float* pfOutputBuffer, int iNumberOfFrames);
protected:
	CVibrato();
	~CVibrato();
private:
	CWavetableLFO* m_pCLFO = 0;
	CRingBuffer<float>* m_pCRingBuff = 0;
	bool m_bIsInitialized;
	float m_fSampleRate;
	float m_fDelayTimeInS;
	float m_fVibratoRangeInS;
	float m_fVibratoFrequencyInHz;
};

#endif // __Vibrato_hdr__
