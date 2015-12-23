#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "glew.h"
#include "glut.h"
#include "mesh.h"
#include "FreeImage.h"
#include "Vector3D.h"

#define NUM_TEXTURE 100
unsigned int texObject[NUM_TEXTURE];
using namespace std;
using namespace MathTool;

const string localPath = "C:\\Users\\XDEX-pc\\Desktop\\0216032V1\\";
const char* viewfile  = "C:\\Users\\XDEX-pc\\Desktop\\0216032V1\\Scene3.view";
const char* lightfile = "C:\\Users\\XDEX-pc\\Desktop\\0216032V1\\Scene3.light";
const char* scenefile = "C:\\Users\\XDEX-pc\\Desktop\\0216032V1\\Scene3.scene";

mesh *objects[1000];
char buf[5010];

int objSum = 0 , windowSize[2] , oldX, oldY, selected = 0 , mouseDown;
double NewX[1001] = { 0 }, NewY[1000] = { 0 } , deltX[1000] = { 0 }, deltY[1000] = { 0 };
double R[3];
bool mohu = false;

char* files[10000];

void light();
void display();
void reshape(GLsizei, GLsizei);
struct View {
	float eye[3], vat[3], vup[3], fovy, dnear, dfar, viewport[4];
}view;
struct Light {
	float pos[3], ambient[3], diffuse[3], specular[3];
};
struct Ambient {
	float data[3];
};
struct Scene {
	static const int no = -1 , single = 0, multi = 1, cube = 2;
	int textType , beginID;
	string fileName;
	 
	float scale[3], angle, angles[3], transfer[3];
	

};

vector<string> textPath;
vector< Light > lights;
vector< Ambient > ambients;
vector< Scene > scenes;

struct ReadView {
	ReadView(const char* path, View &view) {
		ifstream fin(path);
		string OP;
		while (fin >> OP) {
			if (OP == "eye") {
				for (int i = 0; i < 3; i++)
					fin >> view.eye[i];
			}
			else if (OP == "vat") {
				for (int i = 0; i < 3; i++)
					fin >> view.vat[i];
			}
			else if (OP == "vup") {
				for (int i = 0; i < 3; i++)
					fin >> view.vup[i];
			}
			else if (OP == "fovy") {
				fin >> view.fovy;
			}
			else if (OP == "dnear") {
				fin >> view.dnear;
			}
			else if (OP == "dfar") {
				fin >> view.dfar;
			}
			else if (OP == "viewport") {
				for (int i = 0; i < 4; i++) {
					fin >> view.viewport[i];
				}
			}

		}
		fin.close();
	}
};
struct ReadLight {
	ReadLight(const char* path, vector<Light> &lightVec, vector<Ambient> &ambientVec) {
		ifstream fin(path);
		string OP;
		
		cout << fin.eof();

		while (fin >> OP) {
			if (OP == "light") {
				Light light;
				for (int i = 0; i < 3; i++)
					fin >> light.pos[i];
				for (int i = 0; i < 3; i++)
					fin >> light.ambient[i];
				for (int i = 0; i < 3; i++)
					fin >> light.diffuse[i];
				for (int i = 0; i < 3; i++)
					fin >> light.specular[i];
				lightVec.push_back(light);
			}
			else if (OP == "ambient") {
				Ambient ambient;
				for (int i = 0; i < 3; i++)
					fin >> ambient.data[i];
				ambientVec.push_back(ambient);
			}

		}
		fin.close();

	}
};
struct ReadScene {
	ReadScene(const char* path, vector<Scene> &sceneVec) {
		ifstream fin(path);	
		Scene s;
		string OP , tmp2;
		int textType = 0 , beginID = 0;

		while (fin >> OP) {
			if (OP == "model") {
				Scene scene;
				fin >> scene.fileName;
				string tmp = scene.fileName;
				files[objSum] = new char[101];
				strcpy(files[objSum++], tmp.c_str());
				for (int i = 0; i < 3; i++)
					fin >> scene.scale[i];
				fin >> scene.angle;
				for (int i = 0; i < 3; i++)
					fin >> scene.angles[i];
				for (int i = 0; i < 3; i++)
					fin >> scene.transfer[i];

				scene.beginID = beginID;
				scene.textType = textType;
				sceneVec.push_back(scene);
			}
			else if (OP == "no-texture") {
				textType = s.no;
			}
			else if (OP == "single-texture") {
				fin >> tmp2;
				tmp2 = localPath + tmp2;
				textType = s.single;

				beginID = textPath.size();
				textPath.push_back(tmp2);
			}
			else if (OP == "multi-texture") {
				
				textType = s.multi;
				
				char t1[101], t2[101];
				fin.getline(buf, 5000);
				beginID = textPath.size();
				sscanf(buf, "%s %s", t1, t2);
				textPath.push_back(localPath + string(t1));
				textPath.push_back(localPath + string(t2));
				
			}
			else if (OP == "cube-map") {

				textType = s.cube;
				
				beginID = textPath.size();
				for (int i = 0; i < 6; i++) {
					fin >> tmp2;
					textPath.push_back(localPath + tmp2);

				}

			}
		}
		fin.close();

	}

};

