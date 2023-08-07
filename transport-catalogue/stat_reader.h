#pragma once
#include <iostream>
#include <fstream>
#include <string_view>
#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

namespace Catalogue
{

	namespace Detail
	{
		struct QueryObject
		{
			std::string query_subject;
			std::string name;
		};
	}

	void PrintBusInfo(std::ostream& out, const TransportCatalogue& catalogue, std::string_view bus_name);

	void PrintStopInfo(std::ostream& out, const TransportCatalogue& catalogue, std::string_view stop_name);

	Detail::QueryObject ParseRequestQuery(const TransportCatalogue& catalogue, const std::string& query);


}