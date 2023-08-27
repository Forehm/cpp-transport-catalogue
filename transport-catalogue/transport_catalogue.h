#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <set>
#include "geo.h"
#include "domain.h"
#include <map>


namespace Catalogue
{



	class TransportCatalogue
	{
	public:
		void AddStop(const std::string& name, const geo::Coordinates& coordinates);
		bool FindStop(const std::string& name) const;
		void AddBus(const std::string& name, const std::vector<std::string>& stops);
		bool FindBus(const std::string_view name) const;
		Detail::BusObject GetBusInfo(const std::string_view bus_name) const;
		Detail::StopObject GetStopInfo(const std::string_view stop_name) const;
		void SetStopsDistances(const std::string_view stop_name, const std::pair<std::string, size_t>& distances);
		void SetBusDistancesArchive();
		const std::set<std::string_view> GetBusesList() const;
		std::vector<BusStop*> GetBusStops(const std::string bus_name);
		void AddRoundTripBus(const std::string& bus_name);
		bool IsBusRoundTrip(const std::string bus_name);
		std::string GetLastStopToBus(const std::string bus_name);
		void AddLastStopToBus(const std::string bus_name, const std::string stop_name);
		std::unordered_map<std::string_view, const BusStop*> GetBuses();

	private:
		std::map<std::string, std::string> last_stop_to_buses_;
		std::deque<BusStop> stops_;
		std::unordered_map<std::string_view, const BusStop*> bus_stop_indexes_;
		std::deque<std::string> all_buses_;
		std::set<std::string> round_trip_buses;
		std::unordered_map<std::string_view, std::vector<BusStop*>> route_indexes_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stops_;
		std::unordered_map<std::pair<const BusStop*, const BusStop*>, size_t, StopsHasher> distance_between_stops_;
		std::unordered_map<std::string_view, std::pair<double, double>> bus_route_distances_;
	};

}