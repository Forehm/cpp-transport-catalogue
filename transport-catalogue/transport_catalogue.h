#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <set>
#include "geo.h"



namespace Catalogue
{

	namespace Detail
	{
		struct BusObject
		{
			std::string name;
			size_t stops;
			size_t unique_stops;
			double route_length;
			double curvature;
			bool is_ready = false;
		};

		struct StopObject
		{
			std::string name;
			std::vector<std::string> buses;
			bool is_ready = false;
		};

		struct QueryObject
		{
			std::string query_subject;
			std::string name;
		};
	}
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
		Detail::BusObject GetBusInfo(std::ostream& out, const std::string_view bus_name) const;
		Detail::StopObject GetStopInfo(std::ostream& out, const std::string_view stop_name) const;
		void SetStopsDistances(const std::string_view stop_name, const std::pair<std::string, size_t>& distances);
		void SetBusDistancesArchive();

	private:
		std::deque<BusStop> stops_;
		std::unordered_map<std::string_view, const BusStop*> bus_stop_indexes_;
		std::deque<std::string> all_buses_;
		std::unordered_map<std::string_view, std::vector<BusStop*>> route_indexes_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stops_;
		std::unordered_map<std::pair<const BusStop*, const BusStop*>, size_t, StopsHasher> distance_between_stops_;
		std::unordered_map<std::string_view, std::pair<double, double>> bus_route_distances_;
	};

}