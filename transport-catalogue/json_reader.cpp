#include "json_reader.h"
#include <map>
#include <vector>
#include <string>
#include <sstream>

using namespace std::string_literals;

RequestQueue::BusRequestsQueueManager bus_manager;
RequestQueue::StopRequestsQueueManager stop_manager;

namespace RequestQueue
{
	void PerformRequestQueue(Catalogue::TransportCatalogue& catalogue)
	{
		RequestQueue::CheckStopRequestQueue(catalogue);
		RequestQueue::CheckBusRequestQueue(catalogue);
		catalogue.SetBusDistancesArchive();
	}

	bool IsBusReadyToAdd(const std::vector<std::string>& stops,
		const Catalogue::TransportCatalogue& catalogue)
	{
		for (const std::string& stop : stops)
		{
			if (!catalogue.FindStop(stop))
			{
				return false;
			}
		}
		return true;
	}

	void CheckStopRequestQueue(Catalogue::TransportCatalogue& catalogue)
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

	void CheckBusRequestQueue(Catalogue::TransportCatalogue& catalogue)
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

	void BusRequestsQueueManager::AddBusRequest(std::vector<std::string> request)
	{
		bus_request_queue_.push_back(std::move(request));
	}

	size_t BusRequestsQueueManager::GetBusRequestQueueSize()
	{
		return bus_request_queue_.size();
	}

	std::vector<std::vector<std::string>>::iterator BusRequestsQueueManager::begin()
	{
		return bus_request_queue_.begin();
	}

	std::vector<std::vector<std::string>>::iterator BusRequestsQueueManager::end()
	{
		return bus_request_queue_.end();
	}

	void BusRequestsQueueManager::Clear()
	{
		bus_request_queue_.clear();
	}

	void StopRequestsQueueManager::AddStopRequest(const std::string& stop_name, const std::vector<std::pair<std::string, size_t>>& distances)
	{
		for (const auto& [other_stop, dist] : distances)
		{
			stop_request_queue_.insert({ stop_name,{other_stop, dist} });
		}
	}

	size_t StopRequestsQueueManager::GetStopRequestQueueSize()
	{
		return stop_request_queue_.size();
	}

	std::map<std::string, std::pair<std::string, size_t>>::iterator StopRequestsQueueManager::begin()
	{
		return stop_request_queue_.begin();
	}

	std::map<std::string, std::pair<std::string, size_t>>::iterator StopRequestsQueueManager::end()
	{
		return stop_request_queue_.end();
	}

	void StopRequestsQueueManager::Clear()
	{
		stop_request_queue_.clear();
	}
}






void json::SetTransportCatalogue(Catalogue::TransportCatalogue& catalogue, const Node& base_requests)
{
	for (const Node& query : base_requests.AsArray())
	{

		if (query.AsMap().at("type"s).AsString() == "Bus")
		{
			ParseBusJson(catalogue, query);
		}
		if (query.AsMap().at("type"s).AsString() == "Stop")
		{
			ParseStopJson(catalogue, query);
		}
	}

}

json::Dict ExecuteStopRequest(Catalogue::TransportCatalogue& catalogue, const json::Node& request)
{
	json::Array arr;
	json::Dict dict;
	if (!catalogue.FindStop(request.AsMap().at("name").AsString()))
	{
		dict["request_id"] = request.AsMap().at("id").AsInt();
		dict["error_message"] = "not found"s;
		return dict;
	}
	else
	{
		Catalogue::Detail::StopObject stop_object = catalogue.GetStopInfo(request.AsMap().at("name").AsString());
		for (const std::string& bus : stop_object.buses)
		{
			arr.push_back(bus);
		}
		dict["buses"] = arr;
		dict["request_id"] = request.AsMap().at("id");
		return dict;
	}
}

