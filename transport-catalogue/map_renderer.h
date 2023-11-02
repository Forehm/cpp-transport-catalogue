#pragma once
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

inline const double EPSILON = 1e-6;


namespace map_renderer
{


    class SphereProjector {
    public:
        // points_begin è points_end çàäàþò íà÷àëî è êîíåö èíòåðâàëà ýëåìåíòîâ geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Åñëè òî÷êè ïîâåðõíîñòè ñôåðû íå çàäàíû, âû÷èñëÿòü íå÷åãî
            if (points_begin == points_end) {
                return;
            }

            // Íàõîäèì òî÷êè ñ ìèíèìàëüíîé è ìàêñèìàëüíîé äîëãîòîé
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Íàõîäèì òî÷êè ñ ìèíèìàëüíîé è ìàêñèìàëüíîé øèðîòîé
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Âû÷èñëÿåì êîýôôèöèåíò ìàñøòàáèðîâàíèÿ âäîëü êîîðäèíàòû x
            std::optional<double> width_zoom;
            if (!(std::abs(max_lon - min_lon_) < EPSILON)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Âû÷èñëÿåì êîýôôèöèåíò ìàñøòàáèðîâàíèÿ âäîëü êîîðäèíàòû y
            std::optional<double> height_zoom;
            if (!(std::abs(max_lat_ - min_lat) < EPSILON)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Êîýôôèöèåíòû ìàñøòàáèðîâàíèÿ ïî øèðèíå è âûñîòå íåíóëåâûå,
                // áåð¸ì ìèíèìàëüíûé èç íèõ
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Êîýôôèöèåíò ìàñøòàáèðîâàíèÿ ïî øèðèíå íåíóëåâîé, èñïîëüçóåì åãî
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Êîýôôèöèåíò ìàñøòàáèðîâàíèÿ ïî âûñîòå íåíóëåâîé, èñïîëüçóåì åãî
                zoom_coeff_ = *height_zoom;
            }
        }

        // Ïðîåöèðóåò øèðîòó è äîëãîòó â êîîðäèíàòû âíóòðè SVG-èçîáðàæåíèÿ
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;


    };

    struct MapVisualSettings
    {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0;
        size_t bus_label_font_size = 0;
        svg::Point bus_label_offset = { 0.0, 0.0 };
        size_t stop_label_font_size = 0;
        svg::Point stop_label_offset = { 0.0, 0.0 };
        svg::Color underlayer_color = "black";
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette;
    };

    class MapRenderer
    {
    public:

        MapRenderer(const MapVisualSettings& settings, const SphereProjector& projector);

        void AddRoadsToMap(Catalogue::TransportCatalogue& catalogue);

        void AddBusRoutesToMap(Catalogue::TransportCatalogue& catalogue);

        void AddStopsToMap(Catalogue::TransportCatalogue& catalogue);

        void AddStopNameToMap(Catalogue::TransportCatalogue& catalogue);

        void RenderMap(std::ostream& out);

    private:

        svg::Document doc_;
        SphereProjector projector_;
        MapVisualSettings settings_;
    };

    map_renderer::SphereProjector MakeProjector(Catalogue::TransportCatalogue& catalogue, double width, double height, double padding);



}