void LoadTexture( Scene &scene  , int id){
	
	int endID = 0;
	if (scene.textType == scene.no)
		endID = scene.beginID-1;
	else if (scene.textType == scene.single)
		endID = scene.beginID ;
	else if (scene.textType == scene.multi)
		endID = scene.beginID + 1;
	else {
		endID = scene.beginID + 5;
	}
	
	for (int i = scene.beginID , delta = 0; i < endID; i++ , delta++) {

		const char* pFilename = textPath[i].c_str();
		
		FIBITMAP* bitmap = FreeImage_Load(FreeImage_GetFileType(pFilename, 0), pFilename);
		FIBITMAP* bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
		int bitmapWidth = FreeImage_GetWidth(bitmap32);
		int bitmapHeight = FreeImage_GetHeight(bitmap32);

		int textureType = GL_TEXTURE_2D;
		int textureMode = GL_TEXTURE_2D;

		if (scene.textType == scene.cube) {
			textureType = GL_TEXTURE_CUBE_MAP;
			switch (delta) {
				case 0:
					textureMode = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
					break;
				case 1:
					textureMode = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
					break;
				case 2:
					textureMode = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
					break;
				case 3:
					textureMode = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
					break;
				case 4:
					textureMode = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
					break;
				case 5:
					textureMode = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
					break;
			}

		}
		

		glBindTexture(textureType, texObject[id]);
		glTexImage2D(textureMode, 0, GL_RGBA8, bitmapWidth, bitmapHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(bitmap32));
		glGenerateMipmap(textureType);
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		float largest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest);
		glTexParameterf(textureType, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, largest);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		FreeImage_Unload(bitmap32);
		FreeImage_Unload(bitmap);

		if (scene.textType == scene.multi) id++;
	}
}

void Refresh(int sel) {
	deltX[sel] += NewX[sel];
	NewX[sel] = 0;
	deltY[sel] += NewY[sel];
	NewY[sel] = 0;
}
void drawF();
void zoom(int val) {
	val *= -1;
	float tmp[3], length = 0;
	float delta[3] = { 0 };
	for (int i = 0; i < 3; i++) {
		tmp[i] = view.eye[i] - view.vat[i];
		length += tmp[i] * tmp[i];
	}
	length = sqrt(length);
	for (int i = 0; i < 3; i++)
		view.eye[i] += tmp[i] / length * val;
}

