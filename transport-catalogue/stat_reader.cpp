#include "stat_reader.h"

void Catalogue::PrintBusInfo(std::ostream& out, const TransportCatalogue& catalogue, std::string_view bus_name)
{
	catalogue.GetBusInfo(out, bus_name);
}

void Catalogue::PrintStopInfo(std::ostream& out, const TransportCatalogue& catalogue, std::string_view stop_name)
{
	catalogue.GetStopInfo(out, stop_name);
}

Catalogue::Detail::QueryObject Catalogue::ParseRequestQuery(const TransportCatalogue& catalogue, const std::string& query)
{
	std::string query_subject = query.substr(0, query.find_first_of(' '));
	std::string name = query.substr(query.find_first_of(' ') + 1);
	if (name.back() == ' ') { name.pop_back(); }
	return { query_subject , name };
}
