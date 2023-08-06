#pragma once
#include <iostream>
#include <string_view>
#include <vector>
#include <map>
#include "transport_catalogue.h"


namespace Catalogue
{


	void SetTransportCatalogue(TransportCatalogue& cataloque, const std::string& text);


	namespace RequestQueue
	{
		void CheckBusRequestQueue(TransportCatalogue& catalogue);

		void CheckStopRequestQueue(TransportCatalogue& catalogue);

		void PerformRequestQueue(TransportCatalogue& cataloque);

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
	}


	namespace Detail
	{
		std::vector<std::string> BusQueryDelimiter(const std::string& text, const char delimiter);

		std::vector<std::pair<std::string, size_t>> StopQueryDelimiter(const std::string& text);

		void ParseRequestQuery(const TransportCatalogue& catalogue, const std::string& query);
	}

}