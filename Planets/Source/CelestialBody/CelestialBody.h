#pragma once
#include "../Rendering/Program.h"
#include "../Rendering/Camera.h"
#include "../Mathematics/Matrix/Matrix.h"
#include "../Rendering/Vertex/CelestialVertex.h"
#include "../Rendering/Vertex/CelestialVertexGlsl.h"
#include "../DynamicVariableGroup.h"
#include "CelestialBodyTextures.h"

struct CraterData
{
	// Each element of an array inside a uniform block, needs to be
	// stored at a memory address that is a multiple of its own size
	// rounded up to a multiple of a "vec4". "CraterData" has a size of
	// 3 * 4 ("position") + 4 ("randomValue") + 1 ("hasTexture") = 4 * 4 + 1
	// , which is just above a vec4 (4 * 4). Each element should therefore be stored
	// at a memory address that is a multiple of 2 * the size of a "vec4" 
	// = 4 * 4 * 2, hence "alignas (4 * 4 * 2)".
	TightlyPackedVector3 alignas(4 * 4 * 2) position;
	float randomValue;
	bool hasTexture;
};

struct CelestialBodyDimensions
{
	// The length of a cell
	float cellSideLength = 0.0f;

	// The length of the cube's sides, in amount of cells
	int sideLengthInCells = 0;

	// The radius of celestial body in model space
	static constexpr float MODEL_RADIUS = 1.0f;

	// The diameter of celestial body in model space
	static constexpr float MODEL_DIAMETER = MODEL_RADIUS * 2.0f;
};

class CelestialBody
{
public:
	CelestialBody(const std::shared_ptr<Program> renderingProgram,
		const std::shared_ptr<Program> terrainGeneratorProgram, const Vector3& position,
		float scale, float cellSideLength, std::shared_ptr<DynamicVariableGroup<float>> variableGroup);
	~CelestialBody();
	void Render(const Camera& camera, const Matrix4& projectionMatrix) const;
	void Update(float deltaTime);
	Vector3 GetPosition() const;

	// Returns the radius of the rendered celestial body
	float GetRadius() const;
private:
	// Returns all the vertices of the face projected on to a sphere of radius:
	// "mDimensions.MODEL_RADIUS"
	std::vector<CelestialVertex> GetFaceVertices(const Vector3& lowerLeftCornerOfFace,
		const Vector3& tangent, const Vector3& binormal) const;

	CelestialVertex GetVertex(int xOffset, int yOffset, const Vector3& toRight,
		const Vector3& toUp, const Vector3& lowerLeftCornerOfFace) const;

	// Returns a sphere made up of vertices that originally formed the
	// shape of a cube
	std::vector<CelestialVertex> GetSphereVertices() const;

	// Runs the terrain generator program, a compute shader, which will
	// generate the terrain of the vertices inside the shader storage buffer
	void RunTerrainGeneratorProgram(size_t nVertices, int nCraters, float maxCraterTextureRadius);

	// Will run the terrain generator program and update the vertices inside
	// the vbo, which will affect the rendered celestial body. "mSphereVertices"
	// will remain unchanged.
	void UpdateVboVertices(std::vector<CelestialVertex> vertices);

	void InitializeVao();
	void InitializeShaderStorageBufferObject();
	void InitializeUniformBufferObject();
	void InitializeVbo();

	// The following three methods generate crater data that will get passed to
	// the terrain generator program through the uniform buffer object
	std::vector<TightlyPackedVector3> GetCraterPositions(int nCraters,
		const std::vector<CelestialVertex>& vertices) const;
	std::vector<float> GetRandomCraterValues(int nCraters) const;
	std::vector<bool> GetCraterTextureBools(
		const std::vector<TightlyPackedVector3>& craterPositions, int nWantedCraterTextures,
		float maxCraterTextureRadius) const;

	// Updates the uniform buffer object, with the data generated from the three above methods
	void UpdateUniformBufferObject(const std::vector<CelestialVertex>& vertices,
		int nCraters, float maxCraterTextureRadius);

	// Updates the shader storage buffer object with the passed in vertices
	void UpdateShaderStorageBufferObject(const std::vector<CelestialVertex>& vertices) const;

	// Binds all the necessary uniforms for rendering
	void BindUniforms(const Camera& camera, const Matrix4& projectionMatrix) const;
private:
	// The textures are static, since we want all the celestial bodies
	// to share the same textures. We can not initialize the textures
	// here, because the initialization would then happen before 
	// the initialization of GLEW. We therefore use "std::optional"
	// to delay the initialization.
	static inline std::optional<CelestialBodyTextures> msTextures;
	static inline bool msInitializedTextures = false;

	const std::shared_ptr<Program> mRenderingProgram;

	// Generates the vertices' positions and uvs for
	// the terrain of the celestial body
	const std::shared_ptr<Program> mTerrainGeneratorProgram;

	// The OpenGL objects
	GLuint mVao = 0;
	GLuint mVbo = 0;
	GLuint mShaderStorageBufferObject = 0;
	GLuint mUniformBufferObject = 0;

	Vector3 mPosition;
	float mScale = 0.0f;

	// Dynamically updateable variables with parameters 
	// that decide the generation of the terrain
	std::shared_ptr<DynamicVariableGroup<float>> mVariableGroup;

	// Vertices that form the shape of a sphere with the
	// radius of "mDimensions.MODEL_RADIUS"
	std::vector<CelestialVertex> mSphereVertices;

	// The dimensions that decide the positions of the model's vertices
	CelestialBodyDimensions mDimensions;

	static constexpr int MAX_CRATER_COUNT = 1024;
};