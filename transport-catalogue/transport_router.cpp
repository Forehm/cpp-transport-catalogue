#include "transport_router.h"

TransportRouter::TransportRouter(const Catalogue::TransportCatalogue& catalogue) : catalogue_(catalogue)
{
	graph_ = std::make_shared<graph::DirectedWeightedGraph<Catalogue::RoadGraphWeight>>(catalogue.GetStopsCount() * 2);
	FillGraph();
	const graph::DirectedWeightedGraph<Catalogue::RoadGraphWeight>* constGraph = graph_.get();
	router_ = std::make_shared<graph::Router<Catalogue::RoadGraphWeight>>(*constGraph);
}

void TransportRouter::FillGraph()
{
	size_t counter = 0;

	for (const auto stop : catalogue_.GetAllStops())
	{
		graph::Edge<Catalogue::RoadGraphWeight> edge;
		edge.from = counter;
		edge.to = counter + 1;
		edge.weight.time = catalogue_.getBusWaitTime();
		edge.weight.span_count = 0;
		stops_meta_info_[stop.name] = { counter, counter + 1 };
		size_t id_from = stops_meta_info_[stop.name].first;
		size_t id_to = stops_meta_info_[stop.name].second;

		Catalogue::DistancesMetaInfo stops_info;
		stops_info.name = stop.name;
		stops_info.spans_count = 0;
		stops_info.type = "Wait";
		stops_info.weight = catalogue_.getBusWaitTime();
		stop_distances_meta_info_[{id_from, id_to}] = { stops_info };
		graph_->AddEdge(edge);
		counter += 2;
	}

	for (const auto& bus : catalogue_.GetBusesList())
	{
		auto stops = catalogue_.GetBusStops({ bus.begin(), bus.end() });
		for (size_t i = 0; i < stops.size() - 1; ++i)
		{
			for (size_t j = i + 1; j < stops.size(); ++j)
			{
				graph::Edge<Catalogue::RoadGraphWeight> edge;
				edge.from = stops_meta_info_[stops[i]->name].second;
				edge.to = stops_meta_info_[stops[j]->name].first;
				double time = 0.0;
				double speedMetersPerSecond = (catalogue_.GetBusVelocity() * 1000.0) / 3600.0;
				double distance = 0.0;

				size_t span_count = (j - i);

				for (size_t _i = i; _i < j; ++_i)
				{
					distance = catalogue_.GetDistanceBetweenStops(stops[_i], stops[_i + 1]);
					time += distance / speedMetersPerSecond;
				}
				time /= 60.0;

				Catalogue::DistancesMetaInfo bus_route_info;
				bus_route_info.name = bus;
				bus_route_info.spans_count = span_count;
				bus_route_info.type = "Bus";
				bus_route_info.weight = time;
				bus_route_info.from = stops[i]->name;
				bus_route_info.to = stops[j]->name;

				stop_distances_meta_info_[{edge.from, edge.to}] = { bus_route_info };

				edge.weight.time = time;
				edge.weight.span_count = span_count;
				graph_->AddEdge(edge);
			}
		}

	}
}

std::pair<size_t, size_t> TransportRouter::GetStopIdForRouter(const std::string& stop_name) const
{
	return stops_meta_info_.at(stop_name);
}

Catalogue::DistancesMetaInfo TransportRouter::GetRouteMetaInfo(const std::pair<size_t, size_t> ids) const
{
	return stop_distances_meta_info_.at(ids);
}

std::pair<std::string, std::string> TransportRouter::GetStopNamesByMetaIDS(const size_t first, const size_t second) const
{
	std::pair<std::string, std::string> names;

	names.first = stop_distances_meta_info_.at({ first, second }).from;
	names.second = stop_distances_meta_info_.at({ first, second }).to;

	return names;
}

std::optional<graph::Router<Catalogue::RoadGraphWeight>::RouteInfo> TransportRouter::BuildRoute(const size_t from, const size_t to) const
{
	return router_->BuildRoute(from, to);
}

const graph::Edge<Catalogue::RoadGraphWeight>& TransportRouter::GetEdge(const size_t edge_id) const
{
	return graph_->GetEdge(edge_id);
}


