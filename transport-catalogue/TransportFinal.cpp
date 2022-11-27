#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cmath>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "json_reader.h"

using namespace std::string_literals;
using namespace catalogue_core;
using namespace renderer;
using namespace router;
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



void Test_make_base() {
    {
        TransportCatalogue cat;
        MapRenderer mr;
        RequestHandler rh(cat, mr);
        JSONReader jr(rh, mr);

        std::ifstream in("s14_3_opentest_3_make_base.json");



        jr.MakeBaseProcessing(in, std::cout);


    }

}

void Test_process_requests() {
    {
        TransportCatalogue cat;
        MapRenderer mr;
        RequestHandler rh(cat, mr);
        JSONReader jr(rh, mr);

        std::ifstream in("s14_3_opentest_3_process_requests.json");

        std::ofstream out("result.txt");

        jr.ProcessRequests(in, out);

    }
}

void Test() {

    Test_make_base();
    Test_process_requests();
}