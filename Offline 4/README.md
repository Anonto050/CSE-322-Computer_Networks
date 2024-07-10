# Hamming Code and CRC Error Detection and Correction

This project implements a data transmission system using Hamming code for error correction and Cyclic Redundancy Check (CRC) for error detection. The system simulates data transmission, introduces errors probabilistically, and verifies the integrity of the received data.

## Table of Contents

- [Overview](#overview)
- [Getting Started](#getting-started)
- [How It Works](#how-it-works)
  - [Key Concepts](#key-concepts)
  - [Detailed Steps](#detailed-steps)
  - [Examples in the Code](#examples-in-the-code)
- [Files](#files)

## Overview

This project demonstrates error detection and correction in the data link layer using Hamming code and CRC. It involves the following tasks:
1. Padding the data string.
2. Dividing the data into rows.
3. Adding check bits using Hamming code.
4. Serializing data in column-major order.
5. Appending CRC checksum.
6. Simulating data transmission with bit toggling.
7. Verifying data integrity using CRC.
8. Removing CRC checksum and de-serializing data.
9. Correcting errors using Hamming code.
10. Converting the data block back to the original string.

## Getting Started

### Prerequisites

- A C++ compiler (GCC, MSVC, etc.)
- Windows operating system (for `windows.h` dependency)

### Building the Project

To build the project, compile the source code with your preferred C++ compiler.

```sh
g++ -o hamming_crc hamming_crc.cpp -std=c++11
```

### Running the Program

Run the compiled executable and follow the prompts to input data, row size, error probability, and generator polynomial.

```sh
./hamming_crc
```

## How It Works

### Key Concepts

- **Padding**: Ensures the data string's length is a multiple of the row size `m`.
- **Hamming Code**: Adds check bits to enable single-bit error correction.
- **CRC (Cyclic Redundancy Check)**: Computes a checksum to detect errors in the data stream.
- **Column-Major Serialization**: Transforms data into a column-major order for transmission.
- **Error Simulation**: Randomly toggles bits in the data stream based on a specified probability.

### Detailed Steps

1. **Padding the Data String**: The input data string is padded with '~' to make its length a multiple of `m`.

2. **Dividing the Data into Rows**: The padded data is divided into rows, where each row contains `m` characters. Each character is converted to its ASCII binary representation.

3. **Adding Check Bits (Hamming Code)**:
   - Calculate the number of check bits `r` required.
   - Insert check bits into the data block.
   - Calculate parity bits for error detection.

4. **Column-Major Serialization**: The data block is serialized in column-major order for transmission.

5. **Appending CRC Checksum**:
   - Calculate CRC checksum using the generator polynomial.
   - Append the checksum to the serialized data.

6. **Simulating Data Transmission**:
   - Introduce errors by toggling bits with a specified probability.
   - Print the received frame, highlighting the toggled bits in red.

7. **Verifying Data Integrity (CRC)**:
   - Recalculate CRC on the received frame.
   - Check for errors by verifying the checksum.

8. **Removing CRC Checksum**: Extract the original data block by removing the CRC checksum bits.

9. **Correcting Errors (Hamming Code)**:
   - Detect and correct single-bit errors in each row.
   - Remove check bits to retrieve the original data block.

10. **Converting to Original Data String**: Convert the corrected data block back to the original data string.

### Examples in the Code

#### Padding the Data String

```cpp
while (data.length() % m != 0)
{
    data += "~";
}
cout << "data string after padding: " << data << endl;
```

#### Adding Check Bits (Hamming Code)

```cpp
int r = 0;
while (pow(2, r) < m * 8 + r + 1)
{
    r++;
}

for (int i = 0; i < n; i++)
{
    for (int j = 0; j < r; j++)
    {
        int checkBit = pow(2, j);
        dataBlock[i].insert(dataBlock[i].begin() + checkBit - 1, 0);
    }

    for (int k = 0; k < r; k++)
    {
        int checkBitPosition = 1 << k;
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
```

#### Column-Major Serialization

```cpp
vector<int> dataSerialized;

for (int i = 0; i < bitsTotal; i++)
{
    for (int j = 0; j < n; j++)
    {
        dataSerialized.push_back(dataBlock[j][i]);
    }
}

for (int i = 0; i < dataSerialized.size(); i++)
{
    cout << dataSerialized[i];
}
cout << endl;
```

#### Appending CRC Checksum

```cpp
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
```

#### Simulating Data Transmission

```cpp
default_random_engine generator(chrono::steady_clock::now().time_since_epoch().count());
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
```

## Files

- `hamming_crc.cpp`: Main source code implementing the error detection and correction system.
- `Color.h`: Header file for color printing functions.

By following the steps and understanding the provided code, this project effectively demonstrates the application of Hamming code and CRC for error detection and correction in data transmission.
