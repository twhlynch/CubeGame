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
constexpr float half = size * 0.5f;
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
		// a───b  a, b, c
		// │ / │  b, d, c
		// c───d
		0, 1, 4, // front  top left
		1, 5, 4, // front  bottom right
		1, 2, 5, // right  top left
		2, 6, 5, // right  bottom right
		2, 3, 6, // back   top left
		3, 7, 6, // back   bottom right
		3, 0, 7, // left   top left
		0, 4, 7, // left   bottom right
		3, 2, 0, // top    top left
		2, 1, 0, // top    bottom right
		6, 7, 4, // bottom top left
		5, 6, 4, // bottom bottom right
	});
	// clang-format on
}

static RN::Mesh *RectangularPrismMesh()
{
	// clang-format off
	return MeshBuilder({
		{ half,  size,  half}, // top    front left  0
		{-half,  size,  half}, // top    front right 1
		{-half,  size, -half}, // top    back  right 2
		{ half,  size, -half}, // top    back  left  3
		{ half, -size,  half}, // bottom front left  4
		{-half, -size,  half}, // bottom front right 5
		{-half, -size, -half}, // bottom back  right 6
		{ half, -size, -half}, // bottom back  left  7
	}, {
		0, 1, 4, // front  top left
		1, 5, 4, // front  bottom right
		1, 2, 5, // right  top left
		2, 6, 5, // right  bottom right
		2, 3, 6, // back   top left
		3, 7, 6, // back   bottom right
		3, 0, 7, // left   top left
		0, 4, 7, // left   bottom right
		3, 2, 0, // top    top left
		2, 1, 0, // top    bottom right
		6, 7, 4, // bottom top left
		5, 6, 4, // bottom bottom right
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

			vertices[index++] = RN::Vector3(x, y, z) * size;
		}
	}

	index = 0;

	auto triangle = [&](RN::uint16 i0, RN::uint16 i1, RN::uint16 i2) {
		indices[index++] = i0;
		indices[index++] = i2;
		indices[index++] = i1;
	};

	auto idx = [&](size_t i, size_t j) {
		return static_cast<RN::uint16>((i * segments) + j);
	};

	for (size_t i = 0; i < segments - 1; i++)
	{
		for (size_t j = 0; j < segments; j++)
		{
			const size_t next = (j + 1) % segments;

			const RN::uint16 a = idx(i, j);
			const RN::uint16 b = idx(i, next);
			const RN::uint16 c = idx(i + 1, j);
			const RN::uint16 d = idx(i + 1, next);

			triangle(a, c, b);
			triangle(b, c, d);
		}
	}

	return MeshBuilder(vertices, indices);
}

static RN::Mesh *CylinderMesh()
{
	std::array<RN::Vector3, (segments * 2) + 2> vertices;
	std::array<RN::uint16, (segments * 6) + (segments * 3 * 2)> indices;

	size_t index = 0;

	for (size_t ring = 0; ring < 2; ring++)
	{
		float y = (ring == 0) ? -0.5f : 0.5f;

		for (size_t slice = 0; slice < segments; slice++)
		{
			const float frac = static_cast<float>(slice) / segments;
			const float phi = 2.0f * RN::k::Pi * frac;

			const float x = std::sin(phi);
			const float z = std::cos(phi);

			vertices[index++] = RN::Vector3(x, y, z) * size;
		}
	}

	auto bottom = static_cast<RN::uint16>(index);
	vertices[index++] = RN::Vector3(0.0f, -size * 0.5f, 0.0f);

	auto top = static_cast<RN::uint16>(index);
	vertices[index++] = RN::Vector3(0.0f, size * 0.5f, 0.0f);

	index = 0;

	auto triangle = [&](RN::uint16 a, RN::uint16 b, RN::uint16 c) {
		indices[index++] = a;
		indices[index++] = c;
		indices[index++] = b;
	};

	auto idx = [&](size_t i, size_t j) {
		return static_cast<RN::uint16>((i * segments) + j);
	};

	for (size_t j = 0; j < segments; j++)
	{
		const size_t next = (j + 1) % segments;

		const RN::uint16 a = idx(0, j);
		const RN::uint16 b = idx(0, next);
		const RN::uint16 c = idx(1, j);
		const RN::uint16 d = idx(1, next);

		triangle(a, c, b);
		triangle(b, c, d);
		triangle(top, d, c);
		triangle(bottom, a, b);
	}

	return MeshBuilder(vertices, indices);
}

