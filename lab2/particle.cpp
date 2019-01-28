#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <algorithm>
#include <math.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <time.h>
// #include <SOIL/src/SOIL.h>
#include "particle.h"
#include "spline.h"
#include "control.h"

using namespace std;
using namespace glm;

extern vec3 camera_position;

vec3 line_color = vec3(0.25f, 0.25f, 0.25f);
vec4 background_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec3 direction_color = vec3(0.0f, 1.0f, 0.0f);

int temp = 200;

int DEFAULT_V = 1.0f;
vec3 DIRECTION_BIAS = vec3(0.0f, 1.0f, 0.0f);
float DEFAULT_DECAY_SPEED = 0.3f;
float DEFAULT_POINT_SIZE = 0.1f;
vec3 DEFAULT_PARTICLE_COLOR = vec3(0.5f, 0.0f, 0.5f);
int DEFAULT_BIRTH_RATE = 30;

char window_title[] = "Particle system on B-spline";
int window_width = 800, window_height = 800;
int window_pos_x = 100, window_pos_y = 100;
int refresh_rate_millis = 16;

Particle::Particle(vec3 pos, float v, vec3 direction, float decay_speed, float size, vec3 color):
        pos(pos), v(v), direction(direction), decay_speed(decay_speed), size(size), color(color) {
    t = 0.0f;
}

ParticleSource::ParticleSource() {
    point_v = DEFAULT_V;
    direction_bias = DIRECTION_BIAS;
    point_decay_speed = DEFAULT_DECAY_SPEED;
    point_size = DEFAULT_POINT_SIZE;
    point_color = DEFAULT_PARTICLE_COLOR;
    birth_rate = DEFAULT_BIRTH_RATE;
    frames_until_birth = 0;
    point_on_spline = 0;
}

Spline* spline;
GLuint texture;
vector<Particle *> particles;
vector<ParticleSource *> sources;

vec3 starting_object_orientation;
bool is_paused;
bool is_spawning = true;
bool show_grid = true;
bool show_spline = true;

GLuint LoadTextureRAW( const char * filename, int wrap );
void myDisplay();
void myReshape(int width, int height);
void initGL(char** argv);
void timer(int value);

float rand_float(float lower, float upper) {
    return lower + (upper - lower) * (rand() / RAND_MAX);
}

void timer(int value) {
    float delta_time = refresh_rate_millis / 1000.0f;

    for (int i = 0; i < sources.size(); i++) {
        ParticleSource* source = sources[i];

        if (!is_paused) {
            source->pos = spline->points[source->point_on_spline];
            source->point_on_spline += 1;
            if (source->point_on_spline >= spline->points.size()) {
                source->point_on_spline = 0;
            }
        }

        if (is_spawning && source->frames_until_birth <= 0) {
            source->frames_until_birth = (int )(60 / (rand() % source->birth_rate + 1));
            
            float x, y, z, norm;
            x = (rand() % 2000 - 1000) / 1000.0f;
            y = (rand() % 2000 - 1000) / 1000.0f;
            z = (rand() % 2000 - 1000) / 1000.0f;
            norm = pow(pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f), 0.5f);
            vec3 rand_direction = vec3(x / norm, y / norm, z / norm);

            Particle* particle = new Particle(
                source->pos,
                source->point_v * rand_float(0.75, 1.25),
                rand_direction + source->direction_bias,
                source->point_decay_speed * rand_float(0.75, 1.25),
                source->point_size * rand_float(0.75, 1.25),
                source->point_color
            );

            particles.push_back(particle);
        } else {
            source->frames_until_birth -= 1;
        }
    }

    for (int j = particles.size() - 1; j >= 0; j--) {
        Particle* particle = particles[j];
        vec3 s = vec3(0.0f, 0.0f, 1.0f);
        vec3 e = particle->pos - camera_position;
        particle->rot_axis = cross(s, e);
        float rotation_angle = acos(dot(s, e) / (length(s) * length(e)));
        particle->rot_angle = 360 * (rotation_angle / (2 * M_PI));

        particle->pos += particle->v * particle->direction * delta_time;
        particle->t += particle->decay_speed * delta_time;

        if (particle->t >= 1.0f) {
            particles.erase(particles.begin() + j);
        }
    }
    
    glutPostRedisplay();
    glutTimerFunc(refresh_rate_millis, timer, 0);
}

