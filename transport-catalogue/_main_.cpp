#include <iostream>
#include "geo.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std::string_literals;


map_renderer::SphereProjector MakeProjector(Catalogue::TransportCatalogue& catalogue, double width, double height, double padding)
{
	std::vector<geo::Coordinates> coordinates_vector;
	for (const std::string_view bus : catalogue.GetBusesList())
	{
		Catalogue::Detail::BusObject bus_obj = catalogue.GetBusInfo(bus);
		for (const auto& stop : catalogue.GetBusStops(bus.data()))
		{
			coordinates_vector.push_back(stop->coordinates);
		}
	}
	return map_renderer::SphereProjector{coordinates_vector.begin(), coordinates_vector.end(), width, height, padding};
}

void PrintRouteMap(std::ostream& out, const map_renderer::SphereProjector projector, const map_renderer::MapVisualSettings& settings, Catalogue::TransportCatalogue& catalogue)
{
	svg::Document doc;
	auto current_color = settings.color_palette.begin();
	for (const std::string_view bus : catalogue.GetBusesList())
	{
		Catalogue::Detail::BusObject bus_obj = catalogue.GetBusInfo(bus);
		svg::Polyline route;
		for (const auto& stop : catalogue.GetBusStops(bus.data()))
		{
			route.AddPoint(projector(stop->coordinates));
		}
		route.SetFillColor("none");
		route.SetStrokeColor(*current_color);
		if (current_color == settings.color_palette.end() - 1) { current_color = settings.color_palette.begin(); }
		else { ++current_color; }

		route.SetStrokeWidth(settings.line_width);
		route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		doc.Add(route);
	}
	doc.Render(out);
}

int main()
{
	Catalogue::TransportCatalogue catalogue;
	json::Document doc = json::Load(std::cin);
	json::Document answer = json::ExecuteJson(doc, catalogue);
	map_renderer::MapVisualSettings map_settings;
	json::SetMapVisualSettings(map_settings, doc.GetRoot().AsMap().at("render_settings"));
	map_renderer::SphereProjector projector = MakeProjector(catalogue, map_settings.width, map_settings.height, map_settings.padding);

	
	PrintRouteMap(std::cout, projector, map_settings, catalogue);



}