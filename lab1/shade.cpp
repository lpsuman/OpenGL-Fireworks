#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <math.h>
#include <vector>
#include <limits>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <time.h>

using namespace std;
using namespace glm;

GLuint window;
GLuint width = 800, height = 800;

void myDisplay();
void myReshape(int width, int height);
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);

template<typename Out>
void split(const string &s, char delim, Out result);
vector<string> split(const string &s, char delim);
bool readObject(char* file_path);
void scaleObject();
void calcCoeff(vector<vec4>& points);
bool isPointInside();
void printInput();
void calcViewMatrix();
vec4 calcCenter();
void calcPositionVectors();
void addVec4AndVec3(vec4* v4, vec3* v3);
void transformVertices(vector<vec4>& input, vector<vec4>& output);
void calcPointNorms();
void calcAmbient();
void calcDiffuse();
void calcPointDiffuse();

typedef struct {
    int v1, v2, v3;
    vec4 norm;
    float diffuse;
} polygon;

bool isHiddenPolygon(polygon* ppoly);

const float MIN_DOUBLE = std::numeric_limits<float>::min();
const float MAX_DOUBLE = std::numeric_limits<float>::max();
float xmin = MAX_DOUBLE, ymin = MAX_DOUBLE, zmin = MAX_DOUBLE;
float xmax = MIN_DOUBLE, ymax = MIN_DOUBLE, zmax = MIN_DOUBLE;
vec4 size, center;
float maxSize;

vector<vec4> points, currPoints;
vector< vector<int> > polyForPoint;
vector<float> pointDiffuse;
vector<vec4> pointNorms;
vector<polygon> polygons;
vec4 pointO, pointG;
mat4x4 matT, matP;

const int AMBIENT_INTENSITY = 100;
const float AMBIENT_COEFFICIENT = 0.5;
const float DIFFUSE_COEFFICIENT = (1.0f - (AMBIENT_INTENSITY * AMBIENT_COEFFICIENT) / 255.0f);

vec4 lightPos;
ivec3 lightColor;
bool showColor;
bool showGouraud;
int ambient;

vec3 zo, xo, yo, zs;
float speedFactor = 0.30;
float speed;

float scaleFactor;

vector<vec4> controlPoints, currControlPoints;
vector<vec4> bezierPoints, currBezierPoints;
int numOfPoints = 100;

bool isSimulationRunning = false;
int currentSimulationPoint = 0;
const long waitTime = 4L * (1000000000L / (long)numOfPoints);
vec4 defaultO;

int main(int argc, char ** argv) {
    if (argc < 2) {
        cout << "one argument is required: path to an .obj file" << endl;
        return -1;
    }

    if (!readObject(argv[1])) {
        cout << "Error reading object file" << endl;
        return -1;
    }

    scaleObject();
    calcCoeff(points);
    calcPointNorms();

    if (argc > 2) {
        if (argc != 8) {
            return -1;
        }
        lightPos = vec4(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), 1.0f);
        lightColor = vec3(atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
        calcAmbient();
        calcDiffuse();
        calcPointDiffuse();
    }

    zs = vec3(0, 0, 1);

    float a = 8.0f;
    pointO = vec4(a, a * 0.8, -(a / 2.0f), 1.0f);
    pointG = vec4() + (pointO / 0.75f);
    pointG.w = 1.0f;

    defaultO = vec4(pointO);

    glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(20, 20);
	glutInit(&argc, argv);

	window = glutCreateWindow("draw object in 3D");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
    glutKeyboardFunc(myKeyboard);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glutMainLoop();
    return 0;
}



