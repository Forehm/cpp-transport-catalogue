#pragma once
#include <iostream>
#include <string_view>
#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"




namespace Catalogue
{

	void PrintBusInfo(std::ostream& out, const TransportCatalogue& catalogue, const std::string_view bus_name);

	void PrintStopInfo(std::ostream& out, const TransportCatalogue& catalogue, const std::string_view stop_name);

	Detail::QueryObject ParseRequestQuery(const TransportCatalogue& catalogue, const std::string& query);


}