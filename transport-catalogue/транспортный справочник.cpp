#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"


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
		Catalogue::Detail::ParseRequestQuery(tc, query);
		query = "";
		--number_of_queries;
	}

}