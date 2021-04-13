#include "CelestialBody.h"
#include "../Rendering/GlMacro.h"
#include "../Keyboard.h"

CelestialBody::CelestialBody(const std::shared_ptr<Program> renderingProgram,
	const std::shared_ptr<Program> terrainGeneratorProgram, const Vector3& position,
	float scale, float cellSideLength, const std::string& dynamicVariableManager)
	:
	mRenderingProgram(renderingProgram),
	mTerrainGeneratorProgram(terrainGeneratorProgram),
	mPosition(position),
	mScale(scale),
	mDynamicVariables(dynamicVariableManager)
{
	// The radius of the model needs to be equal to
	// 1 for our calculations to work out. The only reason
	// why this variable exists is because a variable name
	// is often easier to read and understand than a raw value.
	assert(mDimensions.MODEL_RADIUS == 1.0f);

	if (!msInitializedTextures)
	{
		msInitializedTextures = true;
		msTextures.emplace("CraterEjectaRay", "AsteroidNormal", "RockCliffNormal");
	}

	mDimensions.sideLengthInCells = int(mDimensions.MODEL_DIAMETER / cellSideLength);
	// Make sure that the side length is not 0
	assert(mDimensions.sideLengthInCells >= 1);

	mDimensions.cellSideLength = mDimensions.MODEL_DIAMETER / (float)mDimensions.sideLengthInCells;

	// The shader storage buffer object needs to be initialized
	// before we initialize the vbo
	InitializeShaderStorageBufferObject();

	// The vbo needs to be initialized
	// before we initialize the vao
	InitializeVbo();

	InitializeVao();
}

CelestialBody::~CelestialBody()
{
	// We do not want to throw an exception inside a destructor. 
	// Hence, we do not use the macro "GL".
	glDeleteVertexArrays(1, &mVao);
	glDeleteBuffers(1, &mVbo);
	glDeleteBuffers(1, &mShaderStorageBufferObject);
}

void CelestialBody::Render(const Camera& camera, const Matrix4& projectionMatrix) const
{
	mRenderingProgram->Bind();
	GL(glBindVertexArray(mVao));

	// Bind all the textures
	msTextures->BindPermutationMap(0);
	msTextures->BindTexture(1);
	msTextures->BindCraterTexture(2);
	msTextures->BindCraterSampler(2);
	msTextures->BindNormalMaps(3, 4);
	msTextures->BindNormalInterpolation(5);

	BindUniforms(camera, projectionMatrix);

	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mSphereVertices.size());

	// We bind the default sampler, since we do not want
	// the crater sampler, bound above, to still be bound
	msTextures->BindDefaultSampler(2);
}

void CelestialBody::Update(float deltaTime)
{
	bool variablesChanged = mDynamicVariables.UpdateValueKeyboard(deltaTime);

	if (variablesChanged)
	{
		// Update the vertices, if the user has changed
		// the variables
		UpdateVboVertices(mSphereVertices);
	}
}

Vector3 CelestialBody::GetPosition() const
{
	return mPosition;
}

float CelestialBody::GetRadius() const
{
	// Return the radius of the rendered celestial body
	return mDimensions.MODEL_RADIUS * mScale;
}

std::vector<CelestialVertex> CelestialBody::GetFaceVertices(const Vector3& lowerLeftCornerOfFace,
	const Vector3& tangent, const Vector3& binormal) const
{
	// A vector that (when added) moves a 
	// position one cell to the right
	Vector3 toRight = tangent * mDimensions.cellSideLength;
	// A vector that (when added) moves a 
	// position one cell up
	Vector3 toUp = binormal * mDimensions.cellSideLength;

	std::vector<CelestialVertex> vertices;
	// There are 2 triangles per cell and 3 vertices per triangle
	vertices.reserve(mDimensions.sideLengthInCells * mDimensions.sideLengthInCells * 2 * 3);
	for (int y = 0; y < mDimensions.sideLengthInCells; ++y)
	{
		for (int x = 0; x < mDimensions.sideLengthInCells; ++x)
		{
			const CelestialVertex lowerLeft = GetVertex(x, y, toRight, toUp, lowerLeftCornerOfFace);
			const CelestialVertex lowerRight = GetVertex(x + 1, y, toRight, toUp, lowerLeftCornerOfFace);
			const CelestialVertex upperRight = GetVertex(x + 1, y + 1, toRight, toUp, lowerLeftCornerOfFace);
			const CelestialVertex upperLeft = GetVertex(x, y + 1, toRight, toUp, lowerLeftCornerOfFace);
			
			// First face
			vertices.push_back(lowerLeft);
			vertices.push_back(lowerRight);
			vertices.push_back(upperLeft);

			// Second face
			vertices.push_back(lowerRight);
			vertices.push_back(upperRight);
			vertices.push_back(upperLeft);
		}
	}
	
	return vertices;
}

