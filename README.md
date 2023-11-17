# cpp-transport-catalogue
This is a final project from Yandex Praktikum - transport catalogue;
This program is to solve problems with making a route between different stops and give me the experience of using modern C++ features. It can draw images of the final route (.svg).

## requirements: C++17, x86;

Еhe program requires json document on input. Document consists of 2 parts - base_requests and stat_requests. First part is to load data to the database, the second part is a part with different queries.
base_requests may consist of the queries of 2 types:
1) type is Bus
{
      "type": "Bus",
      "name": "114",
      "stops": ["Metro", "Bridge"],
      "is_roundtrip": false
}
2) type is Stop
{
      "type": "Stop",
      "name": "metro"
      "latitude": 43.587795,
      "longitude": 39.716901,
      "road_distances": {"bridge": 850}
}
when the database is fullfilled, it can process the stat_requests. We have 3 types of stat_requests:
1)Stop
"id": 1, "type": "Stop", "name": "Bridge" 
2)Bus
"id": 2, "type": "Bus", "name": "114"
3)Map
"id": 3, "type": "Map"

Stop request will return a json document, containing information about a certain stop.
Bus request will return a json document, containing information about a certain bus.
Map request will return a json document, containing .svg document (printed map)

# Example of the json document on input:

{
    "base_requests": [
      {
        "type": "Bus",
        "name": "114",
        "stops": ["metro", "bridge"],
        "is_roundtrip": false
      },
      {
        "type": "Stop",
        "name": "bridge",
        "latitude": 43.587795,
        "longitude": 39.716901,
        "road_distances": {"metro": 850}
      },
      {
        "type": "Stop",
        "name": "metro",
        "latitude": 43.581969,
        "longitude": 39.719848,
        "road_distances": {"bridge": 850}
      }
    ],
    "render_settings": {
      "width": 200,
      "height": 200,
      "padding": 30,
      "stop_radius": 5,
      "line_width": 14,
      "bus_label_font_size": 20,
      "bus_label_offset": [7, 15],
      "stop_label_font_size": 20,
      "stop_label_offset": [7, -3],
      "underlayer_color": [255,255,255,0.85],
      "underlayer_width": 3,
      "color_palette": ["green", [255,160,0],"red"]
    },
    "stat_requests": [
      { "id": 1, "type": "Map" },
      { "id": 2, "type": "Stop", "name": "bridge" },
      { "id": 3, "type": "Bus", "name": "114" }
    ]
  }

  # Example of the output:
  [
    {
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n</svg>",
        "request_id": 1
    },
    {
        "buses": [
            "114"
        ],
        "request_id": 2
    },
    {
        "curvature": 1.23199,
        "request_id": 3,
        "route_length": 1700,
        "stop_count": 3,
        "unique_stop_count": 2
    }
]
