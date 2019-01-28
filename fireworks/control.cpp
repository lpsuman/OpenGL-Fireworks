#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>
#include <algorithm>
#include <string>

#include "control.h"
#include "firework.h"
#include "fuse.h"
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
extern bool is_paused;
extern bool is_spawning;
extern bool show_grid;
extern bool show_spline;
extern vector<ParticleSource *> sources;
extern Fuse *fuse;
extern float fuse_time;
extern vector<ParticleSource *> fuse_particle_sources;

extern Firework *firework;
extern vector<ParticleSource*> firework_sources;

void addColorToSources(vec3 color);

vec3 camera_position = vec3(-10.0f, -3.0f, 15.0f);
vec3 camera_direction;
vec3 camera_right;
vec3 camera_up;
vec3 *camera_focus;
bool is_free_look = false;

float horizontal_angle = 2.35f;
float vertical_angle = -0.15f;

float speed = 0.5f;
float mouse_speed = 0.001f;

float xOrigin = -1.0f;
float yOrigin = -1.0f;

void change_birth_rate(const vector<ParticleSource *> &sources, string name, float delta) {
    for (int i = 0; i < sources.size(); i++) {
        ParticleSource* source = sources[i];
        source->birth_rate += delta;
        source->birth_rate = std::max(1.0f, source->birth_rate);
    }
    cout << "Increased particle generation in " << name << ": " << sources[0]->birth_rate << endl;
}

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
            change_birth_rate(fuse_particle_sources, "the fuse", 50.0f);
            break;
        case DECREASE_BIRTH_RATE:
            change_birth_rate(fuse_particle_sources, "firework", -50.0f);
            break;
        case TOGGLE_SPAWNING_KEY:
            if (is_free_look) {
                is_free_look = false;
                cout << "Free look disabled." << endl;
            } else {
                is_free_look = true;
                cout << "Free look enabled." << endl;
            }
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
            change_birth_rate(firework_sources, "firework", 50.0f);
            break;
        case TOGGLE_SPLINE_KEY:
            change_birth_rate(firework_sources, "firework", -50.0f);
            break;
        case ADD_COLOR_KEY:
            fuse->ignite(fuse_time);
            cout << "Fuse ignited!" << endl;
            break;
        case SUB_COLOR_KEY:
            fuse->reset();
            cout << "Fuse reset." << endl;
            break;
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
    if (!is_free_look && camera_focus != NULL) {
        vec3 diff = normalize(*camera_focus - camera_position);
        vertical_angle = asin(diff.y);
        horizontal_angle = atan2(diff.x, diff.z);
        // if (diff.x >= 0.0f) {
        //     horizontal_angle = -asin(diff.x / cos(vertical_angle)) + 3.14f;
        // } else {
        //     horizontal_angle = -asin(-diff.x / cos(vertical_angle)) + 3.14f;
        // }
    }
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
    printf("%c - pause\n", PAUSE_KEY);
    printf("%c - increase fuse particle spawning speed\n", INCREASE_BIRTH_RATE);
    printf("%c - decrease fuse particle spawning speed\n", DECREASE_BIRTH_RATE);
    printf("%c - toggle free look mode\n", TOGGLE_SPAWNING_KEY);
    printf("%c - increase firework particle spawning speed\n", TOGGLE_GRID_KEY);
    printf("%c - decrease firework particle spawning speed\n", TOGGLE_SPLINE_KEY);
    printf("%c - ignite the fuse\n", ADD_COLOR_KEY);
    printf("%c - reset the fuse\n", SUB_COLOR_KEY);
    printf("\n");
}