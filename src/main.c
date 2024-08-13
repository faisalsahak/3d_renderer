#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include <stdbool.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "triangle.h"

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = {0,0,0};
// vec3_t cube_rotation = {.x=0,.y=0,.z = 0};

float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;



void setup(void){
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t)*window_width*window_height);

	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
		);

	// loads the cube values in the mesh data structure
	// load_cube_mesh_data();
	load_obj_file_data("./assets/cube.obj");

}

void process_input(void){

	SDL_Event event;
	SDL_PollEvent(&event);

	switch(event.type){
	case SDL_QUIT:
		is_running = false;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE)
			is_running=false;
		break;

	}
}
////////////////////////////////////////////////////////////////////////
//Function that recieves a 3d vector and returns a projected 2d point
///////////////////////////////////////////////////////////////////////
vec2_t project(vec3_t point){
	vec2_t projected_point={
		.x = (fov_factor * point.x)/point.z,
		.y = (fov_factor * point.y)/point.z
	};

	return projected_point;
}


void update(void){
	while(!SDL_TICKS_PASSED(SDL_GetTicks(),previous_frame_time+FRAME_TARGET_TIME));
	previous_frame_time = SDL_GetTicks();

	//initialize the array of triangles to render
	triangles_to_render = NULL;

	mesh.rotation.x +=0.01;
	mesh.rotation.y +=0.00;
	mesh.rotation.z +=0.00;

	// Loop all triangle faces
	int num_faces = array_length(mesh.faces);
	for (int i = 0; i<num_faces; i++){
		face_t mesh_face = mesh.faces[i];
		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		triangle_t projected_triangle;

		vec3_t transformed_vertices[3];

		// loop all thre vertices of this current face and apply transformations
		for (int j =0; j<3; j++){
			vec3_t transformed_vertex = face_vertices[j];

			transformed_vertex = vec3_rotate_x(transformed_vertex,mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex,mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex,mesh.rotation.z);

			// translate the vertex away from the camera
			transformed_vertex.z += 5;

			// save transformed vertex in the array of transformed vertices
			transformed_vertices[j]= transformed_vertex;
		}

		// check for backface culling
		vec3_t vector_a = transformed_vertices[0];
		vec3_t vector_b = transformed_vertices[1];
		vec3_t vector_c = transformed_vertices[2];

		vec3_t vector_ab = vec3_sub(vector_b, vector_a);
		vec3_t vector_ac = vec3_sub(vector_c, vector_a);
 		vec3_normalize(&vector_ab);
 		vec3_normalize(&vector_ac);
		// compute the face normal (using cross product to find perpendicular)
		vec3_t normal = vec3_cross(vector_ab,vector_ac);

		//normalize the face normal vector
		vec3_normalize(&normal);


		// FInd the vector betweeen a point in the triangle and the camera origin
		vec3_t camera_ray = vec3_sub(camera_position,vector_a);

		// Calculate how aligned the camera ray is with the face normal (using dot product)
		float dot_normal_camera = vec3_dot(normal,camera_ray);

		// Bypass the triangles that are looking away from the camera
		if (dot_normal_camera < 0){
			continue;
		}

		//loop all three vertices perfomr projection
		for (int j=0; j<3; j++){

			// project the current vertex
			vec2_t projected_point = project(transformed_vertices[j]);

			// scale and translate the projected points to the middle of the screen
			projected_point.x += (window_width/2);
			projected_point.y += (window_height/2);

			projected_triangle.points[j] = projected_point;
		}

		//save the projected triangle in the array of triangles to render
		// triangles_to_render[i] = projected_triangle;
		array_push(triangles_to_render,projected_triangle);
	}

	// for (int i=0; i<N_POINTS; i++){
	// 	vec3_t point = cube_points[i];
	// 	vec3_t transformed_point = vec3_rotate_x(point,cube_rotation.x);
	// 	transformed_point = vec3_rotate_y(transformed_point,cube_rotation.y);
	// 	transformed_point = vec3_rotate_z(transformed_point,cube_rotation.z);
	// 	//translate the point away from the camera 
	// 	transformed_point.z -= camera_position.z;

	// 	//project the current point
	// 	vec2_t projected_point = project(transformed_point);
	// 	//save the projected 2d vector in the array of projected points
	// 	projected_points[i] = projected_point;
	// }


}

void render(void){
	// SDL_SetRenderDrawColor(renderer,255,0,0,255);
	// SDL_RenderClear(renderer);

	draw_grid();

	// Loop all projected traingles and render them
	int num_triangles = array_length(triangles_to_render);
	for (int i=0; i<num_triangles ; i++){
		triangle_t triangle = triangles_to_render[i];

		//draw vertex points
		draw_rect(triangle.points[0].x, triangle.points[0].y, 3,3,0xFFFFFF00);
		draw_rect(triangle.points[1].x, triangle.points[1].y, 3,3,0xFFFFFF00);
		draw_rect(triangle.points[2].x, triangle.points[2].y, 3,3,0xFFFFFF00);
		

		//draw triangle lines
		draw_triangle(
			triangle.points[0].x, 
			triangle.points[0].y, 
			triangle.points[1].x,
			triangle.points[1].y,
			triangle.points[2].x,
			triangle.points[2].y,
			0xFF00FF00
		);
	}
	// clear the array of triangles to render every frame loop
	array_free(triangles_to_render);


	render_color_buffer();
	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);

}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Free the memory that was dynamically allocated by the program
//////////////////////////////////////////////////////////////////////////////////////////////////
void free_resources(void){
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(void){

	is_running = initialize_window();
	setup();


	while(is_running){
		process_input();
		update();
		render();
	}

	destroy_window();
	free_resources();

	return 0;
}






