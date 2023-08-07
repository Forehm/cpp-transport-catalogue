#include "stat_reader.h"
#include <iomanip>
#include <iostream>

namespace Catalogue
{

	void Catalogue::PrintBusInfo(std::ostream& out, const TransportCatalogue& catalogue, const std::string_view bus_name)
	{
		Detail::BusObject bus_obj = catalogue.GetBusInfo(out, bus_name);
		if (!bus_obj.is_ready)
		{
			out << "Bus " << bus_name << ": not found" << std::endl;
			return;
		}
		out << "Bus " << bus_obj.name << ": " << bus_obj.stops << " stops on route, ";
		out << bus_obj.unique_stops << " unique stops, ";
		out << std::setprecision(6) << bus_obj.route_length << " route length, ";
		out << bus_obj.curvature << " curvature" << std::endl;
	}

	void Catalogue::PrintStopInfo(std::ostream& out, const TransportCatalogue& catalogue, const std::string_view stop_name)
	{
		Detail::StopObject stop_obj = catalogue.GetStopInfo(out, stop_name);
		if (!stop_obj.is_ready)
		{
			out << "Stop " << stop_name << ": not found" << std::endl;
			return;
		}
		if (stop_obj.buses.size() == 0)
		{
			out << "Stop " << stop_name << ": no buses" << std::endl;
			return;
		}
		out << "Stop " << stop_obj.name << ": buses";
		for (const auto& bus : stop_obj.buses)
		{
			out << ' ' << bus;
		}
		out << std::endl;
	}

	Catalogue::Detail::QueryObject Catalogue::ParseRequestQuery(const TransportCatalogue& catalogue, const std::string& query)
	{
		std::string query_subject = query.substr(0, query.find_first_of(' '));
		std::string name = query.substr(query.find_first_of(' ') + 1);
		if (name.back() == ' ') { name.pop_back(); }
		return { query_subject , name };
	}


}