#include <iostream>
#include "geo.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include <sstream>

using namespace std::string_literals;




int main()
{
	Catalogue::TransportCatalogue catalogue;
	json::Document doc = json::Load(std::cin);

	json::SetTransportCatalogue(catalogue, doc.GetRoot().AsDict().at("base_requests"));
	RequestQueue::PerformRequestQueue(catalogue);
	json::SetRoutingSettings(catalogue, doc.GetRoot().AsDict().at("routing_settings"));
	map_renderer::MapVisualSettings map_settings;
	json::SetMapVisualSettings(map_settings, doc.GetRoot().AsDict().at("render_settings"));
	map_renderer::SphereProjector projector = map_renderer::MakeProjector(catalogue, map_settings.width, map_settings.height, map_settings.padding);

	graph::DirectedWeightedGraph<Catalogue::RoadGraphWeight> graph(catalogue.GetStopsCount() * 2);
	catalogue.FillGraph(graph);
	graph::Router<Catalogue::RoadGraphWeight> navigator(graph);


	json::Document answer = json::ExecuteRequests(catalogue, doc.GetRoot().AsDict().at("stat_requests"), map_settings, projector, graph, navigator);
	json::Print(answer, std::cout);
}