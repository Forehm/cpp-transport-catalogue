#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

namespace RequestQueue
{
	void CheckBusRequestQueue(Catalogue::TransportCatalogue& catalogue);

	void CheckStopRequestQueue(Catalogue::TransportCatalogue& catalogue);

	void PerformRequestQueue(Catalogue::TransportCatalogue& cataloque);

	class BusRequestsQueueManager
	{
	public:
		void AddBusRequest(std::vector<std::string> request);

		size_t GetBusRequestQueueSize();

		std::vector<std::vector<std::string>>::iterator begin();

		std::vector<std::vector<std::string>>::iterator end();

		void Clear();

	private:
		std::vector<std::vector<std::string>> bus_request_queue_;

	};

	class StopRequestsQueueManager
	{
	public:

		void AddStopRequest(const std::string& stop_name, const std::vector<std::pair<std::string, size_t>>& distances);

		size_t GetStopRequestQueueSize();

		std::map<std::string, std::pair<std::string, size_t>>::iterator begin();

		std::map<std::string, std::pair<std::string, size_t>>::iterator end();

		void Clear();

	private:
		std::multimap<std::string, std::pair<std::string, size_t>> stop_request_queue_;
	};

	bool IsBusReadyToAdd(const std::vector<std::string>& stops,
		const Catalogue::TransportCatalogue& catalogue);
}



namespace json
{

	
	void SetTransportCatalogue(Catalogue::TransportCatalogue& catalogue, const Node& base_requests);

	Document ExecuteRequests(Catalogue::TransportCatalogue& catalogue, const Node& stat_requests, map_renderer::MapVisualSettings& settings,
		map_renderer::SphereProjector& projector);

	void ParseBusJson(Catalogue::TransportCatalogue& catalogue, const Node& query);

	void ParseStopJson(Catalogue::TransportCatalogue& catalogue, const Node& query);

	void SetMapVisualSettings(map_renderer::MapVisualSettings& map_settings, const Node& doc);


}