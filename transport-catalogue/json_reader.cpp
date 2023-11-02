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
	for (const json::Node& query : base_requests.AsArray())
	{
		if (query.AsDict().at("type"s).AsString() == "Bus")
		{
			ParseBusJson(catalogue, query);
		}
		if (query.AsDict().at("type"s).AsString() == "Stop")
		{
			ParseStopJson(catalogue, query);
		}
	}

}

void json::SetRoutingSettings(Catalogue::TransportCatalogue& catalogue, const Node& routing_settings)
{
	const double wait_time = routing_settings.AsDict().at("bus_wait_time").AsDouble();
	const double velocity = routing_settings.AsDict().at("bus_velocity").AsDouble();
	catalogue.SetBusRoutingSettings(wait_time, velocity);
}

json::Dict ExecuteStopRequest(Catalogue::TransportCatalogue& catalogue, const json::Node& request)
{
	json::Builder answer;
	answer.StartDict();
	if (!catalogue.FindStop(request.AsDict().at("name").AsString()))
	{
		answer.Key("request_id"s).Value(request.AsDict().at("id").AsInt());
		answer.Key("error_message"s).Value("not found"s);
		answer.EndDict();
		return answer.Build().AsDict();
	}
	else
	{
		answer.Key("buses"s).StartArray();
		Catalogue::Detail::StopObject stop_object = catalogue.GetStopInfo(request.AsDict().at("name").AsString());
		for (const std::string& bus : stop_object.buses)
		{
			answer.Value(bus);
		}
		answer.EndArray();
		answer.Key("request_id"s).Value(request.AsDict().at("id").AsInt());
		answer.EndDict();
		return answer.Build().AsDict();
	}
}

json::Dict ExecuteBusRequest(Catalogue::TransportCatalogue& catalogue, const json::Node& request)
{
	json::Builder answer;
	answer.StartDict();
	if (!catalogue.FindBus(request.AsDict().at("name").AsString()))
	{
		answer.Key("request_id"s).Value(request.AsDict().at("id").AsInt());
		answer.Key("error_message"s).Value("not found"s);
		answer.EndDict();
		return answer.Build().AsDict();
	}
	else
	{
		Catalogue::Detail::BusObject bus_object = catalogue.GetBusInfo(request.AsDict().at("name").AsString());

		answer.Key("curvature"s).Value(bus_object.curvature);
		answer.Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
		answer.Key("route_length"s).Value(bus_object.route_length);
		answer.Key("stop_count"s).Value(static_cast<int>(bus_object.stops));
		answer.Key("unique_stop_count"s).Value(static_cast<int>(bus_object.unique_stops));
		answer.EndDict();
		return answer.Build().AsDict();
	}

}

json::Dict ExecuteMapRequest(Catalogue::TransportCatalogue& catalogue, const json::Node& request,
	map_renderer::MapVisualSettings& settings, map_renderer::SphereProjector& projector)
{
	json::Builder answer;
	
	answer.StartDict();
	answer.Key("request_id"s).Value(request.AsDict().at("id").AsInt());

	svg::Document doc;
	std::ostringstream ss;
	map_renderer::MapRenderer map(settings, projector);
	map.AddRoadsToMap(catalogue);
	map.AddBusRoutesToMap(catalogue);
	map.AddStopsToMap(catalogue);
	map.AddStopNameToMap(catalogue);
	map.RenderMap(ss);

	answer.Key("map"s).Value(ss.str()).EndDict();
	return answer.Build().AsDict();
}

json::Dict ExecuteRouteRequest(Catalogue::TransportCatalogue& catalogue, const json::Node& request, graph::DirectedWeightedGraph<Catalogue::RoadGraphWeight>& graph, 
	graph::Router<Catalogue::RoadGraphWeight>& navigator)
{
	json::Builder answer;
	
	answer.StartDict();

	int request_id = request.AsDict().at("id").AsInt();
	std::string from_name = request.AsDict().at("from").AsString();
	std::string to_name = request.AsDict().at("to").AsString();

	auto ids_from = catalogue.GetStopIdForRouter(from_name);
	auto ids_to = catalogue.GetStopIdForRouter(to_name);

	auto info = navigator.BuildRoute(ids_from.first, ids_to.first);
	
	if (info.has_value())
	{
		double total_time = info->weight.time;
		answer.Key("items"s).StartArray();

		for (const auto edge : info.value().edges)
		{
			graph::Edge<Catalogue::RoadGraphWeight> edge_info = graph.GetEdge(edge);
			Catalogue::DistancesMetaInfo meta_info = catalogue.GetRouteMetaInfo({ edge_info.from, edge_info.to });
			std::pair<std::string, std::string> names = catalogue.GetStopNamesByMetaIDS(edge_info.from, edge_info.to);
			answer.StartDict();
			answer.Key("type"s).Value(meta_info.type);
			if (meta_info.type == "Wait"s)
			{
				answer.Key("stop_name"s).Value(meta_info.name);
			}
			else
			{
				answer.Key("bus"s).Value(meta_info.name);
				answer.Key("span_count"s).Value((int)edge_info.weight.span_count);
			}
			answer.Key("time"s).Value(edge_info.weight.time);
			answer.EndDict();
		}  
		answer.EndArray();
		answer.Key("request_id"s).Value(request_id);
		answer.Key("total_time"s).Value(total_time);
		answer.EndDict();
	}
	else
	{
		answer.Key("request_id"s).Value(request_id);
		answer.Key("error_message"s).Value("not found"s);
		answer.EndDict();
	}
	
	
	return answer.Build().AsDict();
}