void calcCoeff(vector<vec4>& points) {
    for (int i = 0, n = polygons.size(); i < n; i++) {
        vec4 v1 = points[polygons[i].v1 - 1];
        vec4 v2 = points[polygons[i].v2 - 1];
        vec4 v3 = points[polygons[i].v3 - 1];

        // polygon *p = (polygon *)NULL;
        // int a = p->v1;

        polygons[i].norm = vec4();
        polygons[i].norm.x = (v2.y - v1.y) * (v3.z - v1.z) - (v2.z - v1.z) * (v3.y - v1.y);
        polygons[i].norm.y = -(v2.x - v1.x) * (v3.z - v1.z) + (v2.z - v1.z) * (v3.x - v1.x);
        polygons[i].norm.z = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);
        polygons[i].norm.w = -(v1.x * polygons[i].norm.x) - (v1.y * polygons[i].norm.y) - (v1.z * polygons[i].norm.z);
    }
}

void calcPointNorms() {
    for (int i = 0; i < polygons.size(); i++) {
        // cout << to_string(polygons[i].norm) << endl;
    }
    // cout << endl;
    for (int i = 0; i < polyForPoint.size(); i++) {
        vector<int> polys = polyForPoint[i];
        vec4 pointNorm = vec4();
        for (int j = 0; j < polys.size(); j++) {
            pointNorm += normalize(polygons[polys[j]].norm);
        }

        pointNorm /= (float)polys.size();
        vec3 temp = normalize(vec3(pointNorm));
        pointNorm.x = temp.x;
        pointNorm.y = temp.y;
        pointNorm.z = temp.z;
        pointNorm.w = 1.0f;

        // cout << to_string(pointNorm) << endl;

        pointNorms.push_back(pointNorm);
        pointDiffuse.push_back(0.0f);
    }
    vec3 temp = vec3();
    for (int i = 0; i < pointNorms.size(); i++) {
        temp.x += pointNorms[i].x;
        temp.y += pointNorms[i].y;
        temp.z += pointNorms[i].z;
    }
    // cout << "sum " << to_string(temp) << endl;
}

void myDisplay() {
    
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    calcViewMatrix();
    transformVertices(points, currPoints);

    if (showColor) {
        glBegin (GL_TRIANGLES);
        vec4 *v1, *v2, *v3;
        vector<polygon>::iterator it;
        for(it = polygons.begin(); it < polygons.end(); it++) {
            if (isHiddenPolygon(&(*it))) {
                continue;
            }
            v1 = &currPoints[it->v1 - 1];
            v2 = &currPoints[it->v2 - 1];
            v3 = &currPoints[it->v3 - 1];

            if (!showGouraud) {
                vec3 resultColor = vec3(lightColor);
                resultColor *= DIFFUSE_COEFFICIENT * it->diffuse;
                resultColor += ambient;

                glColor3ub((int)resultColor.r, (int)resultColor.g, (int)resultColor.b);
                glVertex3f(v1->x, v1->y, v1->z);
                glVertex3f(v2->x, v2->y, v2->z);
                glVertex3f(v3->x, v3->y, v3->z);
            } else {
                vec3 colorV1 = vec3(lightColor);
                colorV1 *= DIFFUSE_COEFFICIENT * pointDiffuse[it->v1 - 1];
                colorV1 += ambient;
                glColor3ub((int)colorV1.r, (int)colorV1.g, (int)colorV1.b);
                glVertex3f(v1->x, v1->y, v1->z);

                vec3 colorV2 = vec3(lightColor);
                colorV2 *= DIFFUSE_COEFFICIENT * pointDiffuse[it->v2 - 1];
                colorV2 += ambient;
                glColor3ub((int)colorV2.r, (int)colorV2.g, (int)colorV2.b);
                glVertex3f(v2->x, v2->y, v2->z);

                vec3 colorV3 = vec3(lightColor);
                colorV3 *= DIFFUSE_COEFFICIENT * pointDiffuse[it->v3 - 1];
                colorV3 += ambient;
                glColor3ub((int)colorV3.r, (int)colorV3.g, (int)colorV3.b);
                glVertex3f(v3->x, v3->y, v3->z);
            }
        }
	    glEnd();
    } else {
        glBegin(GL_LINES);
        glColor3f(0.0f, 0.0f, 0.0f);
        vec4 *v1, *v2, *v3;
        vector<polygon>::iterator it;
        for(it = polygons.begin(); it < polygons.end(); it++) {

            if (isHiddenPolygon(&(*it))) {
                continue;
            }
            
            v1 = &currPoints[(*it).v1 - 1];
            v2 = &currPoints[(*it).v2 - 1];
            v3 = &currPoints[(*it).v3 - 1];
            glVertex3f(v1->x, v1->y, v1->z);
            glVertex3f(v2->x, v2->y, v2->z);
            
            glVertex3f(v2->x, v2->y, v2->z);
            glVertex3f(v3->x, v3->y, v3->z);
            
            glVertex3f(v3->x, v3->y, v3->z);
            glVertex3f(v1->x, v1->y, v1->z);
        }
        glEnd();
    }

	glutSwapBuffers();
}

