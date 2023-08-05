#pragma once
#include "geo.h"
#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <set>


namespace Catalogue
{

	struct BusStop
	{
		std::string name;
		Coordinates coordinates;

		bool operator == (const BusStop& other) const
		{
			return name == other.name &&
				coordinates == other.coordinates;
		}
	};

	struct Bus
	{
		std::string name;
		std::vector<const BusStop*> route;

		bool operator == (const Bus& other) const
		{
			return name == other.name;
		}
	};

	struct StopsHasher
	{

		size_t operator()(const std::pair<const BusStop*, const BusStop*> stops) const
		{
			return hasher_(stops.first) + hasher_(stops.second) * 37;
		}


	private:
		std::hash<const void*> hasher_;
	};

	class TransportCatalogue
	{
	public:
		void AddStop(const std::string& name, const Coordinates& coordinates);
		bool FindStop(const std::string& name) const;
		void AddBus(const std::string& name, const std::vector<std::string>& stops);
		bool FindBus(const std::string_view name) const;
		void GetBusInfo(const std::string_view bus_name) const;
		void GetStopInfo(const std::string_view stop_name) const;
		void SetStopsDistances(const std::string_view stop_name, const std::pair<std::string, size_t>& distances);

	private:
		std::deque<BusStop> stops_;
		std::unordered_map<std::string_view, const BusStop*> bus_stop_indexes_;
		std::deque<std::string> all_buses_;
		std::unordered_map<std::string_view, std::vector<BusStop*>> route_indexes_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stops_;
		std::unordered_map<std::pair<const BusStop*, const BusStop*>, size_t, StopsHasher> distance_between_stops_;
	};

}