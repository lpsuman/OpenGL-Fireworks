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
#include <limits>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <time.h>
#include "camera.h"

using namespace std;
using namespace glm;

GLfloat global_ambient[] = {0.0f, 0.5f, 0.0f, 1.0f};
GLfloat pos0[] = {10.0f, 10.0f, 10.0f, 1.0f};
GLfloat diffuse0[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat ambient0[] = {0.0f, 0.0f, 0.0f, 1.0};
GLfloat specular0[] = {1.0f, 1.0f, 1.0f, 1.0};

char window_title[] = "Object B-spline path";
int window_width = 800, window_height = 800;
int window_pos_x = 100, window_pos_y = 100;
int refresh_rate_millis = 16;

int num_t_steps = 100;
int tan_visuals_per_segment = 10;
float tan_visuals_length_factor = 0.1;
float spline_scaling_factor = 10.0;
float object_scaling_factor = 0.5;
float time_per_segment = 1.0;
bool use_dcm = false;

typedef struct {
    int v1, v2, v3;
    vec3 norm;
} polygon;

CCamera camera;
void arrowsCallback(int key, int mouse_x, int mouse_y);
template<typename Out>
void split(const string &s, char delim, Out result);
vector<string> split(const string &s, char delim);
bool readObject(char* file_path);
void scaleObject();
void calcCoeff(vector<vec3>& points);
void calcPointNorms();
bool readBSpline(char* file_path);
void calcSplinePoints();

void myDisplay();
void myReshape(int width, int height);
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);

void initGL();
void timer(int value);

const float MIN_DOUBLE = std::numeric_limits<float>::min();
const float MAX_DOUBLE = std::numeric_limits<float>::max();
float xmin = MAX_DOUBLE, ymin = MAX_DOUBLE, zmin = MAX_DOUBLE;
float xmax = MIN_DOUBLE, ymax = MIN_DOUBLE, zmax = MIN_DOUBLE;
float maxSize;
vec3 size, center;

vector<vec3> points;
vector<polygon> polygons;
vector< vector<int> > polys_near_point;
vector<vec3> pointNorms;

vec3 starting_object_orientation;
int num_segments;
vector<vec3> spline_control_points;
vector<vec3> spline_points;
vector<vec3> spline_tan_points;
vector<vec3> spline_tan2_points;
vector<vec3> spline_tan_visuals_end_points;
float spline_min_pos;
float spline_max_pos;
int current_pos_on_spline;
bool is_paused;

mat4x4 B_3 = transpose(mat4x4(
    -1, 3, -3, 1,
    3, -6, 3, 0,
    -3, 0, 3, 0,
    1, 4, 1, 0
    ));

mat4x3 B_2 = transpose(mat3x4(
    -1, 3, -3, 1,
    2, -4, 2, 0,
    -1, 0, 1, 0
    ));

mat4x2 B_1 = transpose(mat2x4(
    -1, 3, -3, 1,
    1, -2, 1, 0
    ));

void myDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    camera.Render();

    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < spline_points.size(); i++) {
        glVertex3f(spline_points[i].x, spline_points[i].y, spline_points[i].z);
    }
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < num_segments; i++) {
        for (int j = 0; j < tan_visuals_per_segment; j++) {
            int index = i * num_t_steps + j * (num_t_steps / tan_visuals_per_segment);
            glVertex3f(spline_points[index].x, spline_points[index].y, spline_points[index].z);
            glVertex3f(spline_tan_visuals_end_points[index].x,
                spline_tan_visuals_end_points[index].y,
                spline_tan_visuals_end_points[index].z);
        }
    }
    glEnd();

    if (use_dcm) {
        vec3 w = spline_tan_points[current_pos_on_spline];
        vec3 w2 = spline_tan2_points[current_pos_on_spline];
        vec3 u = cross(w, w2);
        vec3 v = cross(w, u);

        mat3x3 dcm = mat3x3();
        dcm[0] = normalize(w);
        dcm[1] = normalize(u);
        dcm[2] = normalize(v);
        dcm = inverse(dcm);

        glTranslatef(spline_points[current_pos_on_spline].x,
            spline_points[current_pos_on_spline].y,
            spline_points[current_pos_on_spline].z);

        glBegin (GL_TRIANGLES);
        for (vector<polygon>::iterator it = polygons.begin(); it < polygons.end(); it++) {
            int indices[3] = {it->v1, it->v2, it->v3};
            for (int i = 0; i < 3; i++) {
                int index = indices[i] - 1;
                vec3 normal = pointNorms[index];
                vec3 dcm_normal = normal * dcm;
                glNormal3f(dcm_normal.x, dcm_normal.y, dcm_normal.z);
                vec3 vertex = points[index];
                vec3 dcm_vertex = vertex * dcm;
                glVertex3f(dcm_vertex.x, dcm_vertex.y, dcm_vertex.z);
            }
        }
        glEnd();
    } else {
        vec3 s = starting_object_orientation;
        vec3 e = spline_tan_points[current_pos_on_spline];
        vec3 rotation_axis = cross(s, e);
        float rotation_angle = acos(dot(s, e) / (length(s) * length(e)));
        rotation_angle = 360 * (rotation_angle / (2 * M_PI));

        glTranslatef(spline_points[current_pos_on_spline].x,
            spline_points[current_pos_on_spline].y,
            spline_points[current_pos_on_spline].z);
        glRotatef(rotation_angle, rotation_axis.x, rotation_axis.y, rotation_axis.z);

        glBegin (GL_TRIANGLES);
        for (vector<polygon>::iterator it = polygons.begin(); it < polygons.end(); it++) {
            int indices[3] = {it->v1, it->v2, it->v3};
            for (int i = 0; i < 3; i++) {
                int index = indices[i] - 1;
                vec3 *vertex = &points[index];
                vec3 *normal = &pointNorms[index];
                glNormal3f(normal->x, normal->y, normal->z);
                glVertex3f(vertex->x, vertex->y, vertex->z);
            }
        }
        glEnd();
    }

    glFlush();
    glutSwapBuffers();
}

void arrowsCallback(int key, int mouse_x, int mouse_y) {
    switch (key) {
        case GLUT_KEY_UP:
            camera.RotateX(5.0f);
            break;
        case GLUT_KEY_DOWN:
            camera.RotateX(-5.0f);
            break;
        case GLUT_KEY_RIGHT:
            camera.RotateY(-5.0f);
            break;
        case GLUT_KEY_LEFT:
            camera.RotateY(5.0f);
            break;
    }
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY) {
    switch (theKey) {
        case 27:    //ESC
            PostQuitMessage(0);
            break;
        case 'w':
            camera.MoveForward(-0.1);
            break;
        case 's':
            camera.MoveForward(0.1);
            break;
        case 'a':
            camera.StrafeRight(-0.1);
            break;
        case 'd':
            camera.StrafeRight(0.1);
            break;
        case 'f':
            camera.MoveUpward(-0.3);
            break;
        case 32:    //spacebar
            camera.MoveUpward(0.3);
            break;
        case 'm':
            if (use_dcm) {
                use_dcm = false;
                cout << "Not using DCM." << endl;
            } else {
                use_dcm = true;
                cout << "Using DCM." << endl;
            }
            break;
        case 'n':
            current_pos_on_spline = 0;
            break;
        case 'b':
            if (is_paused) {
                is_paused = false;
                cout << "Resuming." << endl;
            } else {
                is_paused = true;
                cout << "Pausing." << endl;
            }
            break;
        case 'j':
            if (is_paused) {
                current_pos_on_spline--;
            }
            break;
        case 'k':
            if (is_paused) {
                current_pos_on_spline++;
            }
            break;
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "Two arguments are required: 1) path to an .obj file 2) path to a .txt file which contains a B-spline." << endl;
        return -1;
    }

    if (!readObject(argv[1])) {
        cout << "Error reading object file!" << endl;
        return -1;
    }
    scaleObject();
    calcCoeff(points);
    calcPointNorms();

    if (!readBSpline(argv[2])) {
        cout << "Error reading B-spline file!" << endl;
        return -1;
    }
    calcSplinePoints();

    starting_object_orientation = vec3();
    if (argc > 3) {
        if (argc != 6) {
            cout << "Three optional arguments are the starting object orientation." << endl;
            return -1;
        }
        starting_object_orientation.x = strtod(argv[3], NULL);
        starting_object_orientation.y = strtod(argv[4], NULL);
        starting_object_orientation.z = strtod(argv[5], NULL);
    } else {
        starting_object_orientation.x = 1.0f;
    }

    glutInit(&argc, argv);
    initGL();
    glutMainLoop();

    return 0;
}

