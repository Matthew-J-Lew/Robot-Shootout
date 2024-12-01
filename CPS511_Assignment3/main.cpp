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
#include "surfaceModeller.h"

const double M_PI = 3.14159265358979323846;

GLint windowWidth = 1280;
GLint windowHeight = 720;
GLint viewportWidth = windowWidth;
GLint viewportHeight = windowHeight;
GLdouble aspect = (GLdouble)windowWidth / windowHeight;

//Creating variables and function headers for camera control functionality and cannon rotation funcionality
GLdouble fov = 90;
GLdouble cameraAzimuth = 0;
GLdouble cameraElevation = 0;
GLdouble zoomAmount = 0.25;
GLdouble azimuthRad;
GLdouble elevationRad;
GLdouble lookAtX;
GLdouble lookAtY;
GLdouble lookAtZ;
void updateCamera();
GLdouble eyeX = 0.0, eyeY = 3.0, eyeZ = 10.0;
GLdouble radius = eyeZ;
GLdouble zNear = 0.1, zFar = 40.0;
int lastMouseX;
int lastMouseY;
int currentButton;
GLdouble cannonRotationAngle = 0.0;

//Creating variables for texture mapping functionality
GLuint groundTexture;

//Creating necessary variables and function headers to import and draw cannon mesh
Vector3D* cannonVertexArray;
GLuint* cannonIndexArray;
Vector3D* cannonNormalArray;

GLuint cannonNumVertices;
GLuint cannonNumQuads;

GLuint cannonIndex = 0;
Vector3D cannonVertex;
Vector3D cannonNormal;
GLuint cannonVertexIndex[4];
FILE* cannonFile;

//Creating necessary variables and function headers for projectiles and their animations
struct Projectile {
    //Positions
    GLdouble x, y, z;      
    //Velocity
    GLdouble vx, vy, vz;  
};
GLdouble deltaTime = 0.016;
std::vector<Projectile> projectiles;
double cannonLength;
double projectileSpeed = 0.5;


void initializeWindow() {

    glViewport(0, 0, viewportWidth, viewportHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, zNear, zFar); 
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); 
    gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

}

void updateCamera(){

    azimuthRad = cameraAzimuth * M_PI / 180.0;
    elevationRad = cameraElevation * M_PI / 180.0;

    // Calculate the direction the camera is looking
    lookAtX = cos(elevationRad) * sin(azimuthRad);
    lookAtY = sin(elevationRad);
    lookAtZ = cos(elevationRad) * cos(azimuthRad);

    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ, eyeX + lookAtX, eyeY + lookAtY, eyeZ + lookAtZ, 0.0, 1.0, 0.0);

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

void drawDefensiveCannon(){
    
    //WE NEED A BETTER WAY TO DO THE TEXTURING
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    glPushMatrix();

    //Moving cannon into place and aligning it with camera
    glTranslatef(0, 1.5, 10);
    glRotatef(-90.0, 0.0, 1.0, 0.0);
    glRotatef(-90.0, 0.0, 0.0, 1.0);

    //Updating cannon to look where our camera is looking
    //Horizontal Movement
    glRotatef(-cameraAzimuth, 1.0, 0.0, 0.0);
    //Vertical Movement
    glRotatef(cameraElevation, 0.0, 0.0, 1.0); 

    glBegin(GL_QUADS);
    //Loop through each quad
    for (int i = 0; i < cannonNumQuads; i++) {

        //Get the index of the vertex and normal to draw based on indexArray[i]
        cannonIndex = cannonIndexArray[i];
        cannonVertex = cannonVertexArray[cannonIndex];
        cannonNormal = cannonNormalArray[cannonIndex];
        
        glTexCoord2f(0.0f, 0.0f);
        glNormal3f(cannonNormal.x, cannonNormal.y, cannonNormal.z);
        glVertex3f(cannonVertex.x, cannonVertex.y, cannonVertex.z);
    }
    glEnd();
    glPopMatrix();
}

void drawProjectiles() {


    GLUquadric* quadric = gluNewQuadric();
    glPushMatrix();
    for (const auto& projectile : projectiles) {
        glPushMatrix();
        glTranslatef(projectile.x, projectile.y, projectile.z);
        glColor3f(0.0f, 1.0f, 0.0f);
        //glutSolidSphere(0.01, 16, 16); // Example: Draw as a small sphere
        gluSphere(quadric, 0.01, 16, 16);
        glPopMatrix();
    }
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    gluDeleteQuadric(quadric);
}

void updateProjectiles(GLdouble deltaTime) {
    for (auto& projectile : projectiles) {
        projectile.x += projectile.vx * deltaTime;
        projectile.y += projectile.vy * deltaTime;
        projectile.z += projectile.vz * deltaTime;
    }
    // Optionally, remove projectiles that are out of bounds
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const Projectile& p) { return p.y < -1.0 || p.x > 100 || p.z > 100; }),
        projectiles.end());
}

void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    updateCamera();
    drawGround();
    drawDefensiveCannon();

    updateProjectiles(deltaTime);
    drawProjectiles();

    glutSwapBuffers();

}

