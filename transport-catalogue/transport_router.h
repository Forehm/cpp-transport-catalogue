#pragma once
#include <memory>
#include "graph.h"
#include "transport_catalogue.h"
#include "router.h"





	class TransportRouter
	{
	public:
		TransportRouter(const Catalogue::TransportCatalogue& catalogue);

		void FillGraph();

		std::pair<size_t, size_t> GetStopIdForRouter(const std::string& stop_name) const;
		Catalogue::DistancesMetaInfo GetRouteMetaInfo(const std::pair<size_t, size_t> ids) const;
		std::pair<std::string, std::string> GetStopNamesByMetaIDS(const size_t first, const size_t second) const;
		std::optional<graph::Router<Catalogue::RoadGraphWeight>::RouteInfo> BuildRoute(const size_t from, const size_t to) const;
		const graph::Edge<Catalogue::RoadGraphWeight>& GetEdge(const size_t edge_id) const;

	private:
		std::shared_ptr<graph::DirectedWeightedGraph<Catalogue::RoadGraphWeight>> graph_;
		std::shared_ptr<graph::Router<Catalogue::RoadGraphWeight>> router_;
		const Catalogue::TransportCatalogue& catalogue_;
		std::map<std::string, std::pair<size_t, size_t>> stops_meta_info_;
		std::map<std::pair<size_t, size_t>, Catalogue::DistancesMetaInfo> stop_distances_meta_info_;
	};


