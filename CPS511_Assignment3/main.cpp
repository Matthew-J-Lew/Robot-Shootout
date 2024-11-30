#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <utility>
#include <vector>
#include <math.h>
#include <windows.h>

const double M_PI = 3.14159265358979323846;

GLint windowWidth = 1280;
GLint windowHeight = 720;
GLint viewportWidth = windowWidth;
GLint viewportHeight = windowHeight;
GLdouble aspect = (GLdouble)windowWidth / windowHeight;

//Creating variables and function headers for camera control functionality
GLdouble cameraAzimuth = 0;
GLdouble cameraElevation = 3;
GLdouble zoomAmount = 0.25;
GLdouble azimuthRad;
GLdouble elevationRad;
void updateCamera();
GLdouble eyeX = 0.0, eyeY = 3.0, eyeZ = 10.0;
GLdouble radius = eyeZ;
GLdouble zNear = 0.1, zFar = 40.0;
int lastMouseX;
int lastMouseY;
int currentButton;

//Creating variables for texture mapping functionality
GLuint groundTexture;


void initializeWindow() {

    glViewport(0, 0, viewportWidth, viewportHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspect, zNear, zFar); 
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); 

}

void updateCamera() {

    azimuthRad = cameraAzimuth * M_PI / 180.0;    
    elevationRad = cameraElevation * M_PI / 180.0;

    eyeX = 10.0 * cos(elevationRad) * sin(azimuthRad);
    eyeY = 10.0 * sin(elevationRad);
    eyeZ = 10.0 * cos(elevationRad) * cos(azimuthRad);

    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);


}

void drawGround() {
    

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    glPushMatrix();
    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, -1.0f, -100.0f);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(100.0f, -1.0f, -100.0f);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(100.0f, -1.0f, 100.0f);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-100.0f, -1.0f, 100.0f);

    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}


void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    updateCamera();
    drawGround();
    glutSwapBuffers();

}



//Using this function for our camera movement
void mouseMotionHandler(int x, int y){

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    if (currentButton == GLUT_LEFT_BUTTON) {
        
        cameraAzimuth += dx * 0.25;  
        cameraElevation -= dy * 0.25;

        // Clamp elevation to avoid flipping the view upside-down
        if (cameraElevation > 89.0)
            cameraElevation = 89.0;
        if (cameraElevation < -5.0)
            cameraElevation = -5.0;

        glutPostRedisplay(); // Request a redraw to update the view
    }
    lastMouseX = x;
    lastMouseY = y;

}

void mouseButtonHandler(int button, int state, int x, int y) {

    currentButton = button;
    lastMouseX = x;
    lastMouseY = y;

    if (state == GLUT_DOWN) { 
        switch (button) {
        case GLUT_LEFT_BUTTON:
            //printf("Left mouse button clicked at (%d, %d)\n", x, y);
            
            break;
        case GLUT_MIDDLE_BUTTON:
            //printf("Middle mouse button clicked at (%d, %d)\n", x, y);
            
            break;
        case GLUT_RIGHT_BUTTON:
            //printf("Right mouse button clicked at (%d, %d)\n", x, y);
            
            break;
        default:
            //printf("Unknown mouse button clicked at (%d, %d)\n", x, y);
            
            break;
        }
    }

}

GLuint loadGroundBMP(const char* filePath) {
    
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Error: Unable to open texture file %s\n", filePath);
        return 0;
    }

    //Create a variable to hold the header
    char header[54];
    fread(header, sizeof(char), 54, file);


    //Extract image information from the header
    int dataPos = *(int*)&header[0x0A];
    int imageSize = *(int*)&header[0x22];
    int width = *(int*)&header[0x12];
    int height = *(int*)&header[0x16];

    //If we can't get the imageSize or dataPos for whatever reason we calculate it
    if (imageSize == 0) {
        imageSize = width * height * 3;
    }
    if (dataPos == 0) dataPos = 54;

    //Read image data
    char* data = (char*)malloc(imageSize);
    fread(data, sizeof(char), imageSize, file);

    //We need to swap blue with red based on how bmp files are formatted
    for (int i = 0; i < imageSize; i += 3) {
        char temp = data[i];       
        data[i] = data[i + 2];             
        data[i + 2] = temp;
    }
    fclose(file);

    //Generate OpenGL texture
    GLuint groundTextureID;
    glGenTextures(1, &groundTextureID);
    glBindTexture(GL_TEXTURE_2D, groundTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    free(data);

    //Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return groundTextureID;
}

//void loadTextures() {
//    
//    groundTexture = loadGroundBMP("Filepath");
//    if (!groundTexture) {
//        printf("Error: Failed to load ground texture.\n");
//        exit(1);
//    }
//    
//}

int main(int argc, char** argv) {
    //Initialize GLUT
    glutInit(&argc, argv);

    //Creating the window
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);

    if (glutCreateWindow("Assignment 3 - Robot Attack") == 0) {
        printf("Error: Window creation failed.\n");
        return 1;
    }

    //Loading textures
    groundTexture = loadGroundBMP("textures/ground_texture.bmp");


    //Creating the window
    glEnable(GL_DEPTH_TEST);
    initializeWindow();

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMouseFunc(mouseButtonHandler);
    glutMotionFunc(mouseMotionHandler);

    glutMainLoop();

    return 0;
}