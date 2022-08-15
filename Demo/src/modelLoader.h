#ifndef MODELLOADER_H
#define MODELLOADER_H

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <meshoptimizer.h>
#include <glad/glad.h>
#include <vector>
#include <unordered_map>
struct Vertex3D {
	float vx, vy, vz;
	float nx, ny, nz;
	float u, v;
};
struct DrawElementIndirectCommand {
	GLuint vertexCount;
	GLuint instanceCount;
	GLuint firstIndex;
	GLint baseVertex;
	GLuint baseInstance;
};
struct ModelData {
	glm::mat4 modelMat{ glm::mat4() };
	GLuint isDraw;
	GLuint modelID;
	GLuint clusterCount;
	GLuint materialID;
};
struct MeshletData {
	glm::vec3 center;
	float radius;
	glm::vec3 color;
	GLuint clusterID;
	glm::vec3 coneNorm;
	GLuint isDraw;
	glm::vec3 coneApex;
	float coneAngle;
};
struct BoundingBox {
	glm::vec3 minPos{ glm::vec3(std::numeric_limits<float>().max()) };
	glm::vec3 maxPos{ glm::vec3(std::numeric_limits<float>().min()) };
};

class Mesh {
public:
	std::vector<Vertex3D> vertices;
	std::vector<uint32_t> indices;
	std::vector<DrawElementIndirectCommand> drawCmds;
	std::vector<MeshletData> meshletDatas;
	GLuint VAO;
	GLuint drawCmdsSSBO;
	GLuint meshletDataSSBO;
	BoundingBox bbox;

	Mesh() = default;
	Mesh(const std::vector<Vertex3D>& vert, const std::vector<uint32_t>& index, const std::vector<DrawElementIndirectCommand> Cmds, const std::vector<MeshletData>& meshlets, const BoundingBox& box) {
		this->vertices = vert;
		this->indices = index;
		this->drawCmds = Cmds;
		this->meshletDatas = meshlets;
		this->bbox = box;

		SetUp();
	}
	void ClearUp() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteBuffers(1, &drawCmdsSSBO);
		glDeleteBuffers(1, &meshletDataSSBO);
	}
	void DrawVisible(size_t drawNum, size_t fixedNum) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshletDataSSBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)0, drawNum, 0);
		if(fixedNum != 0) 
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)((drawCmds.size() - fixedNum) * 5 * sizeof(GLuint)), fixedNum, 0);
	}
	void HizCulling() {	
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshletDataSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, drawCmdsSSBO);
		glDispatchCompute((drawCmds.size() + 255) / 256, 1, 1);
	}
	void DrawFixed(size_t drawNum) {
		if (drawNum == 0) return;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshletDataSSBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)((drawCmds.size() - drawNum)* 5 * sizeof(GLuint)), drawNum, 0);
	}
	void DrawNoCulling(size_t drawNum) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshletDataSSBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, drawNum, 0);
	}
private:
	GLuint EBO, VBO;
	void SetUp() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3D) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (GLvoid*)offsetof(Vertex3D, vx));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (GLvoid*)offsetof(Vertex3D, nx));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (GLvoid*)offsetof(Vertex3D, u));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glGenBuffers(1, &drawCmdsSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCmdsSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementIndirectCommand) * drawCmds.size(), drawCmds.data(), GL_DYNAMIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &meshletDataSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshletDataSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MeshletData) * meshletDatas.size(), meshletDatas.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	}
};
class Model {
public:
	Mesh mesh;
	Model(const char* filepath)
		:mesh{ProcessMesh(filepath)}
	{}
	void DrawVisble(size_t drawNum, size_t fixedNum) {
		mesh.DrawVisible(drawNum, fixedNum);
	}
	void HizCulling() {
		mesh.HizCulling();
	}
	void DrawFixed(size_t drawNum) {
		mesh.DrawFixed(drawNum);
	}
	void DrawNoCulling(size_t drawNum) {
		mesh.DrawNoCulling(drawNum);
	}
private:
	Mesh ProcessMesh(const char* filepath) {
		tinyobj::ObjReaderConfig reader_config;
		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(filepath, reader_config)) {
			if (!reader.Error().empty()) {
				std::cout << "TinyObjReader: " << reader.Error() << std::endl;
			}
			if (!reader.Warning().empty()) {
				std::cout << "TinyObjReader: " << reader.Warning() << std::endl;
			}
		}
		auto& attrib = reader.GetAttrib();
		auto& shape = reader.GetShapes();
		auto& material = reader.GetMaterials();
		