json::Document json::ExecuteRequests(Catalogue::TransportCatalogue& catalogue, const Node& stat_requests,
	map_renderer::MapVisualSettings& settings, map_renderer::SphereProjector& projector, graph::DirectedWeightedGraph<Catalogue::RoadGraphWeight>& graph, 
	graph::Router<Catalogue::RoadGraphWeight>& navigator)
{

	json::Builder result;
	result.StartArray();

	for (const Node& request : stat_requests.AsArray())
	{
		if (request.AsDict().at("type").AsString() == "Stop")
		{
			result.Value(ExecuteStopRequest(catalogue, request));
		}
		if (request.AsDict().at("type").AsString() == "Bus")
		{
			result.Value(ExecuteBusRequest(catalogue, request));
		}
		if (request.AsDict().at("type").AsString() == "Map")
		{
			result.Value(ExecuteMapRequest(catalogue, request, settings, projector));
		}
		if (request.AsDict().at("type").AsString() == "Route")
		{
			result.Value(ExecuteRouteRequest(catalogue, request, graph, navigator));
		}
	}
	result.EndArray();

	return Document(result.Build());
}

void json::ParseBusJson(Catalogue::TransportCatalogue& catalogue, const Node& query)
{
	std::vector<std::string> stops;
	std::string bus_name = query.AsDict().at("name"s).AsString();

	if (query.AsDict().at("is_roundtrip"s).AsBool() == true)
	{
		catalogue.AddRoundTripBus(bus_name);
		for (auto stop : query.AsDict().at("stops"s).AsArray())
		{
			stops.push_back(stop.AsString());
		}
		catalogue.AddLastStopToBus(bus_name, stops.back());
	}
	else
	{
		for (auto stop : query.AsDict().at("stops"s).AsArray())
		{
			stops.push_back(stop.AsString());
		}
		catalogue.AddLastStopToBus(bus_name, stops.back());
		auto& container = query.AsDict().at("stops"s).AsArray();
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
	std::string stop_name = query.AsDict().at("name"s).AsString();
	double latitude = query.AsDict().at("latitude"s).AsDouble();
	double longitude = query.AsDict().at("longitude"s).AsDouble();
	catalogue.AddStop(stop_name, { latitude, longitude });


	std::vector<std::pair<std::string, size_t>> distances;
	for (const auto& [stop, distance] : query.AsDict().at("road_distances"s).AsDict())
	{
		distances.push_back({ stop, distance.AsInt() });
	}

	stop_manager.AddStopRequest(stop_name, distances);

}

void json::SetMapVisualSettings(map_renderer::MapVisualSettings& map_settings, const Node& doc)
{
	map_renderer::MapVisualSettings temp_settings;
	if (doc.AsDict().count("width"s)) { temp_settings.width = doc.AsDict().at("width"s).AsDouble(); }
	if (doc.AsDict().count("height"s)) { temp_settings.height = doc.AsDict().at("height"s).AsDouble(); }
	if (doc.AsDict().count("padding"s)) { temp_settings.padding = doc.AsDict().at("padding"s).AsDouble(); }
	if (doc.AsDict().count("stop_radius"s)) { temp_settings.stop_radius = doc.AsDict().at("stop_radius"s).AsDouble(); }
	if (doc.AsDict().count("line_width"s)) { temp_settings.line_width = doc.AsDict().at("line_width"s).AsDouble(); }
	if (doc.AsDict().count("bus_label_font_size"s)) { temp_settings.bus_label_font_size = static_cast<size_t>(doc.AsDict().at("bus_label_font_size"s).AsInt()); }
	if (doc.AsDict().count("bus_label_offset"s)) {
		temp_settings.bus_label_offset = { doc.AsDict().at("bus_label_offset"s).AsArray()[0].AsDouble(),
		doc.AsDict().at("bus_label_offset"s).AsArray()[1].AsDouble() };
	}
	if (doc.AsDict().count("stop_label_font_size"s)) { temp_settings.stop_label_font_size = doc.AsDict().at("stop_label_font_size"s).AsInt(); }
	if (doc.AsDict().count("stop_label_offset"s)) {
		temp_settings.stop_label_offset = { doc.AsDict().at("stop_label_offset"s).AsArray()[0].AsDouble(),
			doc.AsDict().at("stop_label_offset"s).AsArray()[1].AsDouble() };
	}


	if (doc.AsDict().count("underlayer_color"s))
	{
		std::vector<double> arr;
		svg::Color color;
		if (doc.AsDict().at("underlayer_color").IsArray())
		{
			for (auto color : doc.AsDict().at("underlayer_color").AsArray())
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
		if (doc.AsDict().at("underlayer_color").IsString())
		{
			color = doc.AsDict().at("underlayer_color").AsString();
		}
		temp_settings.underlayer_color = color;
	}

	if (doc.AsDict().count("underlayer_width"s)) { temp_settings.underlayer_width = doc.AsDict().at("underlayer_width"s).AsDouble(); }
	if (doc.AsDict().count("color_palette"s))
	{
		std::vector<svg::Color> arr;
		for (auto color : doc.AsDict().at("color_palette").AsArray())
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