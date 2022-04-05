
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{

    std::string             sInputFilePath,                 //!< file paths
        sOutputFilePath;

    static const int            kBlockSize = 1024;
    long long                   iNumFrames = kBlockSize;
    int                         iNumChannels;

    float                       fModFrequencyInHz;
    float                       fModWidthInSec;

    clock_t                     time = 0;

    float** ppfInputAudio = 0;
    float** ppfOutputAudio = 0;

    CAudioFileIf* phAudioFile = 0;
    CAudioFileIf* phAudioOutputFile = 0;

    CAudioFileIf::FileSpec_t    stFileSpec;

    CVibrato* pCVibrato = 0;

    showClInfo();


    // command line args
    if (argc < 5)
    {
        cout << "Incorrect number of arguments!" << endl;
        return -1;
    }
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];
    fModFrequencyInHz = atof(argv[3]);
    fModWidthInSec = atof(argv[4]);

    ///////////////////////////////////////////////////////////////////////////
    CAudioFileIf::create(phAudioFile);
    CAudioFileIf::create(phAudioOutputFile);

    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);
    phAudioOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    iNumChannels = stFileSpec.iNumChannels;
    if (!phAudioFile->isOpen())
    {
        cout << "Input file open error!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    }
    else if (!phAudioOutputFile->isOpen())
    {
        cout << "Output file cannot be initialized!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    }
    ////////////////////////////////////////////////////////////////////////////
    CVibrato::create(pCVibrato);
    pCVibrato->init(fModWidthInSec, stFileSpec.fSampleRateInHz, iNumChannels);

    // allocate memory
    ppfInputAudio = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfInputAudio[i] = new float[kBlockSize];

    ppfOutputAudio = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfOutputAudio[i] = new float[kBlockSize];

    // Set parameters of vibrato
    pCVibrato->setParam(CVibrato::kParamModFreqInHz, fModFrequencyInHz);
    pCVibrato->setParam(CVibrato::kParamModWidthInS, fModWidthInSec);

    // processing
    while (!phAudioFile->isEof())
    {
        phAudioFile->readData(ppfInputAudio, iNumFrames);
        pCVibrato->process(ppfInputAudio, ppfOutputAudio, iNumFrames);
        phAudioOutputFile->writeData(ppfOutputAudio, iNumFrames);
    }
    phAudioFile->getFileSpec(stFileSpec);


    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioOutputFile);
    CVibrato::destroy(pCVibrato);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfInputAudio[i];
        delete[] ppfOutputAudio[i];
    }
    delete[] ppfInputAudio;
    delete[] ppfOutputAudio;
    ppfInputAudio = 0;
    ppfOutputAudio = 0;

    // all done
    return 0;

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;

    return;
}

