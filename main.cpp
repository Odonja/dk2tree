#include <iostream>
#include <fstream>
using namespace std;

#include "DKTree.cpp"
#include "MakeGraphFromFile.cpp"
#include "parameters.cpp"

int main(int argc, char **argv) {
    if(argc != 3){
        std::cout << "error: invalid number of arguments" << std::endl;
        std::cout << "expected: dk2tree inputfilename outputfilename" << std::endl;
        return 1;
    }
    ofstream myfile;
    myfile.open (argv[2], ios::app);
    myfile << "running "<< argv[0] << std::endl;
    myfile << "parametersettings k = "<< k << " B = " << B << std::endl;
    myfile << "processing " <<argv[1] << std::endl;

    auto tree = makeGraphFromFile(argv[1]);

    myfile << argv[1] <<" has size: " <<  tree->memoryUsage() << std::endl ;
    myfile.close();

    delete tree;

    return 0;
}


