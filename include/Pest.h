#pragma once

//#include "stdafx.h"
//#include <emil/EMIL.h>
#include <queue>
//#include <io.h>
//#include <string>
//#include <iostream>
//#include <GL/freeglut.h>
#include <iostream>
#include <sstream>
//#include <stdexcept>

using namespace std;

class Pest
{
    /*
    NOTE: the code reads last line of a file, the deletes it


    note that i'm keeping the full stepSize array
    because i need to check backward for the step
    size prior the last reversal.
    Moreover, in eventual successive implementation
    of a stopping rule I might need that info too.
    */

private:
    static int openInst;
    static int savedInst;
    static string label;

    int instNo;

    float upB, loB;// upper and lower bounds
    float w; // wald's constant
    float p; // target p
    int isLog;

    float lvl;
    float lvlLoB;

    int hits, trials; // k and n for binomial
    int revPos;

    vector <float> step;

    //count of successive steps
    int stpCnt;

    //number of steps taken (counter)
    int s;

public:
    Pest();
    Pest(float startingLevel, float targetP,
         float stepSize, float waldConstant,int isLogarithmic);
    Pest(const char* filename);
    ~Pest();

    int saveInstance(const char* filename);
    void addTrial(int hit);
    static const char* getLabel();
    float getTestLvl();
    int getHits(); // added by Janis 6/22
    int getTrials(); // added by Janis 6/22
};