		std::vector<Vertex3D> vertices;
		BoundingBox box;
		size_t index_count = 0;
		size_t faceCounter = 0;
		for (size_t s = 0; s < shape.size(); s++) {
			size_t index_offset = 0;
			faceCounter += shape[s].mesh.num_face_vertices.size();
			for (size_t f = 0; f < shape[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shape[s].mesh.num_face_vertices[f]);				
				for (size_t v = 0; v < fv; v++) {
					tinyobj::index_t idx = shape[s].mesh.indices[index_offset + v];
					Vertex3D vert;
					vert.vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					vert.vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					vert.vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

					box.minPos = glm::min(box.minPos, glm::vec3(vert.vx, vert.vy, vert.vz));
					box.maxPos = glm::max(box.maxPos, glm::vec3(vert.vx, vert.vy, vert.vz));

					if (idx.normal_index >= 0) {
						vert.nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
						vert.ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
						vert.nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					}

					if (idx.texcoord_index >= 0) {
						vert.u = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						vert.v = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					}

					vertices.push_back(vert);
					index_count++;
				}
				index_offset += fv;
			}
		}

		std::vector<uint32_t> remap(index_count);
		size_t vertex_count = meshopt_generateVertexRemap(remap.data(), nullptr, index_count, vertices.data(), vertices.size(), sizeof(Vertex3D));
		std::vector<uint32_t> remapIndices(index_count);
		std::vector<Vertex3D> remapVertices(vertex_count);
		meshopt_remapIndexBuffer(remapIndices.data(), nullptr, index_count, remap.data());
		meshopt_remapVertexBuffer(remapVertices.data(), vertices.data(), index_count, sizeof(Vertex3D), remap.data());

		const size_t max_vertices = 64;
		const size_t max_triangles = 128;
		const float cone_weight = 0.f;

		size_t max_meshlets = meshopt_buildMeshletsBound(index_count, max_vertices, max_triangles);
		std::vector<meshopt_Meshlet> meshlets(max_meshlets);
		std::vector<unsigned int> meshlet_vertices(max_vertices * max_meshlets);
		std::vector<unsigned char> meshlet_triangles(max_triangles * 3 * max_meshlets);
		size_t meshlet_count = meshopt_buildMeshlets(
			meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(), 
			remapIndices.data(), remapIndices.size(),
			&remapVertices[0].vx, remapVertices.size(), sizeof(Vertex3D), 
			max_vertices, max_triangles, cone_weight
		);

		const auto& last = meshlets[meshlet_count - 1];
		meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
		meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
		meshlets.resize(meshlet_count);

		std::vector<MeshletData> meshletDatas(meshlet_count);
		std::vector<DrawElementIndirectCommand> cmds(meshlet_count);
		std::vector<uint32_t> finalIndex;
		GLuint counter = 0;
		for (size_t i = 0; i < meshlets.size();++i) {
			meshopt_Bounds bound = meshopt_computeMeshletBounds(
				&meshlet_vertices[meshlets[i].vertex_offset],
				&meshlet_triangles[meshlets[i].triangle_offset],
				meshlets[i].triangle_count,
				&remapVertices[0].vx, remapVertices.size(), sizeof(Vertex3D)
			);
			meshletDatas[i].center = glm::vec3(bound.center[0], bound.center[1], bound.center[2]);
			meshletDatas[i].radius = bound.radius;
			meshletDatas[i].clusterID = i;
			meshletDatas[i].color = glm::vec3(rand() % 100 / 100.f, rand() % 100 / 100.f, rand() % 100 / 100.f);
			meshletDatas[i].isDraw = 1;
			meshletDatas[i].coneNorm = glm::vec3(bound.cone_axis[0], bound.cone_axis[1], bound.cone_axis[2]);
			meshletDatas[i].coneApex = glm::vec3(bound.cone_apex[0], bound.cone_apex[1], bound.cone_apex[2]);
			meshletDatas[i].coneAngle = bound.cone_cutoff;
			auto tmpVert = &meshlet_vertices[meshlets[i].vertex_offset];
			auto tmpTri = &meshlet_triangles[meshlets[i].triangle_offset];
			for (size_t j = 0; j < meshlets[i].triangle_count * 3; ++j) {
				auto index = tmpVert[tmpTri[j]];
				finalIndex.push_back(index);
			}
			cmds[i].vertexCount = meshlets[i].triangle_count * 3;
			cmds[i].instanceCount = 1;
			cmds[i].firstIndex = counter;
			cmds[i].baseVertex = 0;
			cmds[i].baseInstance = i;
			counter += meshlets[i].triangle_count * 3;
		}

