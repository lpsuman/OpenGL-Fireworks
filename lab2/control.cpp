#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>
#include <algorithm>
#include "control.h"
#include "particle.h"

#define GRID_SIZE 1000
#define GRID_STEPS 1000

#define MOVE_FORWARD_KEY 'w'
#define MOVE_BACKWARD_KEY 's'
#define STRAFE_LEFT_KEY 'a'
#define STRAFE_RIGHT_KEY 'd'
#define MOVE_UP_KEY 32          //spacebar
#define MOVE_DOWN_KEY 'f'
#define PAUSE_KEY 'p'
#define INCREASE_BIRTH_RATE 'j'
#define DECREASE_BIRTH_RATE 'k'
#define TOGGLE_SPAWNING_KEY 'l'
#define TOGGLE_GRID_KEY 'g'
#define TOGGLE_SPLINE_KEY 'h'
#define ADD_COLOR_KEY 'm'
#define SUB_COLOR_KEY 'n'
#define ADD_SOURCE_KEY 'o'

using namespace std;
using namespace glm;

vec3 color_to_add = vec3(0.0f, 0.1f, 0.0f);

extern vec3 line_color;
extern int current_pos_on_spline;
extern bool is_paused;
extern bool is_spawning;
extern bool show_grid;
extern bool show_spline;
extern vector<ParticleSource *> sources;

void change_birth_rate(int delta);
void addColorToSources(vec3 color);

vec3 camera_position = vec3(-4, 2, 2);
vec3 camera_direction = vec3(0, 0, 0);
vec3 camera_right;
vec3 camera_up;

float horizontal_angle = 2.356f;
float vertical_angle = -0.2f;

float speed = 0.5f;
float mouse_speed = 0.001f;

float xOrigin = -1.0f;
float yOrigin = -1.0f;

void myKeyboard(unsigned char theKey, int mouseX, int mouseY) {
    switch (theKey) {
        case 27:    //ESC
            PostQuitMessage(0);
            break;
        case MOVE_FORWARD_KEY:
            camera_position += camera_direction * speed;
            break;
        case MOVE_BACKWARD_KEY:
            camera_position -= camera_direction * speed;
            break;
        case STRAFE_LEFT_KEY:
            camera_position -= camera_right * speed;
            break;
        case STRAFE_RIGHT_KEY:
            camera_position += camera_right * speed;
            break;
        case MOVE_DOWN_KEY:
            camera_position.y -= speed;
            break;
        case MOVE_UP_KEY:
            camera_position.y += speed;
            break;
        case INCREASE_BIRTH_RATE:
            change_birth_rate(5);
            break;
        case DECREASE_BIRTH_RATE:
            change_birth_rate(-5);
            break;
        case TOGGLE_SPAWNING_KEY:
            is_spawning = !is_spawning;
            break;
        case PAUSE_KEY:
            if (is_paused) {
                is_paused = false;
                cout << "Resuming." << endl;
            } else {
                is_paused = true;
                cout << "Pausing." << endl;
            }
            break;
        case TOGGLE_GRID_KEY:
            show_grid = !show_grid;
            break;
        case TOGGLE_SPLINE_KEY:
            show_spline = !show_spline;
            break;
        case ADD_COLOR_KEY:
            addColorToSources(color_to_add);
            break;
        case SUB_COLOR_KEY:
            addColorToSources(-color_to_add);
            break;
        case ADD_SOURCE_KEY:
            sources.push_back(new ParticleSource());
            break;
    }
}

void change_birth_rate(int delta) {
    for (int i = 0; i < sources.size(); i++) {
        ParticleSource* source = sources[i];
        source->birth_rate += delta;
        source->birth_rate = std::max(-1, source->birth_rate);
    }
}

void addColorToSources(vec3 color) {
    for (int i = 0; i < sources.size(); i++) {
        ParticleSource* source = sources[i];
        source->point_color += color;
        source->point_color = clamp(source->point_color, 0.0f, 1.0f);
    }
}

void mouseMove(int x, int y) { 	
    // this will only be true when the left button is down
    if (xOrigin >= 0.0f) {
        horizontal_angle += (xOrigin - x) * mouse_speed;
		vertical_angle += (yOrigin - y) * mouse_speed;
        glutWarpPointer(xOrigin, yOrigin);
	}
}

void updateCamera() {
    camera_direction = vec3(
        cos(vertical_angle) * sin(horizontal_angle),
        sin(vertical_angle),
        cos(vertical_angle) * cos(horizontal_angle)
    );
    camera_right = vec3(
        sin(horizontal_angle - 3.14f/2.0f),
        0,
        cos(horizontal_angle - 3.14f/2.0f)
    );
    camera_up = cross(camera_right, camera_direction);
    
    vec3 view_point = camera_position + camera_direction;
    gluLookAt(camera_position.x, camera_position.y, camera_position.z,
            view_point.x, view_point.y, view_point.z,
            camera_up.x, camera_up.y, camera_up.z);
}

void mouseButton(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_UP) {
			xOrigin = -1.0f;
            yOrigin = -1.0f;
		}
		else  {
			xOrigin = x;
            yOrigin = y;
		}
	}
}

void draw_horizon_grid() {
    float step_dist = GRID_SIZE / GRID_STEPS;
    glColor3f(line_color.x, line_color.y, line_color.z);
    glLineWidth(0.5f);
    glBegin(GL_LINES);
    for (int i = 0; i < GRID_STEPS; i++) {
        glVertex3f(-GRID_SIZE/2.0f, 0.0f, -GRID_SIZE/2.0f + i * step_dist);
        glVertex3f(GRID_SIZE/2.0f, 0.0f, -GRID_SIZE/2.0f + i * step_dist);
        glVertex3f(-GRID_SIZE/2.0f + i * step_dist, 0.0f, -GRID_SIZE/2.0f);
        glVertex3f(-GRID_SIZE/2.0f + i * step_dist, 0.0f, GRID_SIZE/2.0f);
    }
    glEnd();
    glutWireSphere(0.1f, 10, 10);
}

void print_controls_info() {
    printf("\nControls:\n");
    printf("%c - move camera forward\n", MOVE_FORWARD_KEY);
    printf("%c - move camera forward\n", MOVE_BACKWARD_KEY);
    printf("%c - strafe camera left\n", STRAFE_LEFT_KEY);
    printf("%c - strafe camera right\n", STRAFE_RIGHT_KEY);
    printf("space - move camera up\n");
    printf("%c - move camera down\n", MOVE_DOWN_KEY);
    printf("%c - pause movement on the spline\n", PAUSE_KEY);
    printf("%c - increase particle spawning speed\n", INCREASE_BIRTH_RATE);
    printf("%c - decrease particle spawning speed\n", DECREASE_BIRTH_RATE);
    printf("%c - toggle particle spawning\n", TOGGLE_SPAWNING_KEY);
    printf("%c - show/hide grid\n", TOGGLE_GRID_KEY);
    printf("%c - show/hide spline\n", TOGGLE_SPLINE_KEY);
    printf("%c - add color (green) to sources\n", ADD_COLOR_KEY);
    printf("%c - sub color (green) from sources\n", SUB_COLOR_KEY);
    printf("%c - add new source\n", ADD_SOURCE_KEY);
    printf("\n");
}