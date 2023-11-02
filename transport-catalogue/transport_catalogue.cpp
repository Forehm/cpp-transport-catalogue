#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include "transport_catalogue.h"
#include "geo.h"
#include "domain.h"



namespace Catalogue
{
	using namespace std::literals::string_literals;



	void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates)
	{
		stops_.push_back({ name, coordinates });
		bus_stop_indexes_[stops_.back().name] = &stops_.back();
		buses_to_stops_[stops_.back().name] = {};
	}

	bool TransportCatalogue::FindStop(const std::string& name) const
	{
		return bus_stop_indexes_.count(name);
	}

	void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops)
	{
		all_buses_.push_back(name);
		for (const auto& stop_name : stops)
		{
			auto it = std::find_if(stops_.begin(), stops_.end(), [&stop_name](const BusStop& bs) {return bs.name == stop_name; });
			route_indexes_[all_buses_.back()].push_back(&(*it));
			buses_to_stops_[it->name].insert(all_buses_.back());
		}

	}

	void TransportCatalogue::SetBusRoutingSettings(const double wait_time, const double velocity)
	{
		bus_routing_settings_.bus_wait_time = wait_time;
		bus_routing_settings_.bus_velocity = velocity;
	}

	bool TransportCatalogue::FindBus(const std::string_view name) const
	{
		auto it = std::find_if(all_buses_.begin(), all_buses_.end(), [&name](const std::string& bus)
			{
				return bus == name;
			});
		return it != all_buses_.end();
	}

	void TransportCatalogue::FillGraph(graph::DirectedWeightedGraph<Catalogue::RoadGraphWeight>& graph)
	{
		size_t counter = 0;
		
		for (const BusStop& stop : stops_)
		{
			graph::Edge<RoadGraphWeight> edge;
			edge.from = counter;
			edge.to = counter + 1;
			edge.weight.time = bus_routing_settings_.bus_wait_time;
			edge.weight.span_count = 0;
			stops_meta_info_[stop.name] = { counter, counter + 1 };
			size_t id_from = stops_meta_info_[stop.name].first;
			size_t id_to = stops_meta_info_[stop.name].second;

			DistancesMetaInfo stops_info;
			stops_info.name = stop.name;
			stops_info.spans_count = 0;
			stops_info.type = "Wait"s;
			stops_info.weight = bus_routing_settings_.bus_wait_time;
			stop_distances_meta_info_[{id_from, id_to}] = { stops_info };
			graph.AddEdge(edge);
			counter += 2;
		}

		for (const auto& [bus, stops] : route_indexes_)
		{
			for (size_t i = 0; i < stops.size() - 1; ++i)
			{
				for (size_t j = i + 1; j < stops.size(); ++j)
				{
					graph::Edge<RoadGraphWeight> edge;
					edge.from = stops_meta_info_[stops[i]->name].second;
					edge.to = stops_meta_info_[stops[j]->name].first;
					double time = 0.0;
					double speedMetersPerSecond = (bus_routing_settings_.bus_velocity * 1000.0) / 3600.0;
					double distance = 0.0;
					
					size_t span_count = (j - i);

					for (size_t _i = i; _i < j; ++_i)
					{
						if (distance_between_stops_.count({ stops[_i], stops[_i + 1] }))
						{
							distance = distance_between_stops_.at({ stops[_i], stops[_i + 1] });
							time += distance / speedMetersPerSecond;
						}
						else
						{
							distance = geo::ComputeDistance(stops[_i]->coordinates, stops[_i + 1]->coordinates);
							time += distance / speedMetersPerSecond;
						}
					}
					time /= 60.0;

					DistancesMetaInfo bus_route_info;
					bus_route_info.name = bus;
					bus_route_info.spans_count = span_count;
					bus_route_info.type = "Bus"s;
					bus_route_info.weight = time;
					bus_route_info.from = stops[i]->name;
					bus_route_info.to = stops[j]->name;

					stop_distances_meta_info_[{edge.from, edge.to}] = { bus_route_info };

					edge.weight.time = time;
					edge.weight.span_count = span_count;
					graph.AddEdge(edge);
				}
			}
			
		}
	
	}




	Detail::BusObject TransportCatalogue::GetBusInfo(const std::string_view bus_name) const
	{
		Detail::BusObject bus_obj{};
		if (FindBus(bus_name))
		{
			std::set<BusStop*> unique_stops;
			for (const auto& stop : route_indexes_.at(bus_name))
			{
				unique_stops.insert(stop);
			}

			bus_obj.name = { bus_name.begin(), bus_name.end() };
			bus_obj.route_length = bus_route_distances_.at(bus_name).first;
			bus_obj.unique_stops = unique_stops.size();
			bus_obj.stops = route_indexes_.at(bus_name).size();
			bus_obj.curvature = bus_route_distances_.at(bus_name).second;
			bus_obj.is_ready = true;
		}
		return bus_obj;
	}

	Catalogue::Detail::StopObject TransportCatalogue::GetStopInfo(const std::string_view stop_name) const
	{
		Detail::StopObject stop_obj{};
		std::vector<std::string> buses;
		if (buses_to_stops_.count(stop_name))
		{
			for (const auto& bus : buses_to_stops_.at(stop_name))
			{
				buses.push_back({ bus.begin(), bus.end() });
			}
			stop_obj.buses = buses;
			stop_obj.name = { stop_name.begin(), stop_name.end() };
			stop_obj.is_ready = true;
			stop_obj.coordinates = bus_stop_indexes_.at(stop_name)->coordinates;
		}
		return stop_obj;
	}

	void TransportCatalogue::SetStopsDistances(const std::string_view stop_name, const std::pair<std::string, size_t>& distances)
	{
		auto it_stop_name = std::find_if(stops_.begin(), stops_.end(), [&stop_name](const BusStop& stop)
			{
				return stop.name == stop_name;
			});
		auto it_other_stop_name = std::find_if(stops_.begin(), stops_.end(), [&distances](const BusStop& stop)
			{
				return stop.name == distances.first;
			});

		if ((distance_between_stops_.count({ &(*it_stop_name), &(*it_other_stop_name) }) &&
			(distance_between_stops_.count({ &(*it_other_stop_name), &(*it_stop_name) }))))
		{
			distance_between_stops_[{&(*it_stop_name), & (*it_other_stop_name)}] = distances.second;
		}
		else
		{
			distance_between_stops_[{&(*it_stop_name), & (*it_other_stop_name)}] = distances.second;
			distance_between_stops_[{&(*it_other_stop_name), & (*it_stop_name)}] = distances.second;
		}

	}

	void TransportCatalogue::SetBusDistancesArchive()
	{

		for (const std::string& bus_name : all_buses_)
		{
			double route_length = 0;
			double route_length_real = 0;
			for (size_t i = 1; i < route_indexes_.at(bus_name).size(); ++i)
			{
				route_length += geo::ComputeDistance(route_indexes_.at(bus_name)[i]->coordinates, route_indexes_.at(bus_name)[i - 1]->coordinates);
			}

			for (auto begin = route_indexes_.at(bus_name).begin() + 1; begin != route_indexes_.at(bus_name).end(); ++begin)
			{
				if (distance_between_stops_.count({ *(begin - 1) , *begin }))
				{
					route_length_real += distance_between_stops_.at({ *(begin - 1) , *begin });
				}
				else if (distance_between_stops_.count({ *begin, *(begin - 1) }))
				{
					route_length_real += distance_between_stops_.at({ *begin, *(begin - 1) });
				}
				else
				{
					route_length_real += geo::ComputeDistance((*begin - 1)->coordinates, (*begin)->coordinates);
				}
			}
			bus_route_distances_[bus_name] = { route_length_real, route_length_real / route_length };
		}
	}

	const std::set<std::string_view> TransportCatalogue::GetBusesList() const
	{
		std::set<std::string_view> buses;
		for (const std::string& bus : all_buses_)
		{
			buses.insert(bus);
		}
		return buses;
	}

	std::vector<BusStop*> TransportCatalogue::GetBusStops(const std::string bus_name)
	{
		return route_indexes_.at(bus_name);
	}

	void TransportCatalogue::AddRoundTripBus(const std::string& bus_name)
	{
		round_trip_buses.insert(bus_name);
	}

	bool TransportCatalogue::IsBusRoundTrip(const std::string bus_name)
	{
		return round_trip_buses.count(bus_name);
	}

	std::string TransportCatalogue::GetLastStopToBus(const std::string bus_name)
	{
		std::string stop_name = "";
		if (last_stop_to_buses_.count(bus_name))
		{
			stop_name = last_stop_to_buses_.at(bus_name);
		}
		return stop_name;
	}

	void TransportCatalogue::AddLastStopToBus(const std::string bus_name, const std::string stop_name)
	{
		last_stop_to_buses_[bus_name] = stop_name;
	}

	std::unordered_map<std::string_view, const BusStop*> TransportCatalogue::GetBuses()
	{
		return bus_stop_indexes_;
	}

	size_t TransportCatalogue::GetStopsCount() const
	{
		return stops_.size();
	}

	std::pair<size_t, size_t> TransportCatalogue::GetStopIdForRouter(const std::string& stop_name) const
	{
		return stops_meta_info_.at(stop_name);
	}

	double TransportCatalogue::GetBusVelocity() const
	{
		return bus_routing_settings_.bus_velocity;
	}

	DistancesMetaInfo TransportCatalogue::GetRouteMetaInfo(const std::pair<size_t, size_t> ids) const
	{
		return stop_distances_meta_info_.at(ids);
	}

	double TransportCatalogue::getBusWaitTime() const
	{
		return bus_routing_settings_.bus_wait_time;
	}

	std::pair<std::string, std::string> TransportCatalogue::GetStopNamesByMetaIDS(const size_t first, const size_t second) const
	{
		std::pair<std::string, std::string> names;

		names.first = stop_distances_meta_info_.at({ first, second }).from;
		names.second = stop_distances_meta_info_.at({ first, second }).to;

		return names;
	}

	
	

	

}