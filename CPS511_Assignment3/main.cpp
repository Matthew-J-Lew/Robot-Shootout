#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
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
    glPushMatrix();
    glBegin(GL_QUADS);

    glColor3f(0.0f, 2.0f, 0.0f); 
    glVertex3f(-100.0f, -1.0f, -100.0f); 
    glVertex3f(100.0f, -1.0f, -100.0f);  
    glVertex3f(100.0f, -1.0f, 100.0f);   
    glVertex3f(-100.0f, -1.0f, 100.0f);  

    glEnd();
    glPopMatrix();
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
        printf("%f\n", cameraElevation);
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
            printf("Left mouse button clicked at (%d, %d)\n", x, y);
            break;
        case GLUT_MIDDLE_BUTTON:
            printf("Middle mouse button clicked at (%d, %d)\n", x, y);
            break;
        case GLUT_RIGHT_BUTTON:
            printf("Right mouse button clicked at (%d, %d)\n", x, y);
            break;
        default:
            printf("Unknown mouse button clicked at (%d, %d)\n", x, y);
            break;
        }
    }

}



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