#pragma once 
#include <functional>
#include <vector>
#include <string>
#include "geo.h"



namespace Catalogue
{

	struct BusStop
	{
		std::string name;
		geo::Coordinates coordinates;

		bool operator == (const BusStop& other) const;
	};

	struct Bus
	{
		std::string name;
		std::vector<const BusStop*> route;

		bool operator == (const Bus& other) const;
	};

	struct StopsHasher
	{

		size_t operator()(const std::pair<const BusStop*, const BusStop*> stops) const;


	private:
		std::hash<const void*> hasher_;
	};

	namespace Detail
	{
		struct BusObject
		{
			std::string name;
			size_t stops;
			size_t unique_stops;
			double route_length;
			double curvature;
			bool is_ready = false;
		};

		struct StopObject
		{
			std::string name;
			std::vector<std::string> buses;
			bool is_ready = false;
		};

		struct QueryObject
		{
			std::string query_subject;
			std::string name;
		};
	}

}