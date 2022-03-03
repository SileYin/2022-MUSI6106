#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "ErrorDef.h"
#include "Lfo.h"
#include "RingBuffer.h"

/*! \brief A vibrato using wavetable LFO
* 
* This vibrato is single channel only, to keep the implementation simple, since
* having a multichannel vibrato but keep every parameter in each channel the same 
* don't have much value. If the user wants multichannel vibrato, they can just 
* have one instance of this class for each channels.
*  
* This interface is designed (100% copied) from the previous CCombFilterIf and
* AudioFileIf classes to keep the repository in style.
* The create and destroy function and the protected constructor and destructor
* are probably designed to provide extra memory management to the user. However, 
* I haven't looked into when the program throw an error and run out of the scope
* of an instance, would this give memory issue or not. Modern C++ has things like
* smart pointers to manage memories, don't know if it would be better to use that.
*/

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
    \param pfInputBuffer input buffer [iNumberOfFrames]
    \param pfOutputBuffer output buffer [iNumberOfFrames]
    \param iNumberOfFrames buffer length (per channel)
    \return Error_t
    */
    Error_t process(float* pfInputBuffer, float* pfOutputBuffer, int iNumberOfFrames);
protected:
    CVibrato();
    ~CVibrato();
private:
    CWavetableLFO* m_pCLFO = 0; //<! Wavetable LFO for the vibrato
    CRingBuffer<float>* m_pCRingBuff = 0; //<! Buffer for the vibrato
    bool m_bIsInitialized; //<! true if initialized
    float m_fSampleRate; //<! working sample rate for the vibrato
    float m_fDelayTimeInS; //<! Delay time for the vibrato in seconds
    float m_fVibratoRangeInS; //<! Mod range for the vibrato in seconds
    float m_fVibratoFrequencyInHz; //<! Rate for the vibrato in Hz
};

#endif // __Vibrato_hdr__
