
//#include "stdafx.h"
//#include "ExperimentBody.h"
//#include "WholeFieldNoise.h"
//#include "ExFun.h"
//#include <string>
#include <cstdlib>

//#include <stdio.h>
#include <time.h>
//#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include "Pest.h"
#include <math.h>
#include <cstring>
//#include <sys/stat.h>


/*
It's important to call the different PEST objects in the
same order when saving and loading.
*/

/*
June 16, 2017 - Janis modified this version to have a lower bound on the level and only takes integer value steps (when the start value is a power of 2
*/

using namespace std;


// static variables must be initialized
// outside the class
int Pest::openInst=0;
int Pest::savedInst=0;
string Pest::label="Not Initialized";

Pest::Pest() {

}

// constructor, builds an object from scratch
Pest::Pest(float startingLevel, float targetP,
           float stepSize, float waldConstant,
           int isLogarithmic):
        w(waldConstant),
        p(targetP),
        lvl(startingLevel),
        isLog(isLogarithmic)
{
    if ( !openInst ){
        savedInst=0;
        time_t rawTime;
        struct tm * timeInfo;
        time( &rawTime );
        timeInfo = localtime( &rawTime );
        label = asctime( timeInfo );
    }

    openInst++;
    instNo=openInst;
    step.push_back( stepSize );
    trials=0;
    hits=0;
    stpCnt = 0;
    lvlLoB = 1; // pest level must be positive integer
    s = 0;
    // 6/4/15 Ruei Wu changed the initialization of revPos from 0 to 2. Originally the value was set by Marco Boi at 0.
    revPos=2;
}

// constructor, load saved object
Pest::Pest(const char* filepath)
{
    // reset saved-object count
    if ( !openInst ) savedInst=0;

    // input file name
    string pathStg( filepath );
    string inFileName = pathStg + "/pest.txt";

    // open file for input
    ifstream ifile(inFileName.c_str());

    // get the first line and set up label, if empty
    getline(ifile,label);
    label.append("\n");

    // continue reading out the file
    string line;
    while ( ifile.peek() != EOF )
    {
        getline( ifile,line );
        char* token;
        char* cLine = new char[line.length()+1];
        strcpy(cLine, line.c_str());
        token = strtok(cLine,"\t");
        if ( atoi(token) == 1+openInst )
        {
            openInst++;
            instNo = openInst; // first instance is "1"
            //moves the tokenizer one pos
            token = strtok(NULL,"\t");
            int pos=0;
            while (token != NULL)
            {
                switch ( pos )
                {
                    case 0: w = atof(token); break;
                    case 1: p = atof(token); break;
                    case 2: lvl = atof(token); break;
                    case 3: trials = atoi(token); break;
                    case 4: hits = atoi(token); break;
                    case 5: stpCnt = atoi(token); break;
                    case 6: s = atoi(token); break;
                    case 7: revPos = atoi(token); break;
                    case 8: isLog= atoi(token); break;

                        // retrieves step array
                    default: step.push_back( atof(token) ); break;
                }
                token = strtok(NULL,"\t");
                pos++;
            }
            break;
        }
        delete [] cLine;
    }
    ifile.close();
}

Pest::~Pest()
{
    openInst--;
}

int Pest::saveInstance(const char* filepath)
{
    // file path and output-file name
    string pathStg( filepath );
    string outFileName = pathStg + "/pest.txt";

    // creates a backup file
    if ( !savedInst )
    {
        // check if file exists
        ifstream ifile( outFileName.c_str(), fstream::binary );
        if ( ifile ) //false if file exists
        {
            // create a backup file
            string bakFileName = pathStg + "/pest.back";
            ofstream ofile( bakFileName.c_str(),
                            fstream::trunc | fstream::out | fstream::binary);
            ofile << ifile.rdbuf();
            ofile.close();
            ifile.close();
        }
    }

    ofstream ofile;
    // initialize a new save file
    if ( !savedInst )
    {
        ofile.open( outFileName.c_str() ,
                    fstream::trunc | fstream::out);
        ofile << label;
    }
        // appends instance to existing files
    else
    {
        ofile.open( outFileName.c_str() ,
                    fstream::app | fstream::out);
    }

    ofile << instNo << "\t" ;
    ofile << w << "\t" ;
    ofile << p << "\t" ;
    ofile << lvl << "\t";
    ofile << trials <<"\t" ;
    ofile << hits <<"\t" ;
    ofile << stpCnt << "\t" ;
    ofile << s <<"\t" ;
    ofile << revPos <<"\t" ;
    ofile << isLog <<"\t" ;

    // stores step array
    for( int i=0; i<step.size(); i++ )
        ofile << step[i] << "\t";

    ofile << "\n";
    ofile.close();
    savedInst++;
    // returns 1 if all open instances were saved
    return(openInst==savedInst);
}

const char* Pest::getLabel()
{
    return( label.c_str());
}

float Pest::getTestLvl()
{
    return( lvl );
}

int Pest::getHits()
{
    return (hits);
}

int Pest::getTrials()
{
    return (trials);
}

void Pest::addTrial(int hit)
{
    hits += hit;
    trials ++;

    //upper and lower bounds
    upB =  p * float(trials) + w ;
    loB =  p * float(trials) - w ;

    // step direction
    int dir = - int(hits >= upB) + int(hits <= loB) ;

    // if "dir" is nonzero, we're taking a step
    if ( dir )
    {

        s++; // increase step count
        hits = 0;
        trials = 0;

        // by default, step size is the same
        step.push_back( step[s-1] );

        //check if a reversal occurred
        if ( (stpCnt * dir) < 0 )
        {
            revPos = s;
            stpCnt = dir;
            if(isLog)
                step[s] = sqrt( step[s-1] );
            else
            if (step[s] > 1)
                step[s] = .5 * step[s-1];
        }
        else
        {
            stpCnt += dir;
            // the following is verified if:
            // 1) we're taking a 3rd step in one direction
            //	and last reversal was NOT preceeded by
            //	a doubling in stepSize
            // 2) we're taking a 4th or successive step
            //	in one direction


            // this is true if stepSize has been doubled
            // before last reversal
            bool isDoubled = step[revPos-1] > step[revPos-2];

            if (  ( abs(stpCnt) == 3 && revPos && !isDoubled ) ||
                  ( abs(stpCnt) > 3 )  )
            {
                if( isLog )
                    step[s] = pow(step[s-1],2);
                else
                    step[s] = 2 * step[s-1];
            }
        }
        if (isLog) {
            lvl *= pow(step[s], dir);
        }
        else {
            lvl += dir * step[s];
        }
        lvl = max(lvl, lvlLoB);
    }
}

