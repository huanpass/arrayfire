#include <string>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <af/dim4.hpp>
#include <limits>

typedef unsigned char uchar;

template<typename inType, typename outType, typename FileElementType>
void readTests(const std::string &FileName, std::vector<af::dim4> &inputDims,
                std::vector<std::vector<inType>>  &testInputs,
                std::vector<std::vector<outType>> &testOutputs)
{
    using std::vector;

    std::ifstream testFile(FileName);
    if(testFile.good()) {
        unsigned inputCount;
        testFile >> inputCount;
        for(unsigned i=0; i<inputCount; i++) {
            af::dim4 temp(1);
            testFile >> temp;
            inputDims.push_back(temp);
        }

        unsigned testCount;
        testFile >> testCount;
        testOutputs.resize(testCount);

        vector<unsigned> testSizes(testCount);
        for(unsigned i = 0; i < testCount; i++) {
            testFile >> testSizes[i];
        }

        testInputs.resize(inputCount,vector<inType>(0));
        for(unsigned k=0; k<inputCount; k++) {
            unsigned nElems = inputDims[k].elements();
            testInputs[k].resize(nElems);
            FileElementType tmp;
            for(unsigned i = 0; i < nElems; i++) {
                testFile >> tmp;
                testInputs[k][i] = tmp;
            }
        }

        testOutputs.resize(testCount, vector<outType>(0));
        for(unsigned i = 0; i < testCount; i++) {
            testOutputs[i].resize(testSizes[i]);
            FileElementType tmp;
            for(unsigned j = 0; j < testSizes[i]; j++) {
                testFile >> tmp;
                testOutputs[i][j] = tmp;
            }
        }

    }
    else {
        FAIL() << "TEST FILE NOT FOUND";
    }
}

void readImageTests(const std::string        &pFileName,
                    std::vector<af::dim4>    &pInputDims,
                    std::vector<std::string> &pTestInputs,
                    std::vector<dim_type>    &pTestOutSizes,
                    std::vector<std::string> &pTestOutputs)
{
    using std::vector;

    std::ifstream testFile(pFileName);
    if(testFile.good()) {
        unsigned inputCount;
        testFile >> inputCount;
        for(unsigned i=0; i<inputCount; i++) {
            af::dim4 temp(1);
            testFile >> temp;
            pInputDims.push_back(temp);
        }

        unsigned testCount;
        testFile >> testCount;
        pTestOutputs.resize(testCount);

        pTestOutSizes.resize(testCount);
        for(unsigned i = 0; i < testCount; i++) {
            testFile >> pTestOutSizes[i];
        }

        pTestInputs.resize(inputCount, "");
        for(unsigned k=0; k<inputCount; k++) {
            std::string temp = "";
            while(std::getline(testFile, temp)) {
                if (temp!="")
                    break;
            }
            if (temp=="")
                throw std::runtime_error("Test file might not be per format, please check.");
            pTestInputs[k] = temp;
        }

        pTestOutputs.resize(testCount, "");
        for(unsigned i = 0; i < testCount; i++) {
            std::string temp = "";
            while(std::getline(testFile, temp)) {
                if (temp!="")
                    break;
            }
            if (temp=="")
                throw std::runtime_error("Test file might not be per format, please check.");
            pTestOutputs[i] = temp;
        }
    }
    else {
        FAIL() << "TEST FILE NOT FOUND";
    }
}

/**
 * Below is not a pair wise comparition method, rather
 * it computes the accumulated error of the computed
 * output and gold output.
 *
 * The cut off is decided based on root mean square
 * deviation from cpu result
 *
 * For images, the maximum possible error will happen if all
 * the observed values are zeros and all the predicted values
 * are 255's. In such case, the value of NRMSD will be 1.0
 * Similarly, we can deduce that 0.0 will be the minimum
 * value of NRMSD. Hence, the range of RMSD is [0,255] for image inputs.
 */
template<typename T>
bool compareArraysRMSD(dim_type data_size, T *gold, T *data, float tolerance)
{
    float accum = 0.0f;
    T minion    = std::numeric_limits<T>::lowest();
    T maxion    = std::numeric_limits<T>::max();
    for(dim_type i=0;i<data_size;i++)
    {
        float diff = std::fabs(gold[i]-data[i]) > 1.0e-4 ? gold[i]-data[i] : 0.0f;
        accum  += std::pow(diff,2.0f);
        maxion  = std::max(maxion, data[i]);
        minion  = std::min(minion, data[i]);
    }
    accum      /= data_size;
    float NRMSD = std::sqrt(accum)/(float)(maxion-minion);

    if (NRMSD > tolerance)
        return false;

    return true;
}