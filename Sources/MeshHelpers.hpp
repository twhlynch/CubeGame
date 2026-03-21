#include <Math/RNMatrixQuaternion.h>
#include <Math/RNVector.h>
#include <Scene/RNSceneNode.h>

#include <RNJoltShape.h>
#include <RayneConfig.h>
#include <initializer_list>

namespace CG::Mesh
{

using VertexAttribute = RN::Mesh::VertexAttribute;

constexpr float size = 1.0f;
constexpr size_t segments = 12;

// NOTE: only intended for simple origin centered convex shapes
template <typename V, typename I>
static RN::Mesh *MeshBuilder(
	const V &verticesData,
	const I &indicesData)
{
	auto *mesh = new RN::Mesh(
		{
			VertexAttribute(VertexAttribute::Feature::Vertices, RN::PrimitiveType::Vector3),
			VertexAttribute(VertexAttribute::Feature::Normals, RN::PrimitiveType::Vector3),
			VertexAttribute(VertexAttribute::Feature::Color0, RN::PrimitiveType::Color),
			VertexAttribute(VertexAttribute::Feature::Indices, RN::PrimitiveType::Uint16),
		},
		verticesData.size(), indicesData.size());

	mesh->BeginChanges();
	auto chunk = mesh->GetChunk();

	auto vertices = chunk.GetIterator<RN::Vector3>(VertexAttribute::Feature::Vertices);
	auto normals = chunk.GetIterator<RN::Vector3>(VertexAttribute::Feature::Normals);
	auto colors = chunk.GetIterator<RN::Color>(VertexAttribute::Feature::Color0);
	auto indices = chunk.GetIterator<RN::uint16>(VertexAttribute::Feature::Indices);

	for (const auto &verticeData : verticesData)
	{
		*vertices++ = verticeData;
		*normals++ = verticeData.GetNormalized();
	}

	for (size_t i = 0; i < verticesData.size(); i++)
	{
		*colors++ = {1.0f, 1.0f, 1.0f}; // white
	}

	for (const auto &indiceData : indicesData)
	{
		*indices++ = indiceData;
	}

	mesh->changedVertices = true;
	mesh->changedIndices = true;
	mesh->EndChanges();

	return mesh->Autorelease();
}

static RN::Mesh *MeshBuilder(
	const std::initializer_list<RN::Vector3> &verticesData,
	const std::initializer_list<RN::uint16> &indicesData)
{
	return MeshBuilder<
		std::initializer_list<RN::Vector3>,
		std::initializer_list<RN::uint16>>(
		verticesData, indicesData);
}

// NOTE: box drawing symbols
// ─ │ ┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼ ╱ ╲ ╳ ╴ ╵ ╶ ╷ 🯐 🯑 🯒 🯓

static RN::Mesh *PyramidMesh()
{
	// clang-format off
	return MeshBuilder({
		{ 0.0f,  size,  0.0f}, // top         0
		{-size, -size,  size}, // front left  1
		{ size, -size,  size}, // front right 2
		{ size, -size, -size}, // back  right 3
		{-size, -size, -size}, // back  left  4
	}, {
		//   a   a, b, c
		//  ╱ ╲
		// b───c
		0, 1, 2, // front
		0, 2, 3, // right
		0, 3, 4, // back
		0, 4, 1, // left

		1, 4, 3, // bottom
		3, 2, 1,
	});
	// clang-format on
}

static RN::Mesh *CubeMesh()
{
	// clang-format off
	return MeshBuilder({
		{ size,  size,  size}, // top    front left  0
		{-size,  size,  size}, // top    front right 1
		{-size,  size, -size}, // top    back  right 2
		{ size,  size, -size}, // top    back  left  3
		{ size, -size,  size}, // bottom front left  4
		{-size, -size,  size}, // bottom front right 5
		{-size, -size, -size}, // bottom back  right 6
		{ size, -size, -size}, // bottom back  left  7
	}, {
		// a───b  a, c, b
		// │ / │  b, c, d
		// c───d
		0, 4, 1, // front  top left
		1, 4, 5, // front  bottom right
		1, 5, 2, // right  top left
		2, 5, 6, // right  bottom right
		2, 6, 3, // back   top left
		3, 6, 7, // back   bottom right
		3, 7, 0, // left   top left
		0, 7, 4, // left   bottom right
		3, 0, 2, // top    top left
		2, 0, 1, // top    bottom right
		6, 4, 7, // bottom top left
		5, 4, 6, // bottom bottom right
	});
	// clang-format on
}

static RN::Mesh *SphereMesh()
{
	std::array<RN::Vector3, segments * segments> vertices;
	std::array<RN::uint16, (segments - 1) * segments * 6> indices;

	size_t index = 0;

	for (size_t ring = 0; ring < segments; ring++)
	{
		for (size_t slice = 0; slice < segments; slice++)
		{
			const float ringFrac = static_cast<float>(ring) / (segments - 1);
			const float sliceFrac = static_cast<float>(slice) / segments;

			const float theta = RN::k::Pi * ringFrac;
			const float phi = 2.0f * RN::k::Pi * sliceFrac;

			const float y = -std::cos(theta);
			const float r = std::sqrt(1.0f - (y * y));
			const float x = r * std::sin(phi);
			const float z = r * std::cos(phi);

			RN::Vector3 position(x, y, z);
			vertices[index++] = position * size;
		}
	}

	index = 0;

	auto triangle = [&](RN::uint16 i0, RN::uint16 i1, RN::uint16 i2) {
		indices[index++] = i0;
		indices[index++] = i1;
		indices[index++] = i2;
	};

	auto idx = [&](size_t i, size_t j) {
		return static_cast<RN::uint16>((i * segments) + j);
	};

	for (size_t i = 0; i < segments - 1; i++)
	{
		for (size_t j = 0; j < segments; j++)
		{
			size_t next = (j + 1) % segments;

			auto a = idx(i, j);
			auto b = idx(i, next);
			auto c = idx(i + 1, j);
			auto d = idx(i + 1, next);

			triangle(a, c, b);
			triangle(b, c, d);
		}
	}

	return MeshBuilder(vertices, indices);
}

} // namespace CG::Mesh
