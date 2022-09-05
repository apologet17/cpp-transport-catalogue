#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cmath>

#include "transport_catalogue.h"
#include "json_reader.h"

using namespace std::string_literals;
using namespace catalogue_core;
using namespace renderer;
using namespace transport_catalogue;

void Test();

int main()
{
    Test();
/*
    TransportCatalogue cat;
    RequestHandler rh(cat); 
    InputReader ir(rh);


     ir.AdditionToCatalogue(std::cin);  
     ir.QueryProcessing(std::cin, std::cout);
*/
}



void TestGetInformationFromFileJSONReader() {
    {
        TransportCatalogue cat;
        MapRenderer mr;
        RequestHandler rh(cat);
        JSONReader jr(rh, mr);

        std::ifstream in("s10_final_opentest_3.json");
      
     //   std::ifstream etalon("json_output.txt");
        std::ofstream out("result.txt");

        jr.QueryProcessing(in, out);
       // std::ifstream outf("resultC.txt");
/*
        std::string s_etalon, s_out;
        while (!etalon.eof() && !outf.eof()) {
            getline(etalon, s_etalon);
            getline(outf, s_out);
            assert(s_etalon == s_out);
        }*/
    }
    
}

void Test() {

    TestGetInformationFromFileJSONReader();
}