bool readObject(char* file_path) {
    ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> lineParts = split(line, ' ');
        if (lineParts.size() != 4) continue;
        if (lineParts[0] == "#") {
            // comment in .obj file
        } else if (lineParts[0] == "v") {
            vec3 v = vec3();
            v.x = strtod(lineParts[1].c_str(), NULL);
            v.y = strtod(lineParts[2].c_str(), NULL);
            v.z = strtod(lineParts[3].c_str(), NULL);
            points.push_back(v);

            if (v.x < xmin) xmin = v.x;
            if (v.x > xmax) xmax = v.x;
            if (v.y < ymin) ymin = v.y;
            if (v.y > ymax) ymax = v.y;
            if (v.z < zmin) zmin = v.z;
            if (v.z > zmax) zmax = v.z;

            vector<int> poly;
            polys_near_point.push_back(poly);
        } else if (lineParts[0] == "f") {
            polygon poly;
            poly.v1 = atoi(lineParts[1].c_str());
            poly.v2 = atoi(lineParts[2].c_str());
            poly.v3 = atoi(lineParts[3].c_str());
            polygons.push_back(poly);

            polys_near_point[poly.v1 - 1].push_back(polygons.size() - 1);
            polys_near_point[poly.v2 - 1].push_back(polygons.size() - 1);
            polys_near_point[poly.v3 - 1].push_back(polygons.size() - 1);
        } else {
            // invalid line, do nothing
        }
    }
    size.x = xmax - xmin;
    size.y = ymax - ymin;
    size.z = zmax - zmin;
    maxSize = std::max(std::max(size.x, size.y), size.z);
    return true;
}

void scaleObject() {
    center = vec3((xmax + xmin) / 2.0f, (ymax + ymin) / 2.0f, (zmax + zmin) / 2.0f);
    for (int i = 0; i < points.size(); i++) {
        points[i] -= center;
        points[i] *= object_scaling_factor / maxSize;
    }
}

void calcCoeff(vector<vec3>& points) {
    for (int i = 0, n = polygons.size(); i < n; i++) {
        vec3 v1 = points[polygons[i].v1 - 1];
        vec3 v2 = points[polygons[i].v2 - 1];
        vec3 v3 = points[polygons[i].v3 - 1];

        polygons[i].norm = vec3();
        polygons[i].norm.x = (v2.y - v1.y) * (v3.z - v1.z) - (v2.z - v1.z) * (v3.y - v1.y);
        polygons[i].norm.y = -(v2.x - v1.x) * (v3.z - v1.z) + (v2.z - v1.z) * (v3.x - v1.x);
        polygons[i].norm.z = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);
    }
}

void calcPointNorms() {
    for (int i = 0; i < polys_near_point.size(); i++) {
        vector<int> polys = polys_near_point[i];
        vec3 pointNorm = vec3();
        for (int j = 0; j < polys.size(); j++) {
            pointNorm += normalize(polygons[polys[j]].norm);
        }

        pointNorm /= (float)polys.size();
        vec3 temp = normalize(vec3(pointNorm));
        pointNorm.x = temp.x;
        pointNorm.y = temp.y;
        pointNorm.z = temp.z;

        pointNorms.push_back(pointNorm);
    }
}

