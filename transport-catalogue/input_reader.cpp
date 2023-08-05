#include <iostream>
#include <string_view>
#include <string>
#include <queue>
#include <algorithm>
#include "input_reader.h"
#include "transport_catalogue.h"


namespace Catalogue
{
	
	RequestQueue::BusRequestsQueueManager bus_manager;
	RequestQueue::StopRequestsQueueManager stop_manager;


	void SetTransportCatalogue(TransportCatalogue& catalogue, const std::string& text)
	{

		std::string entity = text.substr(0, text.find_first_of(' '));
		if (entity == "Stop")
		{
			auto begin = text.begin();
			std::advance(begin, entity.size() + 1);
			auto end = text.begin();

			while (*end != ':') { ++end; }
			std::string name(begin, end);
			while (*end != ',') { ++end; }
			while (*begin != ':') { ++begin; }
			std::advance(begin, 2);
			std::string latitude(begin, end);
			++end;

			end = std::find(end, text.end(), ',');

			while (*begin != ' ') { ++begin; }
			std::advance(begin, 1);
			std::string longitude(begin, end);
			begin = end;

			begin = std::find(begin, text.end(), ' ');
			end = text.end();
			std::vector<std::pair<std::string, size_t>> distances;
			if (begin != end)
			{
				distances = Detail::StopQueryDelimiter({ ++begin, end });
			}

			catalogue.AddStop(name, { std::stod(latitude), std::stod(longitude) });


			stop_manager.AddStopRequest(name, distances);


		}
		if (entity == "Bus")
		{
			auto begin = text.begin();
			std::advance(begin, entity.size() + 1);
			auto end = text.begin();
			while (*end != ':') { ++end; }
			std::string bus_name(begin, end);
			begin = end;
			++begin;
			end = text.end();
			std::vector<std::string> stops;
			std::string stop;
			char delimiter_symbol;
			auto it = std::find(text.begin(), text.end(), '>');
			delimiter_symbol = it == text.end() ? '-' : '>';
			stops = Detail::BusQueryDelimiter({ begin + 1, text.end() }, delimiter_symbol);

			bool flag_bus_to_add = true;
			for (const auto& elem : stops)
			{
				if (!catalogue.FindStop(elem))
				{
					stops.push_back(bus_name);
					bus_manager.AddBusRequest(std::move(stops));
					flag_bus_to_add = false;
					break;
				}
			}
			if (flag_bus_to_add == true)
			{
				catalogue.AddBus(bus_name, stops);
			}
		}

	}


	namespace RequestQueue
	{
		void PerformRequestQueue(TransportCatalogue& cataloque)
		{
			RequestQueue::CheckBusRequestQueue(cataloque);
			RequestQueue::CheckStopRequestQueue(cataloque);
		}

		void CheckStopRequestQueue(TransportCatalogue& catalogue)
		{
			std::vector<std::pair<std::string, size_t>> distances;
			for (const auto& [stop_name, other_stop_pair] : stop_manager)
			{
				if (catalogue.FindStop(stop_name) && catalogue.FindStop(other_stop_pair.first))
				{
					catalogue.SetStopsDistances(stop_name, other_stop_pair);
				}
			}
			stop_manager.Clear();
		}

		void CheckBusRequestQueue(TransportCatalogue& catalogue)
		{
			for (const auto& elem : bus_manager)
			{
				bool flag_bus_to_add = true;
				for (size_t i = 0; i < elem.size() - 1; ++i)
				{
					if (!catalogue.FindStop(elem[i]))
					{
						flag_bus_to_add = false;
						break;
					}
				}
				if (flag_bus_to_add == true)
				{
					catalogue.AddBus(elem.back(), { elem.begin(), elem.end() - 1 });
				}
			}
			bus_manager.Clear();
		}
	}

	namespace Detail
	{
		std::vector<std::string> BusQueryDelimiter(const std::string& text, const char delimiter)
		{
			std::vector<std::string> strs;
			std::string str;
			for (size_t i = 0; i < text.size(); ++i)
			{
				if (text[i] == delimiter)
				{
					++i;
					strs.push_back(str);
					str = "";
				}
				else
				{
					str += text[i];
				}

			}
			strs.push_back(str);
			for (auto& elem : strs)
			{
				if (elem.back() == ' ')
				{
					elem.pop_back();
				}
			}
			std::vector<std::string> vec;
			if (delimiter == '-')
			{
				for (int i = strs.size() - 2; i >= 0; --i)
				{
					vec.push_back(strs[i]);
				}
			}
			for (auto& elem : vec)
			{
				strs.push_back(elem);
			}
			return strs;
		}

		std::vector<std::pair<std::string, size_t>> StopQueryDelimiter(const std::string& text)
		{
			std::vector<std::pair<std::string, size_t>> distances;
			std::vector<std::string> raw_stops;
			auto begin = text.begin();
			auto end = text.end();
			while (begin != text.end())
			{
				end = std::find(begin, text.end(), ',');
				raw_stops.push_back({ begin, end });
				if (end != text.end()) { ++end; }
				begin = end;
			}
			for (std::string& raw_stop : raw_stops)
			{
				if (raw_stop.front() == ' ') { raw_stop = raw_stop.substr(1); }
			}
			for (std::string& raw_stop : raw_stops)
			{
				auto begin = raw_stop.begin();
				auto end = std::find(raw_stop.begin(), raw_stop.end(), 'm');
				std::string distance(begin, end);
				begin = std::find(raw_stop.begin(), raw_stop.end(), ' ');
				std::advance(begin, 4);
				end = raw_stop.end();
				distances.push_back({ {begin, end}, static_cast<size_t>(std::stoi(distance)) });
			}

			return distances;
		}

		void ParseRequestQuery(const TransportCatalogue& catalogue, const std::string& query)
		{
			std::string query_subject = query.substr(0, query.find_first_of(' '));
			std::string name = query.substr(query.find_first_of(' ') + 1);
			if (name.back() == ' ') { name.pop_back(); }
			if (query_subject == "Bus")
			{
				catalogue.GetBusInfo(name);
			}
			if (query_subject == "Stop")
			{
				catalogue.GetStopInfo(name);
			}
		}
	}

}