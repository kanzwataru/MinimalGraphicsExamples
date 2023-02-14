#include "glad/glad.h"
#include "glfw3.h"
#include "linmath.h"
#include <math.h>
#include <stdio.h>

#pragma comment (lib, "extern/glfw3_mt.lib")

const char *vertex_shader_src = 
	"#version 450 core																	\n"
	"																					\n"
	"layout (location = 0) in vec3 in_position;											\n"
	"layout (location = 1) in uvec4 in_joint_indices;									\n"
	"layout (location = 2) in vec4 in_joint_weights;									\n"
	"																					\n"
	"layout (location = 0) uniform mat4 mvp_matrix;										\n"
	"																					\n"
	"layout(std430, binding = 0) readonly buffer SkinBuffer {							\n" // Buffer for the skin matrices
	"	mat4 bone_matrices[];															\n"
	"};																					\n"
	"																					\n"
	"void main()																		\n"
	"{																					\n"
	"	vec4 pos = vec4(in_position, 1.0);												\n"
	"	vec4 skinned_pos = vec4(0, 0, 0, 0);											\n"
	"																					\n"
	"	skinned_pos += in_joint_weights.x * (bone_matrices[in_joint_indices.x] * pos);	\n" // Calculate all the skinned positions
	"	skinned_pos += in_joint_weights.y * (bone_matrices[in_joint_indices.y] * pos);	\n" // and add them all together
	"	skinned_pos += in_joint_weights.z * (bone_matrices[in_joint_indices.z] * pos);	\n" // weighted by the joint weight.
	"	skinned_pos += in_joint_weights.w * (bone_matrices[in_joint_indices.w] * pos);	\n"
	"																					\n"
	"	gl_Position = mvp_matrix * skinned_pos;											\n" // Finally, transform into projected space
	"}																					\n";

const char *fragment_shader_src = 
	"#version 450 core																	\n"
	"layout (location = 0) out vec4 out_color;											\n"
	"																					\n"
	"void main()																		\n"
	"{																					\n"
	"	out_color = vec4(0.75, 0.5, 0.25, 1.0);											\n"
	"}																					\n";