CelestialVertex CelestialBody::GetVertex(const int xOffset, const int yOffset, const Vector3& toRight,
	const Vector3& toUp, const Vector3& lowerLeftCornerOfFace) const
{
	CelestialVertex vertex;

	// Calculate the position
	Vector3 vertexPosition = lowerLeftCornerOfFace;
	vertexPosition += toRight * (float)xOffset + toUp * (float)yOffset;
	vertexPosition.Normalize();
	// We force the vertex to have a position that has
	// a distance of "MODEL_RADIUS" to the origin, and as 
	// a consequence, effectively turning the cube into 
	// a sphere
	vertexPosition *= mDimensions.MODEL_RADIUS;
	vertex.position = TightlyPackedVector3(vertexPosition);

	// Calculate the normal
	Vector3 normal = vertexPosition.GetNormalized();
	vertex.normal = TightlyPackedVector3(normal);

	return vertex;
}

std::vector<CelestialVertex> CelestialBody::GetSphereVertices() const
{
	// Will contain vertices whose positions form the shape of 
	// a sphere, but originally hade the shape of a cube
	std::vector<std::vector<CelestialVertex>> cubeVertices;

	// Front face
	cubeVertices.push_back(
		GetFaceVertices({ -mDimensions.MODEL_RADIUS, -mDimensions.MODEL_RADIUS, mDimensions.MODEL_RADIUS },
			{ 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));

	// Back face
	cubeVertices.push_back(
		GetFaceVertices({ mDimensions.MODEL_RADIUS, -mDimensions.MODEL_RADIUS, -mDimensions.MODEL_RADIUS },
			{ -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));

	// Left face
	cubeVertices.push_back(
		GetFaceVertices({ -mDimensions.MODEL_RADIUS, -mDimensions.MODEL_RADIUS, -mDimensions.MODEL_RADIUS },
			{ 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }));

	// Right face
	cubeVertices.push_back(
		GetFaceVertices({ mDimensions.MODEL_RADIUS, -mDimensions.MODEL_RADIUS, mDimensions.MODEL_RADIUS },
			{ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));

	// Top face
	cubeVertices.push_back(
		GetFaceVertices({ -mDimensions.MODEL_RADIUS, mDimensions.MODEL_RADIUS, mDimensions.MODEL_RADIUS },
			{ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }));

	// Bottom face
	cubeVertices.push_back(
		GetFaceVertices({ -mDimensions.MODEL_RADIUS, -mDimensions.MODEL_RADIUS, -mDimensions.MODEL_RADIUS },
			{ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }));

	std::vector<CelestialVertex> vertices;
	// All the sides have the same amount of vertices, hence we can multiply by 6
	// to get the total amount of vertices
	vertices.reserve(cubeVertices.front().size() * 6);

	// Move all the vertices of the cube into a continuous vector of vertices
	for (auto faceVertices : cubeVertices)
	{
		vertices.insert(vertices.end(), std::move_iterator(faceVertices.begin()),
			std::move_iterator(faceVertices.end()));
	}

	return vertices;
}

std::vector<TightlyPackedVector3> CelestialBody::GetCraterPositions(const int nCraters,
	const std::vector<CelestialVertex>& vertices) const
{
	std::vector<CelestialVertex> craterVertices;
	craterVertices.resize(nCraters);

	// Randomly select the vertices that will give birth to the craters
	std::sample(vertices.begin(), vertices.end(), craterVertices.begin(), nCraters,
		std::mt19937{ std::random_device{}() });
	
	std::vector<TightlyPackedVector3> craterPositions;
	craterPositions.resize(nCraters);

	// Get the positions of the randomly selected vertices
	std::transform(craterVertices.begin(), craterVertices.end(), craterPositions.begin(),
		[](const CelestialVertex& vertex)
		{
			return vertex.position;
		});

	return craterPositions;
}

std::vector<float> CelestialBody::GetRandomCraterValues(const int nCraters) const
{
	std::random_device randomNumberGenerator;
	std::mt19937 randomNumberEngine(randomNumberGenerator());
	// The random crater values should range from 0 to 1
	std::uniform_real_distribution<float> distributor(0.0f, 1.0f);
	
	std::vector<float> randomValues;
	std::generate_n(std::back_inserter(randomValues), nCraters,
		std::bind(distributor, std::ref(randomNumberEngine)));

	return randomValues;
}

std::vector<bool> CelestialBody::GetCraterTextureBools(
	const std::vector<TightlyPackedVector3>& craterPositions, 
	const int nWantedCraterTextures,
	const float maxCraterTextureRadius) const
{
	assert(nWantedCraterTextures <= craterPositions.size());

	// The positions of the craters that should have a texture applied to it
	std::vector<std::pair<Vector3, int>> texturePositions;
	for (int i = 0; i < nWantedCraterTextures; ++i)
	{
		Vector3 position = (Vector3)craterPositions[i];

		if (std::all_of(texturePositions.begin(), texturePositions.end(),
			[&position, maxCraterTextureRadius](const std::pair<Vector3, int>& positionAndIndex)
			{
				// The dot product will equal the cosine of the angle between the 
				// positions, since both the positions have a length of 1
				const float angle = acos(position.Dot(positionAndIndex.first));

				// The distance between two points on a sphere is equal to
				// the angle between the two points times the radius. Since the radius
				// of the celestial body is 1, the distance between the points is simply 
				// equal to the angle.
				return angle > maxCraterTextureRadius;
			}))
		{
			// Only give the crater a texture if its position is far enough away 
			// from "all of" the other textures
			texturePositions.push_back({ position, i });
		}
	}

	std::vector<bool> textureBools;
	// Let all the craters, initially, not have a texture applied to it
	std::fill_n(std::back_inserter(textureBools), craterPositions.size(), false);

	std::for_each(texturePositions.begin(), texturePositions.end(),
		[&textureBools](const std::pair<Vector3, int> positionAndIndex)
		{
			// Only apply the texture to the craters whose index corresponds
			// to the above calculated texture positions
			textureBools[positionAndIndex.second] = true;
		});

	return textureBools;
}

void CelestialBody::RunTerrainGeneratorProgram(const size_t nVertices, const GLuint uniformBufferObject,
	const int nCraters, const float maxCraterTextureRadius)
{
	mTerrainGeneratorProgram->Bind();

	// We give the compute shader access to all the vertices, by binding
	// the shader storage buffer object
	GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mShaderStorageBufferObject));

	// The permutation map is needed for perlin noise calculations inside the shader
	msTextures->BindPermutationMap(1);

	// The uniform buffer object contains the crater data that is needed for generating
	// the craters
	GL(glBindBufferBase(GL_UNIFORM_BUFFER, 2, uniformBufferObject));

	GL(glUniform1i(0, nCraters));
	GL(glUniform1f(1, maxCraterTextureRadius));

	std::vector<float> dynamicVariables = mDynamicVariables.GetVariables();
	// Pass the dynamic variables (which contain additional parameters for
	// generating the terrain of the celestial body) to the shader
	GL(glUniform1fv(2, (GLsizei)dynamicVariables.size(), &dynamicVariables.front()));
	
	// Execute the compute shader. We know that "nVertices" will
	// be divisible by 36, since we have generated,
	// n (number of cells) * n * 6 * 2 * 3 = n * n * 36, vertices
	// (We have asserted that n is greater than 0).
	GL(glDispatchCompute(GLuint(nVertices / 36), 1, 1));
}

void CelestialBody::UpdateVboVertices(std::vector<CelestialVertex> vertices)
{
	const int nCraters = (int)mDynamicVariables.Get(0);
	const float maxCraterTextureRadius = mDynamicVariables.Get(2);
	
	// We update the shader storage buffer object, so that we are able
	// to access the vertices from the terrain generator program
	UpdateShaderStorageBufferObject(vertices);

	GLuint uniformBufferObject =
		CreateUniformBufferObject(vertices, nCraters, maxCraterTextureRadius);

	// Update the vertices inside the shader storage buffer,
	// by running the terrain generator program
	RunTerrainGeneratorProgram(vertices.size(),
		uniformBufferObject, nCraters, maxCraterTextureRadius);

	// Wait for the vertices inside the shader storage buffer to 
	// get updated
	GL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));

	// Get the updated vertices
	CelestialVertexGlsl* updatedVerticesGlsl =
		(CelestialVertexGlsl*)GL(glMapNamedBufferRange(mShaderStorageBufferObject, NULL,
			vertices.size() * sizeof(CelestialVertexGlsl), GL_MAP_READ_BIT));

	// Tansform the glsl vertices into regular vertices
	std::transform(updatedVerticesGlsl, updatedVerticesGlsl + vertices.size(), vertices.begin(),
		[](const CelestialVertexGlsl& vertex)
		{
			return CelestialVertex{ vertex.position, vertex.uv, vertex.normal };
		});

	// We are done reading from the shader storage buffer
	GL(glUnmapNamedBuffer(mShaderStorageBufferObject));

	// Finally, update the vbo with the newly updated vertices
	GL(glNamedBufferData(mVbo, sizeof(CelestialVertex) * vertices.size(), &vertices.front(), GL_STATIC_DRAW));

	// We have updated the vertices and do not need the uniform buffer anymore
	GL(glDeleteBuffers(1, &uniformBufferObject));
}