void calcAmbient() {
    ambient = AMBIENT_INTENSITY * AMBIENT_COEFFICIENT;
}

void calcDiffuse() {
    vec3 L = normalize(lightPos);
    for (int i = 0; i < polygons.size(); i++) {
        vec3 N = normalize(vec3(polygons[i].norm));
        float dotLN = dot(L, N);
        if (dotLN <= 0.0f) {
            polygons[i].diffuse = 0.0f;
        } else {
            polygons[i].diffuse = dotLN;
        }
    }
}

void calcPointDiffuse() {
    vec3 L = normalize(lightPos);
    for (int i = 0; i < pointNorms.size(); i++) {
        vec3 N = vec3(pointNorms[i]);
        float dotLN = dot(L, N);
        if (dotLN <= 0.0f) {
            pointDiffuse[i] = 0.0f;
        } else {
            pointDiffuse[i] = dotLN;
        }
    }
}

bool isHiddenPolygon(polygon* ppoly) {
    vec3 towardsEye = normalize(vec3(pointO) - vec3(points[ppoly->v1]));
    vec3 polyNorm = normalize(vec3(ppoly->norm));
    if (dot(towardsEye, polyNorm) >= 0) {
        return false;
    } else {
        return true;
    }
}

void calcViewMatrix() {
    mat4x4 matT1, matT2, matT3, matT4, matT5;
    matT1 = transpose(mat4x4(
        1,          0,          0,      0,
        0,          1,          0,      0,
        0,          0,          1,      0,
        -pointO.x, -pointO.y, -pointO.z, 1
    ));
    vec4 pointG1 = pointG * matT1;

    double denominator = sqrt(pointG1.x * pointG1.x + pointG1.y * pointG1.y);
    double sinAlfa = pointG1.y / denominator;
    double cosAlfa = pointG1.x / denominator;
    matT2 = transpose(mat4x4(
        cosAlfa, -sinAlfa,  0,      0,
        sinAlfa, cosAlfa,   0,      0,
        0,      0,          1,      0,
        0,      0,          0,      1
    ));
    vec4 pointG2 = pointG1 * matT2;

    denominator = sqrt(pointG2.x * pointG2.x + pointG2.z * pointG2.z);
    double sinBeta = pointG2.x / denominator;
    double cosBeta = pointG2.z / denominator;
    matT3 = transpose(mat4x4(
        cosBeta, 0, sinBeta,    0,
        0,      1,      0,      0,
        -sinBeta, 0, cosBeta,   0,
        0,      0,      0,      1
    ));
    vec4 pointG3 = pointG2 * matT3;

    matT4 = transpose(mat4x4(
        0, -1, 0, 0,
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    ));

    matT5 = transpose(mat4x4(
        -1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    ));

    matT = matT1 * matT2 * matT3 * matT4 * matT5;

    float H = sqrt(pow((pointO.x - pointG.x), 2) + pow((pointO.y - pointG.y), 2) + pow((pointO.z - pointG.z), 2));

    matP = transpose(mat4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, (1 / H),
        0, 0, 0, 0 
    ));
}

