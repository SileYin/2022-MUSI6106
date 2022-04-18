
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"
#include "portaudio.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo();

typedef struct PortAudioUserData
{
    CVibrato* pCVibrato = 0;
    float** ppfRingBuffer = 0;
    float** ppfOutputBuffer = 0;
    bool bRingBufferIsFull = false;
    bool bReachedEndOfFile = false;
    bool bLastBufferLoaded = false;
    bool bLastBufferPlayed = false;
    long long iNumFrames;
    int iNumChannels;
} PortAudioUserData;

// This routine will be called by the PortAudio engine when audio is needed.
static int patestCallback ( const void *inputBuffer, 
                            void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
    PortAudioUserData *data = (PortAudioUserData*)userData;
    float *out = (float*)outputBuffer;
    
    if (data->bLastBufferLoaded)
    {
        data->bLastBufferPlayed = true;
        return 0;
    }
    

    if (data->bReachedEndOfFile)
    {
        data->bLastBufferLoaded = true;
        return 0;
    }


    if (!data->bRingBufferIsFull)
    {
        return 0;
    }

    data->pCVibrato->process(data->ppfRingBuffer, data->ppfOutputBuffer, data->iNumFrames);

    for (int i = 0; i < data->iNumFrames; i++)
    {
        for (int c = 0; c < data->iNumChannels; c++)
        {
            *out++ = data->ppfOutputBuffer[c][i];
        }
    }
    // fill in remainder of frames for empty audio output.
    for (int i = data->iNumFrames; i < framesPerBuffer; i++)
    {
        for (int c = 0; c < data->iNumChannels; c++)
        {
            *out++ = 0.f;
        }
    }

    data->bRingBufferIsFull = false;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{

    std::string sInputFilePath;                 //!< input file

    static const int kBlockSize = 1024;

    float fModFrequencyInHz;
    float fModWidthInSec;

    CAudioFileIf::FileSpec_t stFileSpec;
    
    PaStream *pPaStream;
    PaStreamParameters stStreamParameters;
    PaError err;
    PortAudioUserData stUserData;

    CAudioFileIf *phAudioFile;

    showClInfo();

    // command line args
    if (argc < 4)
    {
        fModWidthInSec = 0.f;
        if (argc < 3) 
        {
            fModFrequencyInHz = 0.f;
        }
        if (argc < 2) 
        {
            sInputFilePath = "input.wav";
        }
    } 
    else 
    {
        sInputFilePath = argv[1];
        fModFrequencyInHz = atof(argv[2]);
        fModWidthInSec = atof(argv[3]);
    }

    ///////////////////////////////////////////////////////////////////////////
    // audio file
    CAudioFileIf::create(phAudioFile);

    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);
    stUserData.iNumChannels = stFileSpec.iNumChannels;
    
    if (!phAudioFile->isOpen())
    {
        cout << "Input file open error!";

        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    
    stUserData.iNumFrames = kBlockSize;
    
    ////////////////////////////////////////////////////////////////////////////
    // vibrato instance
    CVibrato::create(stUserData.pCVibrato);
    stUserData.pCVibrato->init(fModWidthInSec, stFileSpec.fSampleRateInHz, stUserData.iNumChannels);

    // allocate memory
    stUserData.ppfRingBuffer = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        stUserData.ppfRingBuffer[i] = new float[kBlockSize];

    stUserData.ppfOutputBuffer = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        stUserData.ppfOutputBuffer[i] = new float[kBlockSize];

    // Set parameters of vibrato
    stUserData.pCVibrato->setParam(CVibrato::kParamModFreqInHz, fModFrequencyInHz);
    stUserData.pCVibrato->setParam(CVibrato::kParamModWidthInS, fModWidthInSec);
    
    ////////////////////////////////////////////////////////////////////////////
    // initialize PortAudio
    err = Pa_Initialize();
    if( err != paNoError ) 
    {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    }

    // setup audio output
    stStreamParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    stStreamParameters.channelCount = stFileSpec.iNumChannels;
    stStreamParameters.sampleFormat = paFloat32;
    stStreamParameters.suggestedLatency = Pa_GetDeviceInfo( stStreamParameters.device )->defaultLowOutputLatency;
    stStreamParameters.hostApiSpecificStreamInfo = NULL;
    
    // setup stream
    err = Pa_OpenStream(&pPaStream, 
                        NULL, 
                        &stStreamParameters, 
                        stFileSpec.fSampleRateInHz, 
                        kBlockSize, 
                        paClipOff, 
                        patestCallback, 
                        &stUserData);

    if( err != paNoError ) 
    {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    }

    err = Pa_StartStream( pPaStream );
    if( err != paNoError ) 
    {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    }
    

    while (!stUserData.bLastBufferPlayed)
    {
        if (stUserData.bRingBufferIsFull) continue;


        if (!phAudioFile->isEof())
        {
            phAudioFile->readData(stUserData.ppfRingBuffer, stUserData.iNumFrames);

            if (phAudioFile->isEof()) stUserData.bReachedEndOfFile = true;

            stUserData.bRingBufferIsFull = true;
        }
    }

    // Sleep for several seconds.
    // Pa_Sleep(0.5*1000);

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    err = Pa_StopStream( pPaStream );
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    }
    
    err = Pa_CloseStream( pPaStream );
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    }
    
    err = Pa_Terminate();
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    }

    CAudioFileIf::destroy(phAudioFile);
    CVibrato::destroy(stUserData.pCVibrato);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] stUserData.ppfRingBuffer[i];
        delete[] stUserData.ppfOutputBuffer[i];
    }
    delete[] stUserData.ppfRingBuffer;
    delete[] stUserData.ppfOutputBuffer;
    stUserData.ppfRingBuffer = 0;
    stUserData.ppfOutputBuffer = 0;


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