json::Dict ExecuteBusRequest(Catalogue::TransportCatalogue& catalogue, const json::Node& request)
{
	json::Dict dict;
	json::Array arr;
	if (!catalogue.FindBus(request.AsMap().at("name").AsString()))
	{
		dict["request_id"] = request.AsMap().at("id").AsInt();
		dict["error_message"] = "not found"s;
		return dict;
	}
	else
	{
		Catalogue::Detail::BusObject bus_object = catalogue.GetBusInfo(request.AsMap().at("name").AsString());
		dict["curvature"] = bus_object.curvature;
		dict["request_id"] = request.AsMap().at("id");
		dict["route_length"] = bus_object.route_length;
		dict["stop_count"] = static_cast<int>(bus_object.stops);
		dict["unique_stop_count"] = static_cast<int>(bus_object.unique_stops);
		return dict;
	}

}

json::Dict ExecuteMapRequest(Catalogue::TransportCatalogue& catalogue, const json::Node& request, 
	map_renderer::MapVisualSettings& settings, map_renderer::SphereProjector& projector)
{
	json::Dict dict;
	json::Array arr;

	std::ostringstream ss;
	dict["request_id"] = request.AsMap().at("id").AsInt();
	svg::Document doc;
	map_renderer::MapRenderer map(settings, projector);
	map.AddRoadsToMap(catalogue);
	map.AddBusRoutesToMap(catalogue);
	map.AddStopsToMap(catalogue);
	map.AddStopNameToMap(catalogue);

	map.RenderMap(ss);
	dict["map"] = ss.str();

	return dict;
}

json::Document json::ExecuteRequests(Catalogue::TransportCatalogue& catalogue, const Node& stat_requests,
	map_renderer::MapVisualSettings& settings, map_renderer::SphereProjector& projector)
{
	Array result;

	for (const Node& request : stat_requests.AsArray())
	{
		Dict dict;
		Array arr;
		if (request.AsMap().at("type").AsString() == "Stop")
		{
			result.push_back(ExecuteStopRequest(catalogue, request));
		}
		if (request.AsMap().at("type").AsString() == "Bus")
		{
			result.push_back(ExecuteBusRequest(catalogue, request));
		}
		if (request.AsMap().at("type").AsString() == "Map")
		{
			result.push_back(ExecuteMapRequest(catalogue, request, settings, projector));
		}
	}
	Document doc(result);
	return doc;
}

void json::ParseBusJson(Catalogue::TransportCatalogue& catalogue, const Node& query)
{
	std::vector<std::string> stops;
	std::string bus_name = query.AsMap().at("name"s).AsString();

	if (query.AsMap().at("is_roundtrip"s).AsBool() == true)
	{
		catalogue.AddRoundTripBus(bus_name);
		for (auto stop : query.AsMap().at("stops"s).AsArray())
		{
			stops.push_back(stop.AsString());
		}
		catalogue.AddLastStopToBus(bus_name, stops.back());
	}
	else
	{
		for (auto stop : query.AsMap().at("stops"s).AsArray())
		{
			stops.push_back(stop.AsString());
		}
		catalogue.AddLastStopToBus(bus_name, stops.back());
		auto& container = query.AsMap().at("stops"s).AsArray();
		for (auto it = container.rbegin() + 1; it != container.rend(); ++it)
		{
			stops.push_back(it->AsString());
		}
	}
	if (RequestQueue::IsBusReadyToAdd(stops, catalogue))
	{
		catalogue.AddBus(bus_name, stops);
	}
	else
	{
		stops.push_back(bus_name);
		bus_manager.AddBusRequest(std::move(stops));
	}
}

void json::ParseStopJson(Catalogue::TransportCatalogue& catalogue, const Node& query)
{
	std::string stop_name = query.AsMap().at("name"s).AsString();
	double latitude = query.AsMap().at("latitude"s).AsDouble();
	double longitude = query.AsMap().at("longitude"s).AsDouble();
	catalogue.AddStop(stop_name, { latitude, longitude });


	std::vector<std::pair<std::string, size_t>> distances;
	for (const auto& [stop, distance] : query.AsMap().at("road_distances"s).AsMap())
	{
		distances.push_back({ stop, distance.AsInt() });
	}

	stop_manager.AddStopRequest(stop_name, distances);

}

