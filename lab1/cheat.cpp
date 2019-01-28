#ifdef _WIN32
#include <windows.h>             //bit ce ukljuceno ako se koriste windows
#endif

//  #include <GL/Gl.h>
//  #include <GL/Glu.h>    nije potrebno ako se koristi glut
#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>

#include <string>
#include <fstream>
#include <vector>
#include <iostream>

using namespace std;

int brF = 0, brV = 0, brB = 0;
int brS = 0;

class Vrh {
public:
	float x, y, z;
	Vrh(float x1, float y1, float z1) {
		x = x1; y = y1; z = z1;
	}
};
class Poligon {
public:
	Vrh * v1, * v2, * v3;
	Poligon (Vrh * vX, Vrh * vY, Vrh * vZ) {
		v1 = vX; v2 = vY; v3 = vZ;
	}
};


Poligon *poligoni;
Vrh *vrhovi;
Vrh *vrhB;
Vrh *bSpline;
Vrh *tanB, *sveTanB;
Vrh srediste(0.0, 0.0, 0.0);
Vrh os(0.0, 0.0, 0.0);
Vrh s(0.0, 0.0, 1.0);
Vrh e(0.0, 0.0, 0.0);
float pi =  3.14159265;

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

int main(int argc, char ** argv)
{
  vector<string> text_file;
  
//   ifstream ifs("747.obj");
//   ifstream ifs("porsche.obj");
//   ifstream ifs("F16.obj");
//   ifstream ifs("bird.obj");
  ifstream ifs("tetrahedron.obj");
//   ifstream ifs("kocka.obj");
  string temp;

  while( getline( ifs, temp ) )
     text_file.push_back( temp );

  for (int i = 0; i < text_file.size(); i++) {
	  if (text_file.at(i).at(0) == 'f') brF++;
	  else if (text_file.at(i).at(0) == 'v') brV++;
  }

  vrhovi = (Vrh *) malloc (brV * sizeof(Vrh));
  poligoni = (Poligon *) malloc (brF * sizeof(Poligon));


  int oVrhovi = 0, oPoligoni = 0;

  for (int i = 0; i < text_file.size(); i++) {
	  if (text_file.at(i).at(0) == 'v') {
		  float d1, d2, d3;
		  char *text = new char[text_file.at(i).size()+1];
		  std::copy(text_file.at(i).begin(), text_file.at(i).end(), text);
		  sscanf(text, "v %f %f %f", &d1, &d2, &d3);
		  d1 *= 4; d2 *= 4; d3 *= 4;
		  srediste.x += d1; srediste.y += d2; srediste.z += d3;
		  Vrh v(d1, d2, d3);
		  vrhovi[oVrhovi] = v;
		  oVrhovi++;
	  }
  }
  srediste.x /= oVrhovi; srediste.y /= oVrhovi; srediste.z /= oVrhovi;
  printf ("srediste %f %f %f\n", srediste.x, srediste.y, srediste.z);
  //cout<<"Vrhovi: ";
  //for (int i = 0; i < brV; i++) {
	 // cout<<vrhovi[i].x<<" "<<vrhovi[i].y<<" "<<vrhovi[i].z<<"\n";
  //}

  for (int i = 0; i < text_file.size(); i++) {
	  if (text_file.at(i).at(0) == 'f') {
		  int d1, d2, d3;
		  char *text = new char[text_file.at(i).size()+1];
		  std::copy(text_file.at(i).begin(), text_file.at(i).end(), text);
		  sscanf(text, "f %d %d %d", &d1, &d2, &d3);
		  Poligon p(&vrhovi[d1-1], &vrhovi[d2-1], &vrhovi[d3-1]);
		  poligoni[oPoligoni] = p;
		  oPoligoni++;
		  //cout<<"Prvi vrh: "<<p.v1->x<<" "<<p.v1->y<<" "<<p.v1->z<<"\n";
		  //cout<<"Drugi vrh: "<<p.v2->x<<" "<<p.v2->y<<" "<<p.v2->z<<"\n";
		  //cout<<"Treci vrh: "<<p.v3->x<<" "<<p.v3->y<<" "<<p.v3->z<<"\n";
	  }
  }

  text_file.clear();
  ifstream ifs2("bSpline.txt");
  string temp2;

  while( getline( ifs2, temp2 ) )
     text_file.push_back( temp2 );
  
  brB = text_file.size();
  vrhB = (Vrh *) malloc (brB * sizeof(Vrh));
  brS = brB - 3;
	

  for (int i = 0; i < brB; i++) {
	float d1, d2, d3;
	char *text = new char[text_file.at(i).size()+1];
	std::copy(text_file.at(i).begin(), text_file.at(i).end(), text);
	sscanf(text, "%f %f %f", &d1, &d2, &d3);
	Vrh v(d1, d2, d3);
	vrhB[i] = v;
  }

  for (int i = 0; i < brS; i++) {
	  Vrh v1 = vrhB[i]; Vrh v2 = vrhB[i+1];
	  Vrh v3 = vrhB[i+2]; Vrh v4 = vrhB[i+3];

	  //printf("Segment %d\n", i);
	  //printf("Vrh 1: %f %f %f\n", v1.x, v1.y, v1.z);
	  //printf("Vrh 2: %f %f %f\n", v2.x, v2.y, v2.z);
	  //printf("Vrh 3: %f %f %f\n", v3.x, v3.y, v3.z);
	  //printf("Vrh 4: %f %f %f\n\n", v4.x, v4.y, v4.z);
  }

  bSpline = (Vrh *) malloc (100 * brS * sizeof(Vrh));
  tanB = (Vrh *) malloc (brS *8 * sizeof(Vrh));
  sveTanB = (Vrh *) malloc (brS * 2 * 100 * sizeof(Vrh));
  int brojTan = 0, brojSvihTan = 0;
  // Za svaki segment
  for (int i = 0; i < brS; i++) {
	  Vrh v0 = vrhB[i];
	  Vrh v1 = vrhB[i+1];
	  Vrh v2 = vrhB[i+2];
	  Vrh v3 = vrhB[i+3];

	  printf("Prvi vrh %f %f %f\n", v0.x, v0.y, v0.z);

	  // Faktor t raste od 0 do 1
	  for (int t = 0; t < 100; t++) {
		  double j = t/100.0;
		  float f1 = (- pow(j,3.0) + 3*pow(j,2.0) - 3*j + 1) / 6.0;
		  float f2 = (3*pow(j,3.0) - 6*pow(j,2.0) + 4) / 6.0;
		  float f3 = (- 3*pow(j,3.0) + 3*pow(j,2.0) + 3*j + 1) / 6.0;
		  float f4 = pow(j,3.0) / 6.0;

/*		  printf("j = %f - %f\n", j, (100*(i+j)));
		  printf("%f %f %f %f\n", f1, f2, f3, f4);		*/  

		  bSpline[100*i + t].x = f1*v0.x + f2*v1.x + f3*v2.x + f4*v3.x;
		  bSpline[100*i + t].y = f1*v0.y + f2*v1.y + f3*v2.y + f4*v3.y;
		  bSpline[100*i + t].z = f1*v0.z + f2*v1.z + f3*v2.z + f4*v3.z;
		  
		  if (t % 25 == 0) {
			  float t1 = 0.5 * (-pow(j, 2.0) + 2*j -1);
			  float t2 = 0.5 * (3 * pow(j, 2.0) - 4*j);
			  float t3 = 0.5 * (-3* pow(j, 2.0) + 2*j +1);
			  float t4 = 0.5 * (pow(j, 2.0));

			  tanB[brojTan].x = f1*v0.x + f2*v1.x + f3*v2.x + f4*v3.x;
			  tanB[brojTan].y = f1*v0.y + f2*v1.y + f3*v2.y + f4*v3.y;
			  tanB[brojTan].z = f1*v0.z + f2*v1.z + f3*v2.z + f4*v3.z;
			  brojTan++;

			  float vx = t1*v0.x + t2*v1.x + t3*v2.x + t4*v3.x;
			  float vy = t1*v0.y + t2*v1.y + t3*v2.y + t4*v3.y;
			  float vz = t1*v0.z + t2*v1.z + t3*v2.z + t4*v3.z;
			  
			  tanB[brojTan].x = tanB[brojTan-1].x + vx/3;
			  tanB[brojTan].y = tanB[brojTan-1].y + vy/3;
			  tanB[brojTan].z = tanB[brojTan-1].z + vz/3;
			  brojTan++;
		  }

		float t1 = 0.5 * (-pow(j, 2.0) + 2*j -1);
		float t2 = 0.5 * (3 * pow(j, 2.0) - 4*j);
		float t3 = 0.5 * (-3* pow(j, 2.0) + 2*j +1);
		float t4 = 0.5 * (pow(j, 2.0));

		sveTanB[brojSvihTan].x = f1*v0.x + f2*v1.x + f3*v2.x + f4*v3.x;
		sveTanB[brojSvihTan].y = f1*v0.y + f2*v1.y + f3*v2.y + f4*v3.y;
		sveTanB[brojSvihTan].z = f1*v0.z + f2*v1.z + f3*v2.z + f4*v3.z;
		brojSvihTan++;

		float vx = t1*v0.x + t2*v1.x + t3*v2.x + t4*v3.x;
		float vy = t1*v0.y + t2*v1.y + t3*v2.y + t4*v3.y;
		float vz = t1*v0.z + t2*v1.z + t3*v2.z + t4*v3.z;
			  
		sveTanB[brojSvihTan].x = sveTanB[brojSvihTan-1].x + vx/3;
		sveTanB[brojSvihTan].y = sveTanB[brojSvihTan-1].y + vy/3;
		sveTanB[brojSvihTan].z = sveTanB[brojSvihTan-1].z + vz/3;
		brojSvihTan++;
	  }

  }

  printf("Napravljeno je %d tocaka tangenti\n", brojSvihTan);
  //for(int i = 0; i < brojTan; i+=2) {
	 // printf("Tocak 1: %f %f %f\n", tanB[i].x, tanB[i].y, tanB[i].z);
	 // printf("Tocak 2: %f %f %f\n", tanB[i+1].x, tanB[i+1].y, tanB[i+1].z);
  //}

  //for (int i = 0; i < 100 * brS; i++) {
	 // printf ("%f %f %f\n", bSpline[i].x, bSpline[i].y, bSpline[i].z);
  //}

  // Nakon ucitavanja svih potrebnih podataka
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(sub_width,sub_height);
	glutInitWindowPosition(100,100);
	glutInit(&argc, argv);

	window = glutCreateWindow("1. laboratorijska vjezba iz Racunalne Grafike");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	glutIdleFunc(myIdle);

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
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
 
    glMatrixMode(GL_MODELVIEW);                 // Select The Modelview Matrix
    glLoadIdentity();                           // Reset The Modelview Matrix
	//-------------------------

	glLoadIdentity();							//	jedinicna matrica
	glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );		//	boja pozadine
	glClear( GL_COLOR_BUFFER_BIT );				//	brisanje pozadine
	glPointSize(1.0);							//	postavi velicinu tocke
	glColor3f(0.0f, 0.0f, 0.0f);				//	postavi boju
}