void CelestialBody::InitializeVao()
{
	GL(glCreateVertexArrays(1, &mVao));
	GL(glBindVertexArray(mVao));

	GL(glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(CelestialVertex, position)));
	GL(glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(CelestialVertex, uv)));
	GL(glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, offsetof(CelestialVertex, normal)));

	GL(glVertexArrayAttribBinding(mVao, 0, 0));
	GL(glVertexArrayAttribBinding(mVao, 1, 0));
	GL(glVertexArrayAttribBinding(mVao, 2, 0));

	GL(glEnableVertexAttribArray(0));
	GL(glEnableVertexAttribArray(1));
	GL(glEnableVertexAttribArray(2));

	GL(glVertexArrayVertexBuffer(mVao, 0, mVbo, NULL, sizeof(CelestialVertex)));
}

void CelestialBody::InitializeShaderStorageBufferObject()
{
	GL(glCreateBuffers(1, &mShaderStorageBufferObject));
}

void CelestialBody::InitializeVbo()
{
	GL(glCreateBuffers(1, &mVbo));

	mSphereVertices = GetSphereVertices();

	// Update the vertices of the vbo
	UpdateVboVertices(mSphereVertices);
}

std::vector<CraterData> CelestialBody::GetUniformBufferData(const std::vector<CelestialVertex>& vertices,
	const int nCraters, const float maxCraterTextureRadius)
{
	const int nWantedCraterTextures = (int)mDynamicVariables.Get(1);

	std::vector<TightlyPackedVector3> craterPositions = GetCraterPositions(nCraters, vertices);
	std::vector<float> randomValues = GetRandomCraterValues(nCraters);
	std::vector<bool> hasTextureBools =
		GetCraterTextureBools(craterPositions, nWantedCraterTextures, maxCraterTextureRadius);

	assert(craterPositions.size() == nCraters);
	assert(randomValues.size() == nCraters);
	assert(hasTextureBools.size() == nCraters);

	std::vector<CraterData> uniformBufferData;
	uniformBufferData.resize(nCraters);

	// Initialize the uniform block buffer data
	for (int i = 0; i < nCraters; ++i)
	{
		auto& craterData = uniformBufferData[i];
		craterData.position = craterPositions[i];
		craterData.randomValue = randomValues[i];
		craterData.hasTexture = hasTextureBools[i];
	}

	return uniformBufferData;
}

