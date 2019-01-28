#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#include <string>
#include <fstream>
#include <vector>
#include <iostream>

using namespace std;

class Izvor {
public:
	float x, y, z;
	int q;
	float cR, cG, cB;
	double size;
};

class Vrh {
public:
	float x, y, z;
	Vrh(float x1, float y1, float z1) {
		x = x1; y = y1; z = z1;
	}
};

class Cestica {
public:
	float x, y, z;
	float r, g, b;
	float v;
	int t;
	float sX, sY, sZ;
	float osX, osY, osZ;
	double kut; double size;
};

vector<Cestica> cestice;
Izvor i;

//*********************************************************************************
//	Pokazivac na glavni prozor i pocetna velicina.
//*********************************************************************************

GLuint window; 
GLuint sub_width = 512, sub_height = 512;

//*********************************************************************************
//	Function Prototypes.
//*********************************************************************************

void myDisplay			();
void myIdle				();
void myReshape			(int width, int height);
void myKeyboard			(unsigned char theKey, int mouseX, int mouseY);

void nacrtajIzvor		();
void nacrtajCestice		();
void nacrtajCesticu		(Cestica c);
GLuint LoadTextureRAW( const char * filename, int wrap );

double maxH = 42.0;
double pi = 3.14159;
int tempQ = -1;
GLuint tex;

Vrh ociste(0.0, 0.0, 75.0);

int main(int argc, char ** argv)
{
  i.x = 0.0; i.y = 0; i.z = 0.0;
  i.q = 10;
  i.cB = 0.0; i.cG = 0.0; i.cR = 1.0;
  i.size = 0.4;
  srand (time(NULL));
  

  // Nakon ucitavanja svih potrebnih podataka
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(sub_width,sub_height);
	glutInitWindowPosition(100,100);
	glutInit(&argc, argv);

	window = glutCreateWindow("2. laboratorijska vjezba iz Racunalne Grafike");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);
	glutIdleFunc(myIdle);
  tex = LoadTextureRAW("textures/cestica.bmp", 0);

  glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Set The Blending Function For Translucency
	glEnable(GL_BLEND);

	
      glEnable( GL_TEXTURE_2D );
      glBindTexture( GL_TEXTURE_2D, tex );

	glutMainLoop();
    	return 0;
}

void myReshape(int width, int height)
{
	sub_width = width;                      	//promjena sirine prozora
    sub_height = height;						//promjena visine prozora

    glViewport(0, 0, sub_width, sub_height);	//  otvor u prozoru
	
	//-------------------------
	glMatrixMode(GL_PROJECTION);                // Select The Projection Matrix
    glLoadIdentity();                           // Reset The Projection Matrix
 
    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,150.0f);
 
    glMatrixMode(GL_MODELVIEW);                 // Select The Modelview Matrix
    glLoadIdentity();                           // Reset The Modelview Matrix
	//-------------------------

	glLoadIdentity();							//	jedinicna matrica
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );		//	boja pozadine
	glClear( GL_COLOR_BUFFER_BIT );				//	brisanje pozadine
	glPointSize(1.0);							//	postavi velicinu tocke
	glColor3f(0.0f, 0.0f, 0.0f);				//	postavi boju
}

int t = 0;

void myDisplay()
{

 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	glTranslatef(ociste.x, ociste.y, -ociste.z);

	nacrtajCestice();

	glutSwapBuffers();
}

int currentTime = 0; int previousTime = 0;