//Using this function for our camera movement
void mouseMotionHandler(int x, int y){

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    if (currentButton == GLUT_LEFT_BUTTON) {
        
        cameraAzimuth -= dx * 0.25;  
        cameraElevation -= dy * 0.25;
        // Clamp elevation to avoid flipping the view upside-down
        if (cameraElevation > 52.0)
            cameraElevation = 52.0;
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

void keyboardHandler(unsigned char key, int x, int y) {

    switch (key) {

    case 32: {
      
        Projectile newProjectile;

        //Calculate the position of the cannon's tip
        cannonLength = 0.225; 
        azimuthRad = cameraAzimuth * M_PI / 180.0;
        elevationRad = cameraElevation * M_PI / 180.0;

        //Set where the projectile will spawn
        newProjectile.x = eyeX + cannonLength * cos(elevationRad) * sin(azimuthRad);
        newProjectile.y = eyeY + cannonLength * sin(elevationRad) - 0.075;
        newProjectile.z = eyeZ + cannonLength * cos(elevationRad) * cos(azimuthRad);

        //Set the velocity of the projectile
        projectileSpeed = 0.5; 
        newProjectile.vx = projectileSpeed * cos(elevationRad) * sin(azimuthRad);
        newProjectile.vy = projectileSpeed * sin(elevationRad);
        newProjectile.vz = projectileSpeed * cos(elevationRad) * cos(azimuthRad);

        //Add to the list of projectiles
        projectiles.push_back(newProjectile);
        break;

    }
    case 'r':
        printf("Restart");
        break;
    default:
        break;
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

//Function to import the .obj mesh file and bind the extracted data to our arrays, note we use the variables vertex, and normal in this function as well as drawQuads
void importMesh() {

    //Open file
    errno_t err = fopen_s(&cannonFile, "meshes/cannon_mesh_export.obj", "r");
    if (err != 0 || !cannonFile) {
        fprintf(stderr, "Error reading cannon file\n");
        return;
    }

    //Temporary lists to extract the data with
    std::vector<Vector3D> vertices;
    std::vector<Vector3D> normals;
    std::vector<GLuint> indices;

    //line header is used to scan the first characters of the line to tell if its a vertex, normal, or index
    char lineHeader[128];
    //While theres still lines in the file, scan each line and bind to the appropriate array
    while (fscanf_s(cannonFile, "%s", lineHeader, (unsigned)_countof(lineHeader)) != EOF) {

        //Case where line starts with v, it is a vertex, so we bind it to our temporary vertex list
        if (strcmp(lineHeader, "v") == 0) {

            fscanf_s(cannonFile, "%lf %lf %lf\n", &cannonVertex.x, &cannonVertex.y, &cannonVertex.z);
            vertices.push_back(cannonVertex);

        }

        //Case where line starts with vn, it is a normal, so we bind it to our temporary normal list
        else if (strcmp(lineHeader, "vn") == 0) {

            fscanf_s(cannonFile, "%lf %lf %lf\n", &cannonNormal.x, &cannonNormal.y, &cannonNormal.z);
            normals.push_back(cannonNormal);

        }
        //Case where line starts with an f, it is an index, so we bind it to our temporary indice list
        else if (strcmp(lineHeader, "f") == 0) {

            //We have another loop as there are 4 vertices for each quad
            for (int i = 0; i < 4; i++) {
                fscanf_s(cannonFile, "%u//%u", &cannonVertexIndex[i], &cannonVertexIndex[i]);
                cannonVertexIndex[i]--; // Convert from 1-based OBJ index to 0-based C/C++ index
            }
            indices.insert(indices.end(), cannonVertexIndex, cannonVertexIndex + 4);

        }
    }
    fclose(cannonFile);

    //Transfer data from vectors to arrays

    //cannonNumVertices and cannonNumQuads are useful for drawing later in drawQuads
    cannonNumVertices = (int)vertices.size();
    cannonNumQuads = (int)(indices.size() / 4);

    //creates memory for arrays and then copies the data from our temporary lists into our arrays
    cannonVertexArray = (Vector3D*)malloc(cannonNumVertices * sizeof(Vector3D));
    memcpy(cannonVertexArray, vertices.data(), cannonNumVertices * sizeof(Vector3D));

    cannonNormalArray = (Vector3D*)malloc(cannonNumVertices * sizeof(Vector3D));
    memcpy(cannonNormalArray, normals.data(), cannonNumVertices * sizeof(Vector3D));

    cannonIndexArray = (GLuint*)malloc(indices.size() * sizeof(GLuint));
    memcpy(cannonIndexArray, indices.data(), indices.size() * sizeof(GLuint));

    printf("Mesh imported from %s successfully.\n", "mesh_export.obj");
}

int main(int argc, char** argv) {
    //Initialize GLUT
    importMesh();
    glutInit(&argc, argv);

    //Creating the window
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);

    if (glutCreateWindow("Assignment 3 - Robot Attack") == 0) {
        printf("Error: Window creation failed.\n");
        return 1;
    }

    //Loading textures
    groundTexture = loadGroundBMP("textures/snow.bmp");


    //Creating the window
    glEnable(GL_DEPTH_TEST);
    initializeWindow();

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutKeyboardFunc(keyboardHandler);
    glutMouseFunc(mouseButtonHandler);
    glutMotionFunc(mouseMotionHandler);

    glutMainLoop();

    return 0;
}