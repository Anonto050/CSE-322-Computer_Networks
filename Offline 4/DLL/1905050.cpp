#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <windows.h>
#include <bitset>

#include "Color.h"

#define BYTE_SIZE 8

using namespace std;

bool isCheckBit(int n)
{
    // check if n is a power of 2
    return (n & (n - 1)) == 0;
}

// Error detection and correction in data link layer using Hamming code and Cyclic Redundancy Check (CRC)

int main()
{

    string data;
    cout << "enter data string: ";
    getline(cin, data);

    // Number of data bytes in a row
    int m;
    cout << "enter number of data bytes in a row <m> : ";
    cin >> m;

    // Probability of each bit being toggled during transmission
    double p;
    cout << "enter probability <p> : ";
    cin >> p;

    // Generator polynomial
    string genPoly;
    cout << "enter generator polynomial : ";
    cin >> genPoly;

    // Task 1 : If the size of the data string is not a multiple of m, append the padding character (~) to the data string accordingly.
    // Print the updated data string

    while (data.length() % m != 0)
    {
        data += "~";
    }

    cout << endl;
    cout << "data string after padding: " << data << endl;

    // Task 2 : Divide the data string into l/m rows, where l is the length of the data string.
    // Print the rows of data string
    cout << endl;
    cout << "data block <ascii code of m characters per row> : " << endl;

    int l = data.length();
    int n = l / m;
    vector<int> dataBlock[n];

    int row = -1;

    for (int i = 0; i < l; i++)
    {
        if (i % m == 0)
        {
            row++;
        }

        char ch = data[i];
        int bitPosition = 1; // mask for checking each bit in a character

        vector<int> bits;

        for (int j = 0; j < BYTE_SIZE; j++)
        {
            int bit = (ch & bitPosition) ? 1 : 0;
            bits.push_back(bit);
            bitPosition <<= 1;
        }

        reverse(bits.begin(), bits.end());

        for (int j = 0; j < BYTE_SIZE; j++)
        {
            dataBlock[row].push_back(bits[j]);
        }
    }

    // Print the data block

    cout << endl;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < dataBlock[i].size(); j++)
        {
            cout << dataBlock[i][j];
        }
        cout << endl;
    }

    // Task 3 : Add check bits to correct at most one-bit error in each row of the data block using Hamming code.
    cout << endl;
    cout << "data block after adding check bits : " << endl;

    int r = 0;
    while (pow(2, r) < m * 8 + r + 1)
    {
        r++;
    }

    int bitsTotal = m * 8 + r;

    for (int i = 0; i < n; i++)
    {

        int checkBit = 0;
        for (int j = 0; j < r; j++)
        {
            checkBit = pow(2, j);
            dataBlock[i].insert(dataBlock[i].begin() + checkBit - 1, 0);
        }

        for (int k = 0; k < r; k++)
        {
            int checkBitPosition = 1;
            checkBitPosition <<= k;

            int parity = 0;

            for (int j = 0; j < dataBlock[i].size(); j++)
            {
                if ((j + 1) & checkBitPosition)
                {
                    parity ^= dataBlock[i][j];
                }
            }

            dataBlock[i][checkBitPosition - 1] = parity;
        }
    }

    // Print the updated data block and show the check bits in green color
    cout << endl;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < dataBlock[i].size(); j++)
        {
            if (isCheckBit(j + 1))
            {
                printGreen(to_string(dataBlock[i][j]));
            }
            else
            {
                cout << dataBlock[i][j];
            }
        }
        cout << endl;
    }

    // Task 4 : Serialize the above data block in column-major manner. Print the serialized data bits.

    cout << endl;
    cout << "data bits after column-wise serialization : " << endl;

    vector<int> dataSerialized;

    for (int i = 0; i < bitsTotal; i++)
    {
        for (int j = 0; j < n; j++)
        {
            dataSerialized.push_back(dataBlock[j][i]);
        }
    }

    cout << endl;
    for (int i = 0; i < dataSerialized.size(); i++)
    {
        cout << dataSerialized[i];
    }
    cout << endl;

    // Task 5 : Compute the CRC checksum of the above bit stream using the generator polynomial. Append the checksum to the bit stream. This is the frame to be transmitted.
    // Print the frame to be transmitted.

    cout << endl;
    cout << "data bits after appending CRC checksum : <sent frame>" << endl;

    int genPolyLength = genPoly.length();
    vector<int> saveDataSerialized = dataSerialized;

    for (int i = 0; i < genPolyLength - 1; i++)
    {
        dataSerialized.push_back(0);
    }

    for (int i = 0; i <= dataSerialized.size() - genPolyLength; i++)
    {
        if (dataSerialized[i] == 1)
        {
            for (int j = 0; j < genPolyLength; j++)
            {
                dataSerialized[i + j] ^= (genPoly[j] - '0');
            }
        }
    }

    vector<int> remainder;

    for (int i = saveDataSerialized.size(); i < dataSerialized.size(); i++)
    {
        remainder.push_back(dataSerialized[i]);
    }

    dataSerialized = saveDataSerialized;

    for (int i = 0; i < remainder.size(); i++)
    {
        dataSerialized.push_back(remainder[i]);
    }

    // Print the frame to be transmitted. Show the CRC checksum bits in cyan color
    cout << endl;

    for (int i = 0; i < dataSerialized.size(); i++)
    {
        if (i >= saveDataSerialized.size())
        {
            printCyan(to_string(dataSerialized[i]));
        }
        else
        {
            cout << dataSerialized[i];
        }
    }

    cout << endl;
    cout << endl;

    // Task 6 : Simulate the physical transmission by toggling each bit of the stream with a probability of p.
    // Print the received frame.

    cout << "received frame : " << endl;

    int sizeFrame = dataSerialized.size();
    vector<int> togglePositions;

    // generate random number from 0 to 1 with time dependent seed. Need to be a uniform distribution.

    uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    uniform_real_distribution<double> distribution(0.0, 1.0);

    for (int i = 0; i < sizeFrame; i++)
    {
        double random = distribution(generator);
        if (random < p)
        {
            togglePositions.push_back(i);
        }
    }

    for (int i = 0; i < togglePositions.size(); i++)
    {
        dataSerialized[togglePositions[i]] ^= 1;
    }

    // Print the received frame. Show the toggled bits in red color
    cout << endl;

    for (int i = 0; i < dataSerialized.size(); i++)
    {
        if (find(togglePositions.begin(), togglePositions.end(), i) != togglePositions.end())
        {
            printRed(to_string(dataSerialized[i]));
        }
        else
        {
            cout << dataSerialized[i];
        }
    }

    // Task 7 : Check if the received frame is correct or not using generator polynomial. If correct, print "No error detected". Otherwise, print "Error detected".

    bool isError = false;

    saveDataSerialized = dataSerialized;

    for (int i = 0; i <= dataSerialized.size() - genPolyLength; i++)
    {
        if (dataSerialized[i] == 1)
        {
            for (int j = 0; j < genPolyLength; j++)
            {
                dataSerialized[i + j] ^= (genPoly[j] - '0');
            }
        }
    }

    for (int i = 0; i < dataSerialized.size(); i++)
    {
        if (dataSerialized[i] == 1)
        {
            isError = true;
            break;
        }
    }

    cout << endl;
    cout << endl;
    cout << "result of CRC checksum matching :";

    if (isError)
    {
        cout << " error detected" << endl;
    }
    else
    {
        cout << " no error detected" << endl;
    }

    // Task 8 : Remove the CRC checksum bits from the data stream and de-serialize it into the data-block in a column-major fashion.

    cout << endl;
    cout << "data block after removing CRC checksum bits : " << endl;

    vector<int> dataBlock2[n];
    vector<int> receivedDataBlock[n];

    // Clear dataSerialized and copy saveDataSerialized to it
    dataSerialized.clear();
    for (int i = 0; i < n * (r + m * 8); i++)
    {
        dataSerialized.push_back(saveDataSerialized[i]);
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < bitsTotal; j++)
        {
            dataBlock2[i].push_back(dataSerialized[i + j * n]);
        }
    }

    // Print the data block after removing CRC checksum bits and show error bits in red color
    cout << endl;   

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < dataBlock2[i].size(); j++)
        {
            if (find(togglePositions.begin(), togglePositions.end(), i + j * n) != togglePositions.end())
            {
                printRed(to_string(dataBlock2[i][j]));
            }
            else
            {
                cout << dataBlock2[i][j];
            }
        }
        cout << endl;
    }

    // Task 9 : Correct the error in each row. Print the data block after correcting the errors and removing the check bits.

    cout << endl;
    cout << "data block after removing check bits : " << endl;

    vector<int> errorCheckBits;

    for (int i = 0; i < n; i++)
    {
        errorCheckBits.clear();

        for (int k = 0; k < r; k++)
        {
            int checkBitPosition = 1;
            checkBitPosition <<= k;

            int parity = 0;

            for (int j = 0; j < dataBlock2[i].size(); j++)
            {
                if ((j + 1) & checkBitPosition)
                {
                    parity ^= dataBlock2[i][j];
                }
            }

            errorCheckBits.push_back(parity);
        }

        int errorPosition = 0;

        for (int j = 0; j < errorCheckBits.size(); j++)
        {
            errorPosition += errorCheckBits[j] * pow(2, j);
        }

        errorPosition--;

        if (errorPosition < 0 || errorPosition >= dataBlock2[i].size())
        {
            continue;
        }

        dataBlock2[i][errorPosition] ^= 1;
    }

    // Remove check bits and print the data block after removing check bits

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < dataBlock2[i].size(); j++)
        {
            if (!isCheckBit(j + 1))
            {
                receivedDataBlock[i].push_back(dataBlock2[i][j]);
            }
        }
    }

    // Print the data block
    cout << endl;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < receivedDataBlock[i].size(); j++)
        {
            cout << receivedDataBlock[i][j];
        }
        cout << endl;
    }

    // Task 10 : Convert the data block into the original data string. Print the original data string.

    cout << endl;
    cout << "output frame : ";

    string outputData = "";
    vector<int> reverseBits;

    // convert the data block into the original data string like the opposite of task 2

    // for (int i = 0; i < n; i++)
    // {
    //   for(int j = 0; j < receivedDataBlock[i].size(); j=j+BYTE_SIZE)
    //   {
    //     bitset<BYTE_SIZE> bits;
    //     for(int k = 0; k < BYTE_SIZE; k++)
    //     {
    //       bits[7-k] = receivedDataBlock[i][j+k];
    //     }

    //     char ch = char(bits.to_ulong());
    //     outputData.push_back(ch);
    //   }
    // }

    for (int i = 0; i < n; i++)
    {
        int bitPosition = 1;
        int ch = 0;
        int bitShift = 7;

        for (int j = 0; j < receivedDataBlock[i].size(); j=j+BYTE_SIZE)
        {
            char result = 0;
            for (int k = 0; k < BYTE_SIZE; k++)
            {
                result |= ( receivedDataBlock[i][j + k] ? (1 << (7 - k)) : 0);
            }
            // cout << result << endl;
            outputData.push_back(result);

        }
    }


    cout << outputData << endl;
}