#include "domain.h"


bool Catalogue::BusStop::operator==(const BusStop& other) const
{
	return name == other.name &&
		coordinates == other.coordinates;
}

bool Catalogue::Bus::operator==(const Bus& other) const
{
	return name == other.name;
}

size_t Catalogue::StopsHasher::operator()(const std::pair<const BusStop*, const BusStop*> stops) const
{
	return hasher_(stops.first) + hasher_(stops.second) * 37;
}