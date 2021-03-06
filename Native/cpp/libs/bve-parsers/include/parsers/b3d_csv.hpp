#pragma once

#include "parsers/dependencies.hpp"
#include "parsers/errors.hpp"
#include "util/datatypes.hpp"
#include <vector>

// ReSharper disable once CppInconsistentNaming
namespace bve::parsers::b3d_csv_object {
	struct Vertex {
		glm::vec3 position = glm::vec3(0);
		glm::vec3 normal = glm::vec3(0);
		glm::vec2 texture_coord = glm::vec2(0);
	};

	struct FaceData {
		util::datatypes::Color8RGB emissive_color = {0, 0, 0};
	};

	struct Mesh {
		std::vector<Vertex> verts;
		std::vector<std::size_t> indices;
		std::vector<FaceData> face_data;
		dependencies::Texture texture;
		util::datatypes::Color8RGBA color = {255, 255, 255, 255};
		enum BlendMode { normal, additive } blend_mode;
		enum GlowAttenuationMode { divide_exponent2, divide_exponent4 } glow_attenuation_mode;
		uint16_t glow_half_distance = 0;
	};

	// ReSharper disable once CppInconsistentNaming IdentifierTypo
	struct ParsedB3DCSVObject {
		std::vector<Mesh> meshes;
		dependencies::Dependencies dependencies;
		errors::Errors errors;
	};

	// defined in b3d_csv_object/parse.cpp
	// ReSharper disable once CppInconsistentNaming
	ParsedB3DCSVObject parse_b3d(std::string file_contents);
	ParsedB3DCSVObject parse_csv(std::string file_contents);
} // namespace bve::parsers::b3d_csv_object
