
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];

    cout << sInputFilePath << endl;
    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    phAudioFile->create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath);
    if (!hOutputFile.is_open()) {
        cout << "Text File Read Error." << endl;
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory

    long long int numFrames = kBlockSize;
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfAudioData[i] = new float[kBlockSize];
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (numFrames == kBlockSize) {
        phAudioFile->readData(ppfAudioData, numFrames);
        for (int i = 0; i < numFrames; i++) {
            for (int j = 0; j < stFileSpec.iNumChannels; j++) {
                if (j != stFileSpec.iNumChannels - 1) {
                    hOutputFile << ppfAudioData[j][i] << " ";
                }
                else {
                    hOutputFile << ppfAudioData[j][i] << endl;
                }
            }

        }
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    phAudioFile->destroy(phAudioFile);
    hOutputFile.close();
    delete[] ppfAudioData;
    // all done
    return 0;

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

