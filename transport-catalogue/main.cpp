#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cmath>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std::string_literals;
using namespace catalogue_core;
using namespace inputreader;
using namespace statreader;
using namespace transportcatalogue;

void Test();

int main()
{
    Test();
/*
    TransportCatalogue cat;
  
    InputReader ir(cat);
    StatReader sr(cat);

     ir.AdditionToCatalogue(std::cin);  
     sr.QueryProcessing(std::cin, std::cout);
*/
}

void TestStopBusInputProcessing() {
    const double ABS = 10e-6;
    std::istringstream data{ "Stop Rasskazovka: 55.632761, 37.333324\n"
                             "   Stop Rasskazovka: 55.632761, 37.333324\n" 
                             "Stop Rasskazovka  : 55.632761, 37.333324\n" 
                             "Stop Rasskazovka:   55.632761, 37.333324\n"
                             "Stop Rasskazovka: 55.632761 , 37.333324\n" 
                             "Stop Rasskazovka: 55.632761,   37.333324\n"
                             "Stop Rasskazovka: 55.632761, 37.333324    \n" };

    size_t i = 7;
    while (--i > 0) {
        Query q;
        data >> q;
        
        assert(abs(q.bus_stop.bus_stop.coordinates.lat- 55.632761) < ABS);
        assert(abs(q.bus_stop.bus_stop.coordinates.lng- 37.333324) < ABS);
        assert(q.bus_stop.bus_stop.name == "Rasskazovka"s);
    }
}

void TestRouteBusInputProcessing() {
    { //проверяем исключение пробелов между разделителями и элементами запроса
        std::istringstream data{ "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                                 "   Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                                 "Bus 750   : Tolstopaltsevo - Marushkino - Rasskazovka\n"
                                 "Bus 750:     Tolstopaltsevo - Marushkino - Rasskazovka\n"
                                 "Bus 750: Tolstopaltsevo     - Marushkino - Rasskazovka\n"
                                 "Bus   750: Tolstopaltsevo -     Marushkino - Rasskazovka\n"
                                 "Bus 750: Tolstopaltsevo - Marushkino    - Rasskazovka\n"
                                 "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka    \n" };

        size_t i = 7;
        while (--i > 0) {
            Query q;
            data >> q;

            assert(q.bus_route.name == "750"s);
            assert(q.bus_route.circular == false);
            assert(q.bus_route.stops[0] == "Tolstopaltsevo"s);
            assert(q.bus_route.stops[1] == "Marushkino"s);
            assert(q.bus_route.stops[2] == "Rasskazovka"s);
        }
    }
    {   //проверяем работу с многословными названиями
        std::istringstream data{ "Bus 750 60: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                                 "Bus 750: Tolstopaltsevo Zapadnoe - Marushkino - Rasskazovka\n"};

        Query q;

            data >> q;
            assert(q.bus_route.name == "750 60"s);
            assert(q.bus_route.circular == false);
            assert(q.bus_route.stops[0] == "Tolstopaltsevo"s);
            assert(q.bus_route.stops[1] == "Marushkino"s);
            assert(q.bus_route.stops[2] == "Rasskazovka"s);

            data >> q;
            assert(q.bus_route.name == "750"s);
            assert(q.bus_route.circular == false);
            assert(q.bus_route.stops[0] == "Tolstopaltsevo Zapadnoe"s);
            assert(q.bus_route.stops[1] == "Marushkino"s);
            assert(q.bus_route.stops[2] == "Rasskazovka"s);
    }
    {  //проверяем разбор кольцевого маршрута
        std::istringstream data{ "Bus 750: Tolstopaltsevo > Marushkino > Rasskazovka\n"};

        Query q;

        data >> q;
        assert(q.bus_route.name == "750"s);
        assert(q.bus_route.circular == true);
        assert(q.bus_route.stops[0] == "Tolstopaltsevo"s);
        assert(q.bus_route.stops[1] == "Marushkino"s);
        assert(q.bus_route.stops[2] == "Rasskazovka"s);
    }
}

void TestGetInformation() {
    std::istringstream data{ "13\n"
                                "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
                                "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n"
                                "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                                "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n"
                                "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n"
                                "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
                                "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
                                "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"
                                "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"
                                "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"
                                "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
                                "Stop Rossoshanskaya ulitsa : 55.595579, 37.605757\n"
                                "Stop Prazhskaya : 55.611678, 37.603831\n" };

    std::istringstream qe{ "6\n"
                            "Bus 256 \n"
                             "Bus 750\n"
                             "Bus 751\n"
                             "Stop Samara\n"
                             "Stop Prazhskaya\n"
                             "Stop Biryulyovo Zapadnoye\n" };

    std::ostringstream etalon{  "Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature\n"
                                "Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature\n"
                                "Bus 751: not found\n"
                                "Stop Samara: not found\n"
                                "Stop Prazhskaya: no buses\n"
                                "Stop Biryulyovo Zapadnoye: buses 256 828\n"};

    std::ostringstream out;

    TransportCatalogue cat;
    InputReader ir(cat);
    StatReader sr(cat);

    ir.AdditionToCatalogue(data);
    sr.QueryProcessing(qe, out);

    assert(out.str() == etalon.str());
}

void TestGetInformationFromFile() {
    {
        TransportCatalogue cat;
        InputReader ir(cat);
        StatReader sr(cat);

        std::ifstream in("tsC_case1_input_1.txt");
        std::ifstream in2("tsC_case1_input_2.txt");

        std::ifstream etalon("tsC_case1_output1.txt");
        std::ofstream out("resultC.txt");

        ir.AdditionToCatalogue(in);
        sr.QueryProcessing(in2, out);

        std::ifstream outf("resultC.txt");

        std::string s_etalon, s_out;
        while (!etalon.eof() && !outf.eof()) {
            getline(etalon, s_etalon);
            getline(outf, s_out);
            assert(s_etalon == s_out);
        }
    }
}

void Test() {
    TestStopBusInputProcessing();
    TestRouteBusInputProcessing();
    TestGetInformation();
    TestGetInformationFromFile();
}