void myIdle() {
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	int timeInterval = currentTime - previousTime;
	if(timeInterval > 100) {
		
		// Stvori nove cestice
		if(i.q > 0) {
			int n = rand() % i.q + 1;
			for (int j = 0; j < n; j++) {
				double x, y, z, norm;
				x = (rand()%2000-1000) / (double)1000;
				y = (rand()%2000-1000) / (double)1000;
				z = (rand()%2000-1000) / (double)1000;
				norm = pow(pow(x, 2.0) + pow(y, 2.0) + pow(z, 2.0), 0.5);
				x /= norm; y /= norm; z /= norm;
				Cestica c;
				c.x = i.x; c.z = i.z; c.y = i.y; 
				c.r = i.cR; c.g = i.cG; c.b = i.cB; c.v = 0.8;
				c.sX = x; c.sY = y; c.sZ = z;
				c.t = 80 + (rand()%21 - 10);
				c.size = i.size;
				cestice.push_back(c);
			}
		}
		printf("%d\n", cestice.size());
		// Pomakni cestice
		for (int j =  cestice.size() - 1; j >= 0; j--) {

			//Rotacija
			Vrh s(0.0, 0.0, 1.0);
			Vrh os(0.0, 0.0, 0.0);
			Vrh e(0.0, 0.0, 0.0);
			
			e.x = cestice.at(j).x - ociste.x;
			e.y = cestice.at(j).y - ociste.y;
			e.z = cestice.at(j).z - ociste.z;

			os.x = s.y*e.z - e.y*s.z;
			os.y = e.x*s.z - s.x*e.z;
			os.z = s.x*e.y - s.y*e.x;

			double apsS = pow(pow((double)s.x, 2.0) + pow((double)s.y, 2.0) + pow((double)s.z, 2.0), 0.5);
			double apsE = pow(pow((double)e.x, 2.0) + pow((double)e.y, 2.0) + pow((double)e.z, 2.0), 0.5);
			double se = s.x*e.x + s.y*e.y + s.z*e.z;
			double kut = acos(se/(apsS*apsE));
			cestice.at(j).kut = kut / (2*pi) * 360;
			cestice.at(j).osX = os.x; cestice.at(j).osY = os.y; cestice.at(j).osZ = os.z;

			// Ostalo
			cestice.at(j).y += cestice.at(j).v * cestice.at(j).sY;
			cestice.at(j).z += cestice.at(j).v * cestice.at(j).sZ;
			cestice.at(j).x += cestice.at(j).v * cestice.at(j).sX;

			cestice.at(j).t--;

			if (cestice.at(j).r > 0) cestice.at(j).r -= 0.008;
			if (cestice.at(j).b < 1.0) cestice.at(j).b += 0.008;
			if (cestice.at(j).t <= 0) {
				cestice.erase(cestice.begin()+j);
			}
		}

		myDisplay();
		previousTime = currentTime;
	}
}

void nacrtajCestice() {
	for (int j = 0; j < cestice.size(); j++) {
		nacrtajCesticu(cestice.at(j));
	}
}

void nacrtajCesticu(Cestica c) {

	glColor3f(c.r, c.g, c.b);
	glTranslatef(c.x, c.y, c.z);
	glRotatef(c.kut, c.osX, c.osY, c.osZ);//
	glBegin(GL_QUADS);
	
	glTexCoord2d(0.0,0.0); glVertex3f(-c.size, -c.size, 0.0);
	glTexCoord2d(1.0,0.0); glVertex3f(-c.size, +c.size, 0.0);
	glTexCoord2d(1.0,1.0); glVertex3f(+c.size, +c.size, 0.0);
	glTexCoord2d(0.0,1.0); glVertex3f(+c.size, -c.size, 0.0);

	glEnd();
	glRotatef(-c.kut, c.osX, c.osY, c.osZ);//
	glTranslatef(-c.x, -c.y, -c.z);
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY) {
	if(theKey == 'a') i.x -= 0.5;
	if(theKey == 'd') i.x += 0.5;
	if(theKey == 'w') i.z -= 0.5;
	if(theKey == 's') i.z += 0.5;
	if(theKey == 'o') i.y -= 0.5;
	if(theKey == 'p') i.y += 0.5;
	
	if(theKey == 'q' && i.q > 0) i.q--;
	if(theKey == 'e' && i.q < 49) i.q++;

	if(theKey == 'm') {
		if(tempQ == -1) {
			tempQ = i.q;
			i.q = 0;
		}
		else {
			i.q = tempQ;
			tempQ = -1;
		}
	}

	if(theKey == 'y' && i.cG > 0.0) i.cG -= 0.02;
	if(theKey == 'x' && i.cG < 1.0) i.cG += 0.02;

	if(theKey == '+' && i.size < 4.0) i.size += 0.01;
	if(theKey == '-' && i.size > 0.02) i.size -= 0.01;

	if(theKey == '8') ociste.y += 0.1;
	if(theKey == '2') ociste.y -= 0.1;
	if(theKey == '4') ociste.x -= 0.1;
	if(theKey == '6') ociste.x += 0.1;
}

// load a 256x256 RGB .RAW file as a texture
GLuint LoadTextureRAW( const char * filename, int wrap )
{
    GLuint texture;
    int width, height;
    BYTE * data;
    FILE * file;
    // open texture data
    file = fopen( filename, "rb" );
    if ( file == NULL )  {
		return 0;
	}

    // allocate buffer
    width = 256;
    height = 256;
    data =(BYTE*) malloc( width * height * 3 );

    // read texture data
    fread( data, width * height * 3, 1, file );
    fclose( file );

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP );

    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
                       GL_RGB, GL_UNSIGNED_BYTE, data );

    // free buffer
    free( data );

    return texture;
}