int main(int argc, char **argv)
{
	// 1. Create window and initialize GL
	const int width = 800;
	const int height = 600;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(width, height, "Minimal GPU Skinning", NULL, NULL);
	
	glfwMakeContextCurrent(window);
	gladLoadGLLoader(glfwGetProcAddress);

	// 2. Load shaders and create resources
	// * Shaders
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
	glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
	glCompileShader(fragment_shader);

	GLuint shader = glCreateProgram();
	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);

	// * Geometry
	struct Vertex {
		float position[3];
		uint16_t joint_indices[4];		// Which joint to weight to, maximum of 4.
		float joint_weights[4];			// The weight of each joint.
	};

	struct Vertex mesh_vertices[] = {
		{{  0.50f,  0.00f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}}, // Bottom quad, fully weighted to first joint
		{{ -0.50f,  0.75f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},
		{{ -0.50f,  0.00f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},
		{{  0.50f,  0.00f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},
		{{  0.50f,  0.75f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},
		{{ -0.50f,  0.75f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},

		{{  0.50f,  0.75f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}}, // Bottom-middle quad, weighted mostly to first joint, and a little bit to second
		{{ -0.50f,  1.00f,  0.00f }, {0, 1, 0, 0}, {0.75f, 0.25f, 0.00f, 0.00f}},
		{{ -0.50f,  0.75f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},
		{{  0.50f,  0.75f,  0.00f }, {0, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},
		{{  0.50f,  1.00f,  0.00f }, {0, 1, 0, 0}, {0.75f, 0.25f, 0.00f, 0.00f}},
		{{ -0.50f,  1.00f,  0.00f }, {0, 1, 0, 0}, {0.75f, 0.25f, 0.00f, 0.00f}},

		{{  0.50f,  1.00f,  0.00f }, {0, 1, 0, 0}, {0.75f, 0.25f, 0.00f, 0.00f}}, // Bottom-top quad, weighted mostly to second joint, and a little bit to first
		{{ -0.50f,  1.25f,  0.00f }, {0, 1, 0, 0}, {0.25f, 0.75f, 0.00f, 0.00f}},
		{{ -0.50f,  1.00f,  0.00f }, {0, 1, 0, 0}, {0.75f, 0.25f, 0.00f, 0.00f}},
		{{  0.50f,  1.00f,  0.00f }, {0, 1, 0, 0}, {0.75f, 0.25f, 0.00f, 0.00f}},
		{{  0.50f,  1.25f,  0.00f }, {0, 1, 0, 0}, {0.25f, 0.75f, 0.00f, 0.00f}},
		{{ -0.50f,  1.25f,  0.00f }, {0, 1, 0, 0}, {0.25f, 0.75f, 0.00f, 0.00f}},

		{{  0.50f,  1.25f,  0.00f }, {0, 1, 0, 0}, {0.25f, 0.75f, 0.00f, 0.00f}}, // Top quad, the top vertices are fully driven by the second joint
		{{ -0.50f,  2.00f,  0.00f }, {1, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},
		{{ -0.50f,  1.25f,  0.00f }, {0, 1, 0, 0}, {0.25f, 0.75f, 0.00f, 0.00f}},
		{{  0.50f,  1.25f,  0.00f }, {0, 1, 0, 0}, {0.25f, 0.75f, 0.00f, 0.00f}},
		{{  0.50f,  2.00f,  0.00f }, {1, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}},
		{{ -0.50f,  2.00f,  0.00f }, {1, 0, 0, 0}, {1.00f, 0.00f, 0.00f, 0.00f}}
	};

	GLuint vbo;
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(mesh_vertices), mesh_vertices, 0);

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(struct Vertex));

	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(struct Vertex, position));
	glVertexArrayAttribBinding(vao, 0, 0);
	glEnableVertexArrayAttrib(vao, 0);

	glVertexArrayAttribIFormat(vao, 1, 4, GL_UNSIGNED_SHORT, offsetof(struct Vertex, joint_indices));
	glVertexArrayAttribBinding(vao, 1, 0);
	glEnableVertexArrayAttrib(vao, 1);

	glVertexArrayAttribFormat(vao, 2, 4, GL_FLOAT, GL_FALSE, offsetof(struct Vertex, joint_weights));
	glVertexArrayAttribBinding(vao, 2, 0);
	glEnableVertexArrayAttrib(vao, 2);

	// * Bones
	#define BONE_COUNT 2
	mat4x4 bind_pose[BONE_COUNT] = { // World-space initial position of the joints. This is where the joints are placed along the mesh, in the default position ("bind pose").
		{ // First joint
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		},
		{ // Second joint
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0.75, 0, 1,
		}
	};

	int parent_indices[BONE_COUNT] = { // The index for each joint's parent if any. NOTE: All of the code assumes that all joints are stored after their parents in the list, and not before.
		-1, // First joint is not parented to anything
		0   // Second joint is parented to the first joint
	};

	GLuint skin_buffer;
	glCreateBuffers(1, &skin_buffer);
	glNamedBufferStorage(skin_buffer, sizeof(bind_pose), NULL, GL_DYNAMIC_STORAGE_BIT);

	// * Camera matrices
	mat4x4 view_matrix = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, -1, -3, 1
	};

	mat4x4 proj_matrix;
	mat4x4_perspective(proj_matrix, 1.5f, (float)width / (float)height, 0.01f, 100.0f);

	mat4x4 mvp_matrix;
	mat4x4_mul(mvp_matrix, proj_matrix, view_matrix);

	// 3. Render loop
	glfwSetTime(0.0f);
	while(!glfwWindowShouldClose(window)) {
		double elapsed_time = glfwGetTime();

		// * Update

		// 1. First "animate" the joints, computing their local transforms.
		mat4x4 local_matrices[BONE_COUNT]; // For each joint, the local transform (equivalent to the values an artist would animate in an animation software).
										   // Identity matrix here would means the joint is at its default position, relative to the parent.

		{ // First bone, animate a slow rotation
			quat rotation;
			quat_rotate(rotation, (float)fmod(elapsed_time * 0.35, 2 * 3.1415), (vec3){ 0, 1, 0 });
			mat4x4_from_quat(local_matrices[0], rotation);
		}

		{ // Second bone, animate a fast rotation
			quat rotation;
			quat_rotate(rotation, (float)sin(elapsed_time * 4.0f), (vec3){ 1, 0, 0 });
			mat4x4_from_quat(local_matrices[1], rotation);
		}

		// 2. Second, we need to flatten the hierarchy and get the world-space transforms.
		//    NOTE: This is NOT the world-space in the sense that the mesh's model matrix is applied, but in the sense that the joint is unparented.
		mat4x4 world_matrices[BONE_COUNT]; // For each joint, the transform in world space.
		for(int i = 0; i < BONE_COUNT; ++i) {
			// NOTE: Since we are accessing the parents here, it is assumed that the joints are stored
			//       in a pre-sorted order where the children are always someplace after the parents.
			//       This way we don't need to traverse a tree everytime we evaluate the animation.
			int parent = parent_indices[i];

			if(parent > -1) {
				// * Joint with parent, calculate world matrix
				
				// Calculate this joint's bind-pose transform relative to the bind-pose parent transform
				mat4x4 bind_pose_parent_inverse_mat;
				mat4x4_invert(bind_pose_parent_inverse_mat, bind_pose[parent]);

				mat4x4 bind_pose_parent_offset_mat;
				mat4x4_mul(bind_pose_parent_offset_mat, bind_pose_parent_inverse_mat, bind_pose[i]);

				// Apply the relative transform above to the current pose's parent, to get the current default transform in world-space
				mat4x4 parent_offset_mat;
				mat4x4_mul(parent_offset_mat, world_matrices[parent], bind_pose_parent_offset_mat);

				// Apply the animation pose's transform, to get the final world-space transform
				mat4x4_mul(world_matrices[i], parent_offset_mat, local_matrices[i]);
			}
			else {
				// * Joint with no parent, the local matrix is the same as the world matrix
				mat4x4_dup(world_matrices[i], local_matrices[i]);
			}
		}

		// 3. Lastly, we need to get the joint transforms relative to their bind pose, so that the mesh doesn't get doubly-transformed.
		mat4x4 bind_space_matrices[BONE_COUNT];
		for(int i = 0; i < BONE_COUNT; ++i) {
			// Calculate this joint's world-space transform relative to the world-space bind-pose (-> bind-space)
			mat4x4 inverse_bind_pose_mat;
			mat4x4_invert(inverse_bind_pose_mat, bind_pose[i]);

			mat4x4_mul(bind_space_matrices[i], world_matrices[i], inverse_bind_pose_mat);
		}

		glNamedBufferSubData(skin_buffer, 0, sizeof(bind_space_matrices), bind_space_matrices); // Updating the buffer here

		// * Render
		glClearColor(0.25f, 0.35f, 0.45f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(shader);
		glUniformMatrix4fv(0, 1, GL_FALSE, &mvp_matrix[0][0]);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, skin_buffer);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(mesh_vertices) / sizeof(mesh_vertices[0]));

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