template <typename V, typename I, typename N>
static RN::Mesh *MeshBuilderWithNormals(
	const V &verticesData,
	const I &indicesData,
	const N &normalsData)
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

	for (size_t i = 0; i < verticesData.size(); i++)
	{
		*vertices++ = verticesData[i];
		*normals++ = normalsData[i];
		*colors++ = {1.0f, 1.0f, 1.0f};
		*indices++ = indicesData[i];
	}

	mesh->changedVertices = true;
	mesh->changedIndices = true;
	mesh->EndChanges();

	return mesh->Autorelease();
}

static RN::Mesh *PyramidMeshWithNormals()
{
	std::array<RN::Vector3, 18> vertices;
	std::array<RN::Vector3, 18> normals;
	std::array<RN::uint16, 18> indices;

	RN::uint16 index = 0;

	const RN::Vector3 top = {0.0f, size, 0.0f};
	const RN::Vector3 fl = {-size, -size, size};
	const RN::Vector3 fr = {size, -size, size};
	const RN::Vector3 br = {size, -size, -size};
	const RN::Vector3 bl = {-size, -size, -size};

	auto tri = [&](const RN::Vector3 &a, const RN::Vector3 &b, const RN::Vector3 &c) {
		const RN::Vector3 normal = (b - a).GetCrossProduct(c - a).GetNormalized();

		// clang-format off
		vertices[index] = a; normals[index] = normal; indices[index] = index; index++;
		vertices[index] = b; normals[index] = normal; indices[index] = index; index++;
		vertices[index] = c; normals[index] = normal; indices[index] = index; index++;
		// clang-format on
	};

	tri(top, fl, fr); // sides
	tri(top, fr, br);
	tri(top, br, bl);
	tri(top, bl, fl);

	tri(fl, bl, br); // base
	tri(br, fr, fl);

	return MeshBuilderWithNormals(vertices, indices, normals);
}

static RN::Mesh *CylinderMeshWithNormals()
{
	constexpr size_t triCount =
		(segments * 2) + // sides (2 tris per segment)
		segments +		 // top
		segments;		 // bottom

	std::array<RN::Vector3, triCount * 3> vertices;
	std::array<RN::Vector3, triCount * 3> normals;
	std::array<RN::uint16, triCount * 3> indices;

	RN::uint16 index = 0;

	auto tri = [&](const RN::Vector3 &a, const RN::Vector3 &b, const RN::Vector3 &c, const RN::Vector3 &normal) {
		// clang-format off
		vertices[index] = a; normals[index] = normal; indices[index] = index; index++;
		vertices[index] = c; normals[index] = normal; indices[index] = index; index++;
		vertices[index] = b; normals[index] = normal; indices[index] = index; index++;
		// clang-format on
	};

	auto circle = [&](float y, float angle) {
		return RN::Vector3(std::sin(angle), y, std::cos(angle)) * size;
	};

	const RN::Vector3 topCenter = {0.0f, size * 0.5f, 0.0f};
	const RN::Vector3 topNormal = {0.0f, 1.0f, 0.0f};
	const RN::Vector3 bottomCenter = {0.0f, -size * 0.5f, 0.0f};
	const RN::Vector3 bottomNormal = {0.0f, -1.0f, 0.0f};

	for (size_t i = 0; i < segments; i++)
	{
		float a0 = (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * RN::k::Pi;
		float a1 = static_cast<float>((i + 1) % segments) / static_cast<float>(segments) * 2.0f * RN::k::Pi;

		RN::Vector3 b0 = circle(-0.5f, a0);
		RN::Vector3 b1 = circle(-0.5f, a1);
		RN::Vector3 t0 = circle(0.5f, a0);
		RN::Vector3 t1 = circle(0.5f, a1);

		RN::Vector3 normal = (b1 - b0).GetCrossProduct(t0 - b0).GetNormalized();

		tri(b0, t0, b1, normal);
		tri(b1, t0, t1, normal);

		tri(topCenter, t1, t0, topNormal);
		tri(bottomCenter, b0, b1, bottomNormal);
	}

	return MeshBuilderWithNormals(vertices, indices, normals);
}

} // namespace CG::Mesh