bool readBSpline(char* file_path) {
    ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> lineParts = split(line, ' ');
        if (lineParts.size() != 3) continue;
        
        vec3 v = vec3();
        v.x = strtod(lineParts[0].c_str(), NULL);
        v.y = strtod(lineParts[1].c_str(), NULL);
        v.z = strtod(lineParts[2].c_str(), NULL);
        spline_min_pos = std::min(spline_min_pos, std::min(v.x, std::min(v.y, v.z)));
        spline_max_pos = std::max(spline_max_pos, std::max(v.x, std::max(v.y, v.z)));
        spline_control_points.push_back(v);
    }
    num_segments = spline_control_points.size() - 3;

    if (num_segments < 1) {
        cout << "At least 4 points are required for a B-spline." << endl;
        return false;
    }

    for (int i = 0; i < spline_control_points.size(); i++) {
        spline_control_points[i].x -= (spline_min_pos + spline_max_pos) / 2.0;
        spline_control_points[i].y -= (spline_min_pos + spline_max_pos) / 2.0;
        spline_control_points[i].z -= (spline_min_pos + spline_max_pos) / 2.0;

        float spline_max_abs_pos = std::max(abs(spline_min_pos), abs(spline_max_pos));

        spline_control_points[i].x *= spline_scaling_factor / spline_max_abs_pos;
        spline_control_points[i].y *= spline_scaling_factor / spline_max_abs_pos;
        spline_control_points[i].z *= spline_scaling_factor / spline_max_abs_pos;
    }

    return true;
}

void calcSplinePoints() {
    for (int i = 0; i < num_segments; i++) {
        mat4x3 R_T = mat4x3();
        for (int j = 0; j < 4; j++) {
            R_T[j] = spline_control_points[i + j];
        }
        mat3x4 R = transpose(R_T);

        for (int j = 0; j < num_t_steps; j++) {
            float t = j / float(num_t_steps);

            vec4 T_3 = vec4(pow(t, 3), t * t, t, 1);
            vec4 temp = T_3 * B_3;
            vec3 spline_point = temp * R * (1 / 6.0f);
            spline_points.push_back(spline_point);

            vec3 T_2 = vec3(t * t, t, 1);
            temp = T_2 * B_2;
            vec3 tan_point = temp * R * 0.5f;
            spline_tan_points.push_back(tan_point);

            vec2 T_1 = vec2(t, 1);
            temp = T_1 * B_1;
            vec3 tan2_point = temp * R;
            spline_tan2_points.push_back(tan2_point);

            vec3 tan_end_point = spline_point + tan_visuals_length_factor * tan_point;
            spline_tan_visuals_end_points.push_back(tan_end_point);
        }
    }
}

void initGL() {
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(window_pos_x, window_pos_y);
    glutCreateWindow(window_title);

	camera.Move(F3dVector(0.0, 0.0, 3.0));
	camera.MoveForward(1.0);
    glutSpecialFunc(arrowsCallback);

    glutDisplayFunc(myDisplay);
    glutReshapeFunc(myReshape);
    glutKeyboardFunc(myKeyboard);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    glutTimerFunc(0, timer, 0);
}

void timer(int value) {
    glutPostRedisplay();
    if (!is_paused && ++current_pos_on_spline >= spline_points.size()) {
        current_pos_on_spline = 0;
    }
    glutTimerFunc(refresh_rate_millis, timer, 0);
}
 
void myReshape(GLsizei width, GLsizei height) {
    if (height == 0) height = 1;
    GLfloat aspect = (GLfloat)width / (GLfloat)height;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.5f, 20.0f);

    glViewport(0, 0, width, height);
}

template<typename Out>
void split(const string &s, char delim, Out result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        if (!item.empty()) {
            *(result++) = item;
        }
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}