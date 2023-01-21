# Transport router

An intermediate project written on C++ during Yandex Praktikum course

# Description

The Transport Router is a back-end application which helps to calculate an optimal route by public transport. Includes transport database:

For buses: name, type, stops

For stops: name, gps coord, list of distances to other stops (real length)

Based on these data the app can show route statistics like:

Request:
```
{
    "stat_requests": [
        {
            "id": 1,
            "name": "289",
            "type": "Bus"
        }
    ]
}
```
Answer:
```
[
    {
        "curvature": 1.63414,
        "request_id": 1,
        "route_length": 24490,
        "stop_count": 7,
        "unique_stop_count": 4
    }
]
```
All saved bus routes can be visualized in SVG format by request:
```
{
    "stat_requests": [
        {
            "id": 1,
            "type": "Map"
        }
    ]
}
```
An answer is a text representation of svg file:
```
[
    {
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" ... "
        ,
        "request_id": 1"
    }
]
```
# Used language features
OOP, templates, patterns, method chaining, std algorithms, JSON, SVG, graphs.

# Build
CMakeLists.txt file is included for fast build with CMAKE. Only STL library is used.