void rotate(int val) {
	val *= -1;
	float tmp[4], length = 0, up[4] , len = 0, nlen = 0;
	float cross[4] = { 0 };
	for (int i = 0; i < 3; i++) {
		tmp[i] = view.eye[i] - view.vat[i];
		len += tmp[i] * tmp[i];
	}

	tmp[3] = tmp[0];
	for (int i = 0; i < 3; i++)
		up[i] = view.vup[i];
	up[3] = up[0];

	for (int i = 0; i < 3; i++)
		cross[(i + 2) % 3] = tmp[i] * up[i + 1] - tmp[i + 1] * up[i];


	for (int i = 0; i < 3; i++)
		length += cross[i] * cross[i];
	length = sqrt(length);

	for (int i = 0; i < 3; i++) {
		tmp[i] += cross[i] / length * val;
		nlen += tmp[i] * tmp[i] ;
	}

	nlen = sqrt(nlen);
	len = sqrt(len);
	float newlen = len / nlen;

	for (int i = 0; i < 3; i++)
		view.eye[i] = view.vat[i] + tmp[i] * newlen;

}

void keyboard(unsigned char key, int x, int y)
{
	if (key <= '9' && '1' <= key) {
		int i = key - '1';
		Refresh(selected);
		if (i < objSum) {
			selected = i;
		}
	}
	
	switch (key){
	case 'r':
		mohu = !mohu;
		break;
	case 'w':
		zoom(1);
		glutPostRedisplay();
		break;
	case 's':
		zoom(-1);
		glutPostRedisplay();
		break;
	case 'a':
		rotate(-1);
		glutPostRedisplay();
		break;
	case 'd':
		rotate(1);
		glutPostRedisplay();
		break;
	}

	
}

void mouseClicks(int button, int state, int x, int y) {
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			mouseDown = 1;
			oldX = x, oldY = y;
		}
		else {
			mouseDown = 0;
			Refresh(selected);
		}
		glutPostRedisplay();
		break;
	};
}

void mouseMove(int x, int y) {

	NewX[selected] = (x - oldX) / 100.0;
	NewY[selected] = -(y - oldY) / 100.0;

	glutPostRedisplay();
}


int main(int argc, char** argv){

	ReadView viewReader(viewfile, view);
	ReadLight lightReader(lightfile, lights, ambients);
	ReadScene sceneReader(scenefile, scenes);

	Vector3D aaa;

	for (int i = 0; i < 3; i++)
		R[i] = view.eye[i] - view.vat[i];
	for (int i = 0; i < objSum; i++)
		objects[i] = new mesh(files[i]);
	
	glutInit(&argc, argv);
	glutInitWindowSize(view.viewport[2], view.viewport[3]);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	glutCreateWindow("CGHW2");

	glewInit();

	FreeImage_Initialise();
	glGenTextures(NUM_TEXTURE, texObject);
	for (int i = 0; i < scenes.size(); i++) {
		if ( (i > 0 && !( scenes[i].beginID == scenes[i-1].beginID && scenes[i].textType == scenes[i-1].textType )) || i == 0 )
		LoadTexture( scenes[i] ,  scenes[i].beginID);
		
	}
	FreeImage_DeInitialise();


	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseClicks);
	glutMotionFunc(mouseMove);
	glutMainLoop();

	return 0;
}

void ambientl() {
	glShadeModel(GL_SMOOTH);
	// z buffer enable
	glEnable(GL_DEPTH_TEST);
	// enable lighting 
	glEnable(GL_LIGHTING);
	// set light property
	float tmp[4] = { 0 };
	for (int i = 0; i < lights.size(); i++) {
		Light light = lights[i];
		glEnable(GL_LIGHT0 + i);
		glLightfv(GL_LIGHT0 + i, GL_POSITION, light.pos);
		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, tmp);
		glLightfv(GL_LIGHT0 + i, GL_SPECULAR, tmp);
		glLightfv(GL_LIGHT0 + i, GL_AMBIENT, light.ambient);
	}
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambients[0].data);
}

void light(){
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_position[] = { 150.0, 150.0, 150.0, 1.0 };
	glShadeModel(GL_SMOOTH);
	// z buffer enable
	glEnable(GL_DEPTH_TEST);
	// enable lighting 
	glEnable(GL_LIGHTING);
	// set light property
	for (int i = 0; i < lights.size(); i++) {
		Light light = lights[i];
		glEnable(GL_LIGHT0 + i);
		glLightfv(GL_LIGHT0 + i, GL_POSITION, light.pos);
		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, light.diffuse);
		glLightfv(GL_LIGHT0 + i, GL_SPECULAR, light.specular);
		glLightfv(GL_LIGHT0 + i, GL_AMBIENT, light.ambient);
	}

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambients[0].data);
}

