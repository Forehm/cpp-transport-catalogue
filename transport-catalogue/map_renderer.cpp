#include "map_renderer.h"
#include <string>
#include "transport_catalogue.h"

using namespace std::string_literals;


map_renderer::SphereProjector map_renderer::MakeProjector(Catalogue::TransportCatalogue& catalogue, double width, double height, double padding)

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
	return map_renderer::SphereProjector{ coordinates_vector.begin(), coordinates_vector.end(), width, height, padding };
}

map_renderer::MapRenderer::MapRenderer(const MapVisualSettings& settings, const SphereProjector& projector) : projector_(projector)
{
	this->settings_ = settings;
}

void map_renderer::MapRenderer::AddRoadsToMap(Catalogue::TransportCatalogue& catalogue)
{
	auto current_color = settings_.color_palette.begin();
	for (const std::string_view bus : catalogue.GetBusesList())
	{
		Catalogue::Detail::BusObject bus_object = catalogue.GetBusInfo(bus);
		svg::Polyline route;
		if (bus_object.stops == 0)
		{
			continue;
		}
		for (const auto& stop : catalogue.GetBusStops(bus.data()))
		{
			route.AddPoint(projector_(stop->coordinates));
		}

		route.SetFillColor("none");
		route.SetStrokeColor(*current_color);
		if (current_color == settings_.color_palette.end() - 1) { current_color = settings_.color_palette.begin(); }
		else { ++current_color; }

		route.SetStrokeWidth(settings_.line_width);
		route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		doc_.Add(route);
	}
}

void map_renderer::MapRenderer::AddBusRoutesToMap(Catalogue::TransportCatalogue& catalogue)
{
	auto current_color = settings_.color_palette.begin();
	for (const std::string_view bus : catalogue.GetBusesList())
	{

		Catalogue::Detail::BusObject bus_object = catalogue.GetBusInfo(bus.data());
		if (bus_object.stops == 0)
		{
			continue;
		}
		svg::Text bus_index;
		svg::Text underlayer;

		std::string bus_name(bus.begin(), bus.end());
		bus_index.SetPosition(projector_(catalogue.GetBusStops(bus.data()).front()->coordinates));
		if (current_color == settings_.color_palette.end()) { current_color = settings_.color_palette.begin(); }
		bus_index.SetFillColor(*current_color);

		bus_index.SetFontFamily("Verdana"s);
		bus_index.SetFontWeight("bold"s);
		bus_index.SetData(bus_name);
		bus_index.SetOffset(settings_.bus_label_offset);
		bus_index.SetFontSize(settings_.bus_label_font_size);


		++current_color;

		underlayer = bus_index;

		underlayer.SetStrokeWidth(settings_.underlayer_width);
		underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		underlayer.SetStrokeColor(settings_.underlayer_color);
		underlayer.SetFillColor(settings_.underlayer_color);

		doc_.Add(underlayer);
		doc_.Add(bus_index);

		if (!catalogue.IsBusRoundTrip(bus.data()) && (catalogue.GetBusStops(bus.data()).front()->name != catalogue.GetLastStopToBus(bus.data())))
		{
			svg::Text additional_index = bus_index;
			svg::Text additional_underlayer = underlayer;
			Catalogue::Detail::StopObject stop_object = catalogue.GetStopInfo(catalogue.GetLastStopToBus(bus_name));
			additional_index.SetPosition(projector_(stop_object.coordinates));
			additional_underlayer.SetPosition(projector_(stop_object.coordinates));
			doc_.Add(additional_underlayer);
			doc_.Add(additional_index);
		}
	}
}

void map_renderer::MapRenderer::AddStopsToMap(Catalogue::TransportCatalogue& catalogue)

{
	std::set<std::string> bus_stops;
	for (const auto& bus : catalogue.GetBusesList())
	{
		Catalogue::Detail::BusObject bus_object = catalogue.GetBusInfo(bus.data());
		if (bus_object.stops == 0)
		{
			continue;
		}
		for (size_t i = 0; i < bus_object.stops; ++i)
		{
			bus_stops.insert(catalogue.GetBusStops(bus.data())[i]->name);
		}
	}

	for (const std::string& stop_name : bus_stops)
	{
		svg::Circle stop;
		Catalogue::Detail::StopObject stop_object = catalogue.GetStopInfo(stop_name);
		stop.SetCenter(projector_(stop_object.coordinates));
		stop.SetRadius(settings_.stop_radius);
		stop.SetFillColor("white"s);
		doc_.Add(stop);
	}
}

void map_renderer::MapRenderer::AddStopNameToMap(Catalogue::TransportCatalogue& catalogue)

{
	std::set<std::string> bus_stops;
	for (const auto& bus : catalogue.GetBusesList())
	{
		Catalogue::Detail::BusObject bus_object = catalogue.GetBusInfo(bus.data());
		if (bus_object.stops == 0)
		{
			continue;
		}
		for (size_t i = 0; i < bus_object.stops; ++i)
		{
			bus_stops.insert(catalogue.GetBusStops(bus.data())[i]->name);
		}
	}
	for (const std::string stop_name : bus_stops)
	{
		Catalogue::Detail::StopObject stop_object = catalogue.GetStopInfo(stop_name);
		svg::Text stop_name_text;
		stop_name_text.SetPosition(projector_(stop_object.coordinates));
		stop_name_text.SetOffset(settings_.stop_label_offset);
		stop_name_text.SetFontSize(settings_.stop_label_font_size);
		stop_name_text.SetFontFamily("Verdana"s);
		stop_name_text.SetData(stop_object.name);
		stop_name_text.SetFillColor("black"s);

		svg::Text stop_name_underlayer = stop_name_text;
		stop_name_underlayer.SetFillColor(settings_.underlayer_color);
		stop_name_underlayer.SetStrokeColor(settings_.underlayer_color);
		stop_name_underlayer.SetStrokeWidth(settings_.underlayer_width);
		stop_name_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		stop_name_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		stop_name_underlayer.SetData(stop_object.name);
		doc_.Add(stop_name_underlayer);
		doc_.Add(stop_name_text);
	}
}

void map_renderer::MapRenderer::RenderMap(std::ostream& out)
{
	this->doc_.Render(out);
}