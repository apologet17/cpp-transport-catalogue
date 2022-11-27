#pragma once
#include <string>
#include <fstream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include "transport_router.pb.h"
#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>



namespace serializator {

	using NameToId = std::unordered_map<std::string_view, uint32_t>;

	struct Settings {
		std::string filename;
	};

	class Serialization {
	public:
		Serialization() = default;
		bool SerializeFull( const catalogue_core::transport_catalogue::TransportCatalogue* catalogue_,
						    catalogue_core::renderer::MapRenderer* map_renderer_,
						    std::unique_ptr<catalogue_core::router::TransportRouter>& transport_router_);

		bool DeserializeFull(catalogue_core::transport_catalogue::TransportCatalogue* catalogue_,
							 catalogue_core::renderer::MapRenderer* map_renderer_,
							 std::unique_ptr<catalogue_core::router::TransportRouter>& transport_router_);

		std::tuple<transport_serialize::Catalogue, NameToId, NameToId>  SerializeCatalogue(const catalogue_core::transport_catalogue::TransportCatalogue* catalogue_);
		std::pair<std::vector<domain::BusStop*>, std::vector<domain::BusRoute*>> DeserializeCatalogue(catalogue_core::transport_catalogue::TransportCatalogue* catalogue_, transport_serialize::Catalogue& serialize_catalogue);

		transport_serialize::Settings SerializeMap(catalogue_core::renderer::MapRenderer* map_renderer_);
		void DeserializeMap(catalogue_core::renderer::MapRenderer* map_renderer_, transport_serialize::Settings& serialize_map);
		
		transport_serialize::Router SerializeRouter(std::unique_ptr<catalogue_core::router::TransportRouter>& transport_router_,
													const NameToId& stopname_to_ids_,
													const NameToId& busname_to_ids_);
		void DeserializeRouter( catalogue_core::transport_catalogue::TransportCatalogue& catalogue_,
								std::unique_ptr<catalogue_core::router::TransportRouter>& transport_router_,
								const transport_serialize::Router& serialize_router,
								const std::vector<domain::BusStop*>&,
								const std::vector<domain::BusRoute*>&);
		void SetSettings(const std::string& filename);


	private:
		Settings settings_;	

	};
}