void drawpoly(float* pos)
{
	
	mesh *obj;
	for (int ID = 0;ID < objSum; ID++)
	{
		obj = objects[ID];//object
		glPushMatrix();
		Scene scene = scenes[ID];
		glTranslatef(scene.transfer[0], scene.transfer[1], scene.transfer[2]);
		glRotatef(scene.angle, scene.angles[0], scene.angles[1], scene.angles[2]);
		glScalef(scene.scale[0], scene.scale[1], scene.scale[2]);

		int lastMaterial = -1;//object
		for (size_t i = 0; i < obj->fTotal; ++i)
		{
			// set material property if this face used different material
			if (lastMaterial != obj->faceList[i].m)
			{
				lastMaterial = (int)obj->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT, obj->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, obj->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR, obj->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &obj->mList[lastMaterial].Ns);
			}
			float vtemp[2][3];
			for (int xx = 0;xx < 2;xx++)
				for (int yy = 0;yy < 3;yy++)
				{
					vtemp[xx][yy] = obj->vList[obj->faceList[i][0].v].ptr[yy] - obj->vList[obj->faceList[i][xx + 1].v].ptr[yy];
				}//邊的向量

			Vector3D vt0( vtemp[0]) , vt1( vtemp[1] );
			Vector3D vv0 = vt0.Cross(vt1);
			Vector3D vv1;

			for (int yy = 0;yy < 3;yy++)
				vv1[yy] = obj->vList[obj->faceList[i][0].v].ptr[yy] - pos[yy];//點到光源
			
			if ( vv0.Dot(vv1) < 0)//內積
			{
				glBegin(GL_POLYGON);
				glNormal3fv(obj->nList[obj->faceList[i][1].n].ptr);
				glVertex3fv(obj->vList[obj->faceList[i][1].v].ptr);
				glNormal3fv(obj->nList[obj->faceList[i][0].n].ptr);
				glVertex3fv(obj->vList[obj->faceList[i][0].v].ptr);
				glNormal3fv(obj->nList[obj->faceList[i][0].n].ptr);
				glVertex3f(obj->vList[obj->faceList[i][0].v].ptr[0] + 5 * (obj->vList[obj->faceList[i][0].v].ptr[0] - pos[0]),
					obj->vList[obj->faceList[i][0].v].ptr[1] + 5 * (obj->vList[obj->faceList[i][0].v].ptr[1] - pos[1]),
					obj->vList[obj->faceList[i][0].v].ptr[2] + 5 * (obj->vList[obj->faceList[i][0].v].ptr[2] - pos[2]));
				glNormal3fv(obj->nList[obj->faceList[i][1].n].ptr);
				glVertex3f(obj->vList[obj->faceList[i][1].v].ptr[0] + 5 * (obj->vList[obj->faceList[i][1].v].ptr[0] - pos[0])
					, obj->vList[obj->faceList[i][1].v].ptr[1] + 5 * (obj->vList[obj->faceList[i][1].v].ptr[1] - pos[1])
					, obj->vList[obj->faceList[i][1].v].ptr[2] + 5 * (obj->vList[obj->faceList[i][1].v].ptr[2] - pos[2]));
				glEnd();

				glBegin(GL_POLYGON);
				glNormal3fv(obj->nList[obj->faceList[i][2].n].ptr);
				glVertex3fv(obj->vList[obj->faceList[i][2].v].ptr);
				glNormal3fv(obj->nList[obj->faceList[i][1].n].ptr);
				glVertex3fv(obj->vList[obj->faceList[i][1].v].ptr);
				glNormal3fv(obj->nList[obj->faceList[i][1].n].ptr);
				glVertex3f(obj->vList[obj->faceList[i][1].v].ptr[0] + 5 * (obj->vList[obj->faceList[i][1].v].ptr[0] - pos[0]),
					obj->vList[obj->faceList[i][1].v].ptr[1] + 5 * (obj->vList[obj->faceList[i][1].v].ptr[1] - pos[1]),
					obj->vList[obj->faceList[i][1].v].ptr[2] + 5 * (obj->vList[obj->faceList[i][1].v].ptr[2] - pos[2]));
				glNormal3fv(obj->nList[obj->faceList[i][2].n].ptr);
				glVertex3f(obj->vList[obj->faceList[i][2].v].ptr[0] + 5 * (obj->vList[obj->faceList[i][2].v].ptr[0] - pos[0]),
					obj->vList[obj->faceList[i][2].v].ptr[1] + 5 * (obj->vList[obj->faceList[i][2].v].ptr[1] - pos[1]),
					obj->vList[obj->faceList[i][2].v].ptr[2] + 5 * (obj->vList[obj->faceList[i][2].v].ptr[2] - pos[2]));
				glEnd();

				glBegin(GL_POLYGON);
				glNormal3fv(obj->nList[obj->faceList[i][0].n].ptr);
				glVertex3fv(obj->vList[obj->faceList[i][0].v].ptr);
				glNormal3fv(obj->nList[obj->faceList[i][2].n].ptr);
				glVertex3fv(obj->vList[obj->faceList[i][2].v].ptr);
				glNormal3fv(obj->nList[obj->faceList[i][2].n].ptr);
				glVertex3f(obj->vList[obj->faceList[i][2].v].ptr[0] + 5 * (obj->vList[obj->faceList[i][2].v].ptr[0] - pos[0]),
					obj->vList[obj->faceList[i][2].v].ptr[1] + 5 * (obj->vList[obj->faceList[i][2].v].ptr[1] - pos[1]),
					obj->vList[obj->faceList[i][2].v].ptr[2] + 5 * (obj->vList[obj->faceList[i][2].v].ptr[2] - pos[2]));
				glNormal3fv(obj->nList[obj->faceList[i][0].n].ptr);
				glVertex3f(obj->vList[obj->faceList[i][0].v].ptr[0] + 5 * (obj->vList[obj->faceList[i][0].v].ptr[0] - pos[0]),
					obj->vList[obj->faceList[i][0].v].ptr[1] + 5 * (obj->vList[obj->faceList[i][0].v].ptr[1] - pos[1]),
					obj->vList[obj->faceList[i][0].v].ptr[2] + 5 * (obj->vList[obj->faceList[i][0].v].ptr[2] - pos[2]));
				glEnd();
			}

		}

		glPopMatrix();
	}//object
}

