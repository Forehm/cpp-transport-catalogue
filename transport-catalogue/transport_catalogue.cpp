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

	bool TransportCatalogue::FindBus(const std::string_view name) const
	{
		auto it = std::find_if(all_buses_.begin(), all_buses_.end(), [&name](const std::string& bus)
			{
				return bus == name;
			});
		return it != all_buses_.end();
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

}