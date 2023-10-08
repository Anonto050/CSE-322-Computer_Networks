#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include <windows.h>

using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void resetColor()
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void printRed(string s)
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    std::cout << s;

    // reset to default color
    resetColor();
}

void printGreen(string s)
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
    std::cout << s;

    // reset to default color
    resetColor();
}

void printYellow(string s)
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
    std::cout << s;

    // reset to default color
    resetColor();
}

void printCyan(string s)
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << s;

    // reset to default color
    resetColor();
}

#endif // !COLOR_H