void json::SetMapVisualSettings(map_renderer::MapVisualSettings& map_settings, const Node& doc)
{
	map_renderer::MapVisualSettings temp_settings;
	if (doc.AsMap().count("width"s)) { temp_settings.width = doc.AsMap().at("width"s).AsDouble(); }
	if (doc.AsMap().count("height"s)) { temp_settings.height = doc.AsMap().at("height"s).AsDouble(); }
	if (doc.AsMap().count("padding"s)) { temp_settings.padding = doc.AsMap().at("padding"s).AsDouble(); }
	if (doc.AsMap().count("stop_radius"s)) { temp_settings.stop_radius = doc.AsMap().at("stop_radius"s).AsDouble(); }
	if (doc.AsMap().count("line_width"s)) { temp_settings.line_width = doc.AsMap().at("line_width"s).AsDouble(); }
	if (doc.AsMap().count("bus_label_font_size"s)) { temp_settings.bus_label_font_size = static_cast<size_t>(doc.AsMap().at("bus_label_font_size"s).AsInt()); }
	if (doc.AsMap().count("bus_label_offset"s)) {
		temp_settings.bus_label_offset = { doc.AsMap().at("bus_label_offset"s).AsArray()[0].AsDouble(),
doc.AsMap().at("bus_label_offset"s).AsArray()[1].AsDouble() };
	}
	if (doc.AsMap().count("stop_label_font_size"s)) { temp_settings.stop_label_font_size = doc.AsMap().at("stop_label_font_size"s).AsInt(); }
	if (doc.AsMap().count("stop_label_offset"s)) {
		temp_settings.stop_label_offset = { doc.AsMap().at("stop_label_offset"s).AsArray()[0].AsDouble(),
			doc.AsMap().at("stop_label_offset"s).AsArray()[1].AsDouble() };
	}


	if (doc.AsMap().count("underlayer_color"s))
	{
		std::vector<double> arr;
		svg::Color color;
		if (doc.AsMap().at("underlayer_color").IsArray())
		{
			for (auto color : doc.AsMap().at("underlayer_color").AsArray())
			{
				if (color.IsInt()) { arr.push_back(color.AsInt()); }
				if (color.IsPureDouble())
				{
					arr.push_back(color.AsDouble());
				}
			}
			if (arr.size() == 3)
			{
				color = svg::Rgb(static_cast<uint8_t>(arr[0]), static_cast<uint8_t>(arr[1]), static_cast<uint8_t>(arr[2]));
			}
			else if (arr.size() == 4)
			{
				color = svg::Rgba(static_cast<uint8_t>(arr[0]), static_cast<uint8_t>(arr[1]), static_cast<uint8_t>(arr[2]), arr[3]);
			}
		}
		if (doc.AsMap().at("underlayer_color").IsString())
		{
			color = doc.AsMap().at("underlayer_color").AsString();
		}
		temp_settings.underlayer_color = color;
	}

	if (doc.AsMap().count("underlayer_width"s)) { temp_settings.underlayer_width = doc.AsMap().at("underlayer_width"s).AsDouble(); }
	if (doc.AsMap().count("color_palette"s))
	{
		std::vector<svg::Color> arr;
		for (auto color : doc.AsMap().at("color_palette").AsArray())
		{
			if (color.IsString()) { arr.push_back(color.AsString()); }
			if (color.IsArray())
			{
				if (color.AsArray().size() == 3)
				{
					arr.push_back(svg::Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt()));
				}
				else if (color.AsArray().size() == 4)
				{
					arr.push_back(svg::Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble()));
				}
			}
		}
		temp_settings.color_palette = arr;
	}
	std::swap(temp_settings, map_settings);
}
