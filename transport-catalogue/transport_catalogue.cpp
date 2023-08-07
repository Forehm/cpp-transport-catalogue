#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include "transport_catalogue.h"
#include "geo.h"



namespace Catalogue
{
	using namespace std::literals::string_literals;

	void TransportCatalogue::AddStop(const std::string& name, const Coordinates& coordinates)
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

	void TransportCatalogue::GetBusInfo(std::ostream& out, const std::string_view bus_name) const
	{
		if (!FindBus(bus_name))
		{
			out << "Bus "s << bus_name << ": not found"s << std::endl;
			return;
		}
		
		out << "Bus " << bus_name << ": " << route_indexes_.at(bus_name).size() << " stops on route, ";

		std::set<BusStop*> unique_stops;
		for (const auto& stop : route_indexes_.at(bus_name))
		{
			unique_stops.insert(stop);
		}

		out << unique_stops.size() << " unique stops, ";
		out << std::setprecision(6) << bus_route_distances_.at(bus_name).first << " route length, ";
		out << bus_route_distances_.at(bus_name).second << " curvature" << std::endl;
		
	}

	void TransportCatalogue::GetStopInfo(std::ostream& out, const std::string_view stop_name) const
	{
		if (!buses_to_stops_.count(stop_name))
		{
			out << "Stop " << stop_name << ": not found" << std::endl;
			return;
		}
		if (buses_to_stops_.at(stop_name).size() == 0)
		{
			out << "Stop " << stop_name << ": no buses" << std::endl;
			return;
		}
		out << "Stop " << stop_name << ": buses";
		for (const auto& bus : buses_to_stops_.at(stop_name))
		{
			std::cout << ' ' << bus;
		}
		out << std::endl;
		
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
				route_length += ComputeDistance(route_indexes_.at(bus_name)[i]->coordinates, route_indexes_.at(bus_name)[i - 1]->coordinates);
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
					route_length_real += ComputeDistance((*begin - 1)->coordinates, (*begin)->coordinates);
				}
			}
			bus_route_distances_[bus_name] = {route_length_real, route_length_real / route_length} ;
		}
	}

}