		return Mesh(remapVertices, finalIndex, cmds, meshletDatas, box);
	}
};

class ModelBatch {
private:
	struct ModelDataBuffer {
		Model* model;
		ModelData modelData;
		GLuint dataSSBO;
		GLuint visibleLogSSBO;
		GLuint visibleListSSBO;
		GLuint counterAC;
		uint32_t visibleCounter;
		uint32_t fixedCounter;
		bool isDraw{ true };
	};

	struct MaterialData {
		float diffuse;
		float specular;
		float ambient;
		float amplify;
	};

	GLuint materialUBO;
	std::vector<MaterialData> materials;
	std::unordered_multimap<uint32_t, ModelDataBuffer> opaqueObj;

	size_t visibleNum;
	size_t fixedNum;
	size_t totalNum;
	size_t visibleObjCounter;
	glm::vec4 boxcorner[8];
	glm::vec4 minPos;
	glm::vec4 maxPos;
	glm::vec4 center;
	float dist;
public:
	ModelBatch() = default;

	void LoadModel(Model* model, uint32_t materialID,	float scale = 1.f, glm::vec3 translate = glm::vec3(0.f), glm::vec4 rotate = glm::vec4(0.f)) {
		ModelDataBuffer buffer;
		buffer.model = model;

		buffer.modelData.modelMat = glm::translate(glm::mat4(1.f), translate) * glm::scale(glm::mat4(1.f), glm::vec3(scale));
		buffer.modelData.isDraw = 1;
		buffer.modelData.clusterCount = model->mesh.drawCmds.size();
		buffer.modelData.modelID = opaqueObj.size();
		buffer.modelData.materialID = materialID;
		
		glGenBuffers(1, &buffer.dataSSBO);
		glBindBuffer(GL_UNIFORM_BUFFER, buffer.dataSSBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ModelData), &buffer.modelData, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		std::vector<uint32_t> log(model->mesh.drawCmds.size(), 1);
		buffer.visibleCounter = log.size();
		glGenBuffers(1, &buffer.visibleLogSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.visibleLogSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * log.size(), log.data(), GL_DYNAMIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &buffer.visibleListSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.visibleListSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementIndirectCommand) * model->mesh.drawCmds.size(), model->mesh.drawCmds.data(), GL_DYNAMIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &buffer.counterAC);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buffer.counterAC);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 2, nullptr, GL_DYNAMIC_COPY);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

		opaqueObj.emplace(std::pair<uint32_t, ModelDataBuffer>(materialID, buffer));

		totalNum += model->mesh.drawCmds.size();
	}

	void LoadMaterials(float diff, float spec, float ambi, float ampli) {
		MaterialData data = { diff, spec, ambi, ampli };
		materials.push_back(data);
	}
	void SetUp() {
		glGenBuffers(1, &materialUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialData), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	void DrawVisible() {
		for (size_t i = 0; i < materials.size(); ++i) {
			glBindBufferBase(GL_UNIFORM_BUFFER, 2, materialUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialData), &materials[i]);
			auto range = opaqueObj.equal_range((uint32_t)i);
			for (auto obj = range.first; obj != range.second; ++obj) {
				if (!obj->second.isDraw || obj->second.visibleCounter == 0) continue;
				glBindBufferBase(GL_UNIFORM_BUFFER, 1, obj->second.dataSSBO);
				glBindBuffer(GL_DRAW_INDIRECT_BUFFER, obj->second.visibleListSSBO);
				obj->second.model->DrawVisble(obj->second.visibleCounter, obj->second.fixedCounter);
			}
		}
	}

	void HizCulling(const glm::mat4 vpMat, const glm::vec3& cameraPos, float projFeild) {
		for (auto& obj : opaqueObj) {
			minPos = obj.second.modelData.modelMat * glm::vec4(obj.second.model->mesh.bbox.minPos, 1.f);
			maxPos = obj.second.modelData.modelMat * glm::vec4(obj.second.model->mesh.bbox.maxPos, 1.f);
			if (isFrustumCulled(boxcorner, minPos, maxPos, vpMat)) {
				obj.second.isDraw = false;
				continue;
			}
			else {
				obj.second.isDraw = true;
				visibleObjCounter++;
				//center = (boxcorner[0] + boxcorner[7]) / 2.f;
				//dist = glm::distance(cameraPos, glm::vec3(center.x, center.y, center.z));
				
			}
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, obj.second.dataSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, obj.second.visibleListSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, obj.second.visibleLogSSBO);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, obj.second.counterAC);
			glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
			obj.second.model->HizCulling();
			glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
			glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &obj.second.visibleCounter);
			glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), sizeof(GLuint), &obj.second.fixedCounter);
			visibleNum += obj.second.visibleCounter;
			fixedNum += obj.second.fixedCounter;
		}