void display(){

	// clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      //清除用color
	glClearDepth(1.0f);                        // Depth Buffer (就是z buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//這行把畫面清成黑色並且清除z buffer

													   // viewport transformation
													   //	glViewport(0, 0, windowSize[0], windowSize[1]);



	glViewport(view.viewport[0], view.viewport[1], view.viewport[2], view.viewport[3]);
	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(view.fovy, (GLfloat)view.viewport[2] / (GLfloat)view.viewport[3], view.dnear, view.dfar);

	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(view.eye[0], view.eye[1], view.eye[2],// eye
		view.vat[0], view.vat[1], view.vat[2],     // center
		view.vup[0], view.vup[1], view.vup[2]);    // up


	if ( mohu ) {

		glClear(GL_ACCUM_BUFFER_BIT);
		for (int xx = -5;xx < 5;xx++)
		{
			for (int yy = -5;yy < 5;yy++)
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				gluLookAt(view.eye[0] + 0.1*xx, view.eye[1] + 0.1*yy, view.eye[2],
					view.vat[0], view.vat[1], view.vat[2],
					view.vup[0], view.vup[1], view.vup[2]);

				glEnable(GL_STENCIL_TEST);
				glClearStencil(0);
				//pass1
				glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				ambientl();
				drawF();
				//pass2
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);// color buffer
				glDepthMask(GL_FALSE);//depth buffer
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);//enabl back face
				glStencilFunc(GL_ALWAYS, 0, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

				float* lightpos = lights[0].pos;
				drawpoly(lightpos);
				glDisable(GL_CULL_FACE);
				//pass3
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				glStencilFunc(GL_ALWAYS, 0, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
				drawpoly(lightpos);
				glDisable(GL_CULL_FACE);
				//pass4
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				glStencilFunc(GL_EQUAL, 0, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				light();
				drawF();

				glAccum(GL_ACCUM, 0.01);
			}
		}
		glAccum(GL_RETURN, 1.0);

	}
	else {
		glEnable(GL_STENCIL_TEST);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearStencil(0);

		//pass1
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		ambientl();
		drawF();
		//pass2
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);// color buffer
		glDepthMask(GL_FALSE);//depth buffer
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);//enabl back face
		glStencilFunc(GL_ALWAYS, 0, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		float* lightpos = lights[0].pos;
		drawpoly(lightpos);
		glDisable(GL_CULL_FACE);
		//pass3
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glStencilFunc(GL_ALWAYS, 0, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		drawpoly(lightpos);
		glDisable(GL_CULL_FACE);
		//pass4
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilFunc(GL_EQUAL, 0, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		light();
		drawF();


	}



	glutSwapBuffers();
}

void drawF() {

	int lastMaterial = -1;

	for (int k = 0; k < objSum; k++) {

		bool en = false;
		mesh* object = objects[k];
		glPushMatrix();//儲存現在的矩陣 (目前是modelview)
		int ptr = k;

		Scene scene = scenes[ptr];

		glTranslatef(scene.transfer[0], scene.transfer[1], scene.transfer[2]);
		glRotatef(scene.angle, scene.angles[0], scene.angles[1], scene.angles[2]);
		glScalef(scene.scale[0], scene.scale[1], scene.scale[2]);

		switch (scene.textType) {
		case scene.single:
			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texObject[scene.beginID]);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			break;
		case scene.multi:

			for (int multi = 0; multi < 2; multi++) {
				glActiveTexture(GL_TEXTURE0 + multi);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texObject[scene.beginID + multi]);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			}

			break;
		case  scene.cube:
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, texObject[scene.beginID]);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			break;
		default:
			break;
		}


		for (size_t i = 0; i < object->fTotal; ++i) {
			if (1) {

				lastMaterial = (int)object->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT, object->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, object->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR, object->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &object->mList[lastMaterial].Ns);
			}


			glBegin(GL_TRIANGLES);
			for (size_t j = 0; j < 3; ++j) {

				switch (scene.textType) {
				case scene.single:
					glTexCoord2fv(object->tList[object->faceList[i][j].t].ptr);
					break;
				case scene.multi:
					glMultiTexCoord2fv(GL_TEXTURE0, object->tList[object->faceList[i][j].t].ptr);
					glMultiTexCoord2fv(GL_TEXTURE1, object->tList[object->faceList[i][j].t].ptr);
					break;
				case scene.cube:
					glTexCoord2fv(object->tList[object->faceList[i][j].t].ptr);
					break;
				}

				glNormal3fv(object->nList[object->faceList[i][j].n].ptr);

				float tmp[3];
				for (int a = 0; a < 3; a++)
					tmp[a] = object->vList[object->faceList[i][j].v].ptr[a];
				tmp[0] += deltX[k] + NewX[k];
				tmp[1] += deltY[k] + NewY[k];

				glVertex3fv(tmp);
			}
			glEnd();
		}

		switch (scene.textType) {
		case scene.single:
			glActiveTexture(GL_TEXTURE0);
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			break;
		case scene.multi:
			for (int mul = 1; mul >= 0; mul--) {
				glActiveTexture(GL_TEXTURE0 + mul);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
			}

			break;
		case  scene.cube:
			glDisable(GL_TEXTURE_CUBE_MAP);
			glDisable(GL_TEXTURE_GEN_R);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_S);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			break;
		}

		glPopMatrix();

	}
}


void reshape(GLsizei w, GLsizei h){
	windowSize[0] = w;
	windowSize[1] = h;
}

