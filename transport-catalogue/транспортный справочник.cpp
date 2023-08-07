#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

using namespace Catalogue;

int main()
{
	TransportCatalogue tc;

	size_t number_of_queries = 0;
	std::cin >> number_of_queries;
	std::cin.ignore();
	std::string query;



	while (number_of_queries > 0)
	{
		std::getline(std::cin, query);
		SetTransportCatalogue(tc, query);
		query = "";
		--number_of_queries;

	}

	RequestQueue::PerformRequestQueue(tc);

	std::cin >> number_of_queries;
	std::cin.ignore();
	query = "";

	while (number_of_queries > 0)
	{
		std::getline(std::cin, query);
		Catalogue::Detail::QueryObject query_object = Catalogue::ParseRequestQuery(tc, query);
		if (query_object.query_subject == "Stop")
		{
			Catalogue::PrintStopInfo(std::cout, tc, query_object.name);
		}
		if (query_object.query_subject == "Bus")
		{
			Catalogue::PrintBusInfo(std::cout, tc, query_object.name);
		}
		query = "";
		--number_of_queries;
	}

}