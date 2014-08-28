//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "ConvolutionalCoderTest.h"
#include "BitVector.h"
#include "ShortBitVector.h"
#include <fstream>

namespace inet {
namespace physicallayer {

Define_Module(ConvolutionalCoderTest);

void ConvolutionalCoderTest::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        convCoderModule = getModuleFromPar<ConvolutionalCoder>(par("convolutionalCoderModule"), this);
        testFile = par("testFile");
        int numberOfRandomErrors = par("numberOfRandomErrors");
        identityTest(testFile, numberOfRandomErrors, "terminated");
    }
}

void ConvolutionalCoderTest::identityTest(const char *testFile, unsigned int numberOfRandomErrors, const char *mode) const
{
    srand(time(NULL));
    std::ifstream file(testFile);
    if (!file.is_open())
        throw cRuntimeError("Cannot open the test file: %s", testFile);
    std::string line;
    clock_t t1 = clock();
    std::string outputFile(testFile);
    outputFile += ".out";
    std::ofstream outFile(outputFile.c_str());
    while (file >> line)
    {
        BitVector input(line.c_str());
        std::cout << "Input: " << input << endl;
        outFile << "Input: " << input << endl;
        BitVector encoded;
        if (!strcmp(mode, "terminated"))
            encoded = convCoderModule->encode(input, true);
        else if (!strcmp(mode, "truncated"))
            encoded = convCoderModule->encode(input, false);
        else
            throw cRuntimeError("Unknown %s mode", mode);
        int numOfErrors = numberOfRandomErrors;
        std::cout << "Encoded before errors: " << encoded << endl;
        outFile << "Encoded before errors: " << encoded << endl;
        while (numOfErrors--)
        {
            int pos = rand() % encoded.getSize();
            encoded.toggleBit(pos);
        }
        std::cout << "Encoded after errors:  " << encoded << endl;
        outFile << "Encoded after errors:  " << encoded << endl;
        BitVector decoded = convCoderModule->decode(encoded, "terminated");
        std::cout << "Decoded: " << decoded << endl;
        outFile << "Decoded: " << decoded << endl;
        if (!strcmp(mode, "terminated"))
            input.appendBit(false, convCoderModule->getMemorySizeSum());
        if (input != decoded)
            throw cRuntimeError("Identity test failed");
    }
    clock_t t2 = clock();
    printf("Used %g CPU seconds\n",(t2 - t1) / (double)CLOCKS_PER_SEC);
}

} /* namespace physicallayer */
} /* namespace inet */
