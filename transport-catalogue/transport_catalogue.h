#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <set>
#include "geo.h"
#include "domain.h"
#include "graph.h"
#include "router.h"
#include <map>



namespace Catalogue
{
	struct RoadGraphWeight {
		double time;
		size_t span_count;

		bool operator < (const RoadGraphWeight& another) const
		{
			return this->time < another.time;
		}
		bool operator > (const RoadGraphWeight& another) const
		{
			return this->time > another.time;
		}

		RoadGraphWeight operator + (const RoadGraphWeight& another) const
		{
			RoadGraphWeight f;
			f.span_count = this->span_count + another.span_count;
			f.time = this->time + another.time;
			return f;
		}


	};

	

	struct BusRoutingSettings
	{
		double bus_wait_time = 0;
		double bus_velocity = 0;
	};

	struct DistancesMetaInfo
	{
		double weight = 0.0;
		int spans_count = 0;
		std::string type;
		std::string name;
		std::string from;
		std::string to;
	};

	

	class TransportCatalogue
	{
	public:	

		Detail::BusObject GetBusInfo(const std::string_view bus_name) const;
		Detail::StopObject GetStopInfo(const std::string_view stop_name) const;
		const std::set<std::string_view> GetBusesList() const;
		std::vector<BusStop*> GetBusStops(const std::string bus_name) const;
		std::string GetLastStopToBus(const std::string bus_name);
		std::unordered_map<std::string_view, const BusStop*> GetBuses();
		size_t GetStopsCount() const;
		std::deque<BusStop> GetAllStops() const;
		double GetBusVelocity() const;
		double getBusWaitTime() const;
		double GetDistanceBetweenStops(const BusStop* from, const BusStop* to) const;

		void SetStopsDistances(const std::string_view stop_name, const std::pair<std::string, size_t>& distances);
		void SetBusDistancesArchive();
		void AddLastStopToBus(const std::string bus_name, const std::string stop_name);
		void AddRoundTripBus(const std::string& bus_name);
		void AddStop(const std::string& name, const geo::Coordinates& coordinates);
		void AddBus(const std::string& name, const std::vector<std::string>& stops);
		void SetBusRoutingSettings(const double wait_time, const double velocity);

		bool IsBusRoundTrip(const std::string bus_name);
		bool FindStop(const std::string& name) const;
		bool FindBus(const std::string_view name) const;

	private:
		
		BusRoutingSettings bus_routing_settings_;
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