int t = 0;

void myDisplay()
{
	glLoadIdentity();
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTranslatef(-5.0, -5.0, -75.0);
	glBegin (GL_LINE_STRIP);
	for(int i = 0; i < 100*brS; i++) {
		glVertex3f(bSpline[i].x, bSpline[i].y, bSpline[i].z);
	}
	glEnd();
	glBegin (GL_LINES);
	for (int i = 0; i < brS*8; i+=2) {
		glVertex3f(tanB[i].x, tanB[i].y, tanB[i].z);
		glVertex3f(tanB[i+1].x, tanB[i+1].y, tanB[i+1].z);
	}
	glEnd();

	glTranslatef(bSpline[t].x, bSpline[t].y, bSpline[t].z);
	
	e.x = sveTanB[2*t+1].x - sveTanB[2*t].x;
	e.y = sveTanB[2*t+1].y - sveTanB[2*t].y;
	e.z = sveTanB[2*t+1].z - sveTanB[2*t].z;
	
	os.x = s.y*e.z - e.y*s.z;
	os.y = e.x*s.z - s.x*e.z;
	os.z = s.x*e.y - s.y*e.x;

	double apsS = pow(pow((double)s.x, 2.0) + pow((double)s.y, 2.0) + pow((double)s.z, 2.0), 0.5);
	double apsE = pow(pow((double)e.x, 2.0) + pow((double)e.y, 2.0) + pow((double)e.z, 2.0), 0.5);
	double se = s.x*e.x + s.y*e.y + s.z*e.z;
	double kut = acos(se/(apsS*apsE));
	kut = kut / (2*pi) * 360;
	glRotatef(kut, os.x, os.y, os.z);
	
	glTranslatef(-srediste.x, -srediste.y, -srediste.z);

	glBegin(GL_LINES);
	for (int i = 0; i < brF; i++) {
		Vrh v1 = *poligoni[i].v1;
		Vrh v2 = *poligoni[i].v2;
		Vrh v3 = *poligoni[i].v3;
		
		glVertex3f(v1.x, v1.y, v1.z);
		glVertex3f(v2.x, v2.y, v2.z);

		glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v3.x, v3.y, v3.z);

		glVertex3f(v3.x, v3.y, v3.z);
		glVertex3f(v1.x, v1.y, v1.z);
	}
	
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(srediste.x, srediste.y, srediste.z);
	glVertex3f(srediste.x + 2.5, srediste.y, srediste.z);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(srediste.x, srediste.y, srediste.z);
	glVertex3f(srediste.x, srediste.y + 2.5, srediste.z);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(srediste.x, srediste.y, srediste.z);
	glVertex3f(srediste.x, srediste.y, srediste.z + 2.5);

	glColor3f(0.0, 0.0, 0.0);
	glEnd();
	/*glBegin(GL_POINTS);
	glPointSize(15.0);
	glVertex3f(vrhB[0].x, vrhB[0].y, vrhB[0].z);
	glVertex3f(vrhB[1].x, vrhB[1].y, vrhB[1].z);
	glVertex3f(vrhB[2].x, vrhB[2].y, vrhB[2].z);
	glVertex3f(vrhB[3].x, vrhB[3].y, vrhB[3].z);
	glEnd();*/
	
	t++;
	if (t == 100*brS) t = 0;

	glFlush();
}

int currentTime = 0; int previousTime = 0;

void myIdle() {
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	int timeInterval = currentTime - previousTime;
	//printf("%d\n", timeInterval);
	if(timeInterval > 10) {
		myDisplay();
		previousTime = currentTime;
	}
}