void transformVertices(vector<vec4>& input, vector<vec4>& output) {
    output.clear();
    vec4 result;
    for (int i = 0, n = input.size(); i < n; i++) {
        result = input[i] * matT * matP;
        float hp = result.w;
        result /= hp;
        output.push_back(result);
    }
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
            vec4 v = vec4();
            v.x = strtod(lineParts[1].c_str(), NULL);
            v.y = strtod(lineParts[2].c_str(), NULL);
            v.z = strtod(lineParts[3].c_str(), NULL);
            v.w = 1.0;
            points.push_back(v);

            if (v.x < xmin) xmin = v.x;
            if (v.x > xmax) xmax = v.x;
            if (v.y < ymin) ymin = v.y;
            if (v.y > ymax) ymax = v.y;
            if (v.z < zmin) zmin = v.z;
            if (v.z > zmax) zmax = v.z;

            vector<int> poly;
            polyForPoint.push_back(poly);
        } else if (lineParts[0] == "f") {
            polygon poly;
            poly.v1 = atoi(lineParts[1].c_str());
            poly.v2 = atoi(lineParts[2].c_str());
            poly.v3 = atoi(lineParts[3].c_str());
            polygons.push_back(poly);

            polyForPoint[poly.v1 - 1].push_back(polygons.size() - 1);
            polyForPoint[poly.v2 - 1].push_back(polygons.size() - 1);
            polyForPoint[poly.v3 - 1].push_back(polygons.size() - 1);
        } else {
            // invalid line
        }
    }
    size.x = xmax - xmin;
    size.y = ymax - ymin;
    size.z = zmax - zmin;
    maxSize = std::max(std::max(size.x, size.y), size.z);
    speed = maxSize * speedFactor;
    return true;
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY) {
    if (theKey == 'l') {
        lightPos = vec4();
        cout << "unesite poziciju izvora (x, y, z):" << endl;
        cin >> lightPos.x >> lightPos.y >> lightPos.z;
        lightPos.w = 1.0f;
        cout << "unesite boju izvora (r, g, b):" << endl;
        cin >> lightColor.r >> lightColor.g >> lightColor.b;

        calcAmbient();
        calcDiffuse();
        calcPointDiffuse();

        showColor = true;
        return;
    }
    if (theKey == 'c') {
        showColor = !showColor;
    }
    if (theKey == 'g') {
        showGouraud = !showGouraud;
    }

    calcPositionVectors();
    vec3 temp = vec3(0, 0, 0);
    switch (theKey) {
        case 'w':
            temp = speed * zo;
            break;
        case 'a':
            temp = -speed * xo;
            break;
        case 's':
            temp = -speed * zo;
            break;
        case 'd':
            temp = speed * xo;
            break;
        case 'e':
            temp = speed * yo;
            break;
        case 'q':
            temp = -speed * yo;
            break;
        case '+':
            speedFactor += 0.01;
            speed = speedFactor * maxSize;
            break;
        case '-':
            speedFactor -= 0.01;
            speed = speedFactor * maxSize;
            break;
            
        case 'r':
            if (isSimulationRunning) {
                isSimulationRunning = false;
            } else {
                if (currentSimulationPoint == 0) {
                    isSimulationRunning = true;
                } else {
                    currentSimulationPoint = 0;
                }
            }
            break;
    }
    addVec4AndVec3(&pointO, &temp);
    pointG = vec4() + (pointO / 0.75f);
    pointG.w = 2.0f;
    glutPostRedisplay();
}

void calcPositionVectors() {
    zo = normalize(pointG - pointO);
    xo = -normalize(cross(zo, zs));
    yo = normalize(cross(xo, zo));
}

unsigned int factorial(unsigned int n) {
    unsigned int ret = 1;
    for (unsigned int i = 1; i <= n; ++i)
        ret *= i;
    return ret;
}

void addVec4AndVec3(vec4* v4, vec3* v3) {
    v4->x += v3->x;
    v4->y += v3->y;
    v4->z += v3->z;
}

void myReshape(int w, int h) {
	width = w; height = h;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
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