void drawParticles() {
    for (int i = 0; i < particles.size(); i++) {
        Particle* p = particles[i];
        glColor3f(p->color.x, p->color.y, p->color.z);
        glTranslatef(p->pos.x, p->pos.y, p->pos.z);
        glRotatef(p->rot_angle, p->rot_axis.x, p->rot_axis.y, p->rot_axis.z);
        glBegin(GL_QUADS);
        
        float size = p->size * (1 - p->t);

        glTexCoord2d(0.0, 0.0); glVertex3f(-size, -size, 0.0);
        glTexCoord2d(1.0, 0.0); glVertex3f(-size, +size, 0.0);
        glTexCoord2d(1.0, 1.0); glVertex3f(+size, +size, 0.0);
        glTexCoord2d(0.0, 1.0); glVertex3f(+size, -size, 0.0);

        glEnd();
        glRotatef(-p->rot_angle, p->rot_axis.x, p->rot_axis.y, p->rot_axis.z);
        glTranslatef(-p->pos.x, -p->pos.y, -p->pos.z);
    }
}

void myDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    updateCamera();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    if (show_grid) {
        draw_horizon_grid();
    }

    if (show_spline) {
        glLineWidth(3.0f);
        glBegin(GL_LINE_STRIP);
        glColor3f(line_color.x, line_color.y, line_color.z);
        for (int i = 0; i < spline->points.size(); i++) {
            glVertex3f(spline->points[i].x, spline->points[i].y, spline->points[i].z);
        }
        glEnd();
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    drawParticles();

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "Two arguments are required:\n1) path to a texture file\n2) path to a .txt file which contains a B-spline." << endl;
        return -1;
    }

    spline = new Spline(argv[2]);

    starting_object_orientation = vec3();
    if (argc > 3) {
        if (argc != 6) {
            cout << "Three optional arguments are the starting orientation." << endl;
            return -1;
        }
        starting_object_orientation.x = strtod(argv[3], NULL);
        starting_object_orientation.y = strtod(argv[4], NULL);
        starting_object_orientation.z = strtod(argv[5], NULL);
    } else {
        starting_object_orientation.x = 1.0f;
    }

    srand (time(NULL));
    ParticleSource* source = new ParticleSource();
    sources.push_back(source);

    print_controls_info();
    
    glutInit(&argc, argv);
    initGL(argv);
    glutMainLoop();

    return 0;
}

void initGL(char** argv) {
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(window_pos_x, window_pos_y);
    glutCreateWindow(window_title);

    glutDisplayFunc(myDisplay);
    glutReshapeFunc(myReshape);
    glutKeyboardFunc(myKeyboard);
    glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);

    texture = LoadTextureRAW(argv[1], 0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glClearColor(background_color.x, background_color.y, background_color.z, background_color.w);
    glPointSize(1.0);
    // glClearDepth(1.0f);
    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LEQUAL);
    // glShadeModel(GL_SMOOTH);
    // glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glutTimerFunc(0, timer, 0);
}
 
void myReshape(GLsizei width, GLsizei height) {
    if (height == 0) height = 1;
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 30.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
}

// load a 256x256 RGB .RAW file as a texture
GLuint LoadTextureRAW(const char * filename, int wrap) {
    GLuint texture;
    int width, height;
    BYTE * data;
    FILE * file;
    file = fopen(filename, "rb");
    if (file == NULL)  {
		return 0;
	}

    width = 256;
    height = 256;
    data =(BYTE *) malloc(width * height * 3);
    fread(data, width * height * 3, 1, file);
    fclose(file);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height,
                       GL_RGB, GL_UNSIGNED_BYTE, data);

    free(data);

    return texture;
}