#ifdef _DEBUG
		std::cout << "Visible Num: " << visibleNum + fixedNum << "("<< (visibleNum + fixedNum) * 100.f / totalNum << "%)" << std::endl;
		std::cout << "Fixed Num: " << fixedNum << std::endl;
		std::cout << "Visible Objects: " << visibleObjCounter << std::endl;
#endif // _DEBUG
		visibleNum = 0;
		fixedNum = 0;
		visibleObjCounter = 0;
	}

	void DrawFixed() {
		for (size_t i = 0; i < materials.size(); ++i) {
			glBindBufferBase(GL_UNIFORM_BUFFER, 2, materialUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialData), &materials[i]);
			auto range = opaqueObj.equal_range((uint32_t)i);
			for (auto obj = range.first; obj != range.second; ++obj) {
				if (!obj->second.isDraw || obj->second.fixedCounter == 0) continue;
				glBindBufferBase(GL_UNIFORM_BUFFER, 1, obj->second.dataSSBO);
				glBindBuffer(GL_DRAW_INDIRECT_BUFFER, obj->second.visibleListSSBO);
				obj->second.model->DrawFixed(obj->second.fixedCounter);
			}
		}
	}
private:
	bool isFrustumCulled(glm::vec4* boxcorner, glm::vec4& minPos, glm::vec4& maxPos, const glm::mat4& vpMat) {
		boxcorner[0] = vpMat * glm::vec4(minPos.x, minPos.y, minPos.z, 1.0);
		boxcorner[1] = vpMat * glm::vec4(maxPos.x, minPos.y, minPos.z, 1.0);
		boxcorner[2] = vpMat * glm::vec4(minPos.x, maxPos.y, minPos.z, 1.0);
		boxcorner[3] = vpMat * glm::vec4(maxPos.x, maxPos.y, minPos.z, 1.0);
		boxcorner[4] = vpMat * glm::vec4(minPos.x, minPos.y, maxPos.z, 1.0);
		boxcorner[5] = vpMat * glm::vec4(maxPos.x, minPos.y, maxPos.z, 1.0);
		boxcorner[6] = vpMat * glm::vec4(minPos.x, maxPos.y, maxPos.z, 1.0);
		boxcorner[7] = vpMat * glm::vec4(maxPos.x, maxPos.y, maxPos.z, 1.0);

		uint8_t bit = 0;
		uint8_t resBit = 0;
		bit |= boxcorner[0].x >  boxcorner[0].w ? 1 : 0;
		bit |= boxcorner[0].x < -boxcorner[0].w ? 2 : 0;
		bit |= boxcorner[0].y >  boxcorner[0].w ? 4 : 0;
		bit |= boxcorner[0].y < -boxcorner[0].w ? 8 : 0;
		bit |= boxcorner[0].z >  boxcorner[0].w ? 16 : 0;
		bit |= boxcorner[0].z < -boxcorner[0].w ? 32 : 0;
		bit |= boxcorner[0].w <= 0 ? 64 : 0;
		resBit = bit;
		for (size_t i = 1; i < 8; ++i) {
			bit = 0;
			bit |= boxcorner[i].x >  boxcorner[i].w ? 1 : 0;
			bit |= boxcorner[i].x < -boxcorner[i].w ? 2 : 0;
			bit |= boxcorner[i].y >  boxcorner[i].w ? 4 : 0;
			bit |= boxcorner[i].y < -boxcorner[i].w ? 8 : 0;
			bit |= boxcorner[i].z >  boxcorner[i].w ? 16 : 0;
			bit |= boxcorner[i].z < -boxcorner[i].w ? 32 : 0;
			bit |= boxcorner[i].w <= 0 ? 64 : 0;
			resBit &= bit;
		}
		return resBit != 0;
	}



};

#endif