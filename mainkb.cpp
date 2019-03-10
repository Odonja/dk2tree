#include <iostream>
#include <fstream>
using namespace std;

#include "DKTree.cpp"
#include "MakeGraphFromFile.cpp"
#include "parameters.cpp"
#include "LargeGraphTest.cpp"

int main(int argc, char **argv) {
    if (argc != 5) {
        std::cout << "error: invalid number of arguments" << std::endl;
        std::cout << "expected: dk2tree inputfilename outputfilename posEdges negEdges" << std::endl;
        return 1;
    }
    ofstream myfile;
    myfile.open(argv[2], ios::app);
    myfile << std::endl << "running " << argv[0] << std::endl;
    myfile << "parametersettings k = " << k << " B = " << B+k*k << std::endl;
    myfile << "processing " << argv[1] << std::endl;

    auto tree = makeGraphFromFile(argv[1]);

    myfile << argv[1] << " has size: " << tree->memoryUsage() << std::endl;

    Timer timer;
    double average = 0;
    unsigned long counter = 0;

    ifstream negEdgesfile(argv[4]);
    unsigned long from, to;
    while (negEdgesfile >> from >> to) {
        timer.start();
        tree->addEdge(from, to);
        timer.stop();
        counter++;
        average += timer.read();
    }
    myfile << argv[1] << "average time to add an edge: " << average/counter << std::endl;
    negEdgesfile.close();

    average = 0;
    counter = 0;
    ifstream posEdgesfile(argv[3]);

    while (posEdgesfile >> from >> to) {
        timer.start();
        tree->reportEdge(from, to);
        timer.stop();
        counter++;
        average += timer.read();
    }
    myfile << argv[1] <<"average time to report 1 positive edge: " <<  average/counter << std::endl;
    posEdgesfile.close();
    myfile.close();

    delete tree;

    return 0;
}