GLuint CelestialBody::CreateUniformBufferObject(const std::vector<CelestialVertex>& vertices,
	const int nCraters, const float maxCraterTextureRadius)
{
	GLuint uniformBufferObject = 0;
	GL(glCreateBuffers(1, &uniformBufferObject));

	std::vector<CraterData> uniformBufferData = GetUniformBufferData(vertices, nCraters, maxCraterTextureRadius);
	// All instances of "CraterData" are aligned the same way as a "vec4" is, hence
	// we multiply by "4 * 4 * 2"
	GL(glNamedBufferData(uniformBufferObject, uniformBufferData.size() * 4 * 4 * 2,
		&uniformBufferData.front(), GL_STATIC_DRAW));

	return uniformBufferObject;
}

void CelestialBody::UpdateShaderStorageBufferObject(const std::vector<CelestialVertex>& vertices) const
{
	// Convert the vector of "CelestialVertex" to a vector of 
	// "CelestialVertexGlsl", i.e. a vector of vertices that 
	// conform to the std430 storage layout
	std::vector<CelestialVertexGlsl> verticesGlsl;
	verticesGlsl.resize(vertices.size());
	std::transform(vertices.begin(), vertices.end(), verticesGlsl.begin(),
		[](const CelestialVertex& vertex)
		{
			return CelestialVertexGlsl{ vertex.position, vertex.uv, vertex.normal };
		});

	// Update the shader storage buffer object with the correctly memory aligned vertices
	GL(glNamedBufferData(mShaderStorageBufferObject, verticesGlsl.size() * sizeof(CelestialVertexGlsl)
		, &verticesGlsl.front(), GL_DYNAMIC_READ));
}

void CelestialBody::BindUniforms(const Camera& camera, const Matrix4& projectionMatrix) const
{
	const Matrix4 viewRotation = 
		(Matrix4)matrix::GetRotation(-camera.GetXRotation(), -camera.GetYRotation(), 0.0f);

	GL(glUniform3fv(0, 1, camera.GetPosition().GetPointerToData()));
	GL(glUniformMatrix4fv(1, 1, GL_FALSE, viewRotation.GetPointerToData()));
	GL(glUniformMatrix4fv(2, 1, GL_FALSE, projectionMatrix.GetPointerToData()));
	GL(glUniform3fv(3, 1, mPosition.GetPointerToData()));
	GL(glUniform1f(4, mScale));	
}