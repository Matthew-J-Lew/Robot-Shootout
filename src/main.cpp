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
#include "Robot.h"

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
GLuint cannonTexture;

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

//Creating necessary variables and function headers for spawning robots
struct Robot {
    //Positions
    GLdouble x, y, z;
    //Walking speed
    GLdouble speed;
    GLdouble rotationAngle;
    float stepIncrement;

    // Joint angles for walking animation
    float leftHipAngle = 0.0f;
    float rightHipAngle = 0.0f;
    float leftKneeAngle = 0.0f;
    float rightKneeAngle = 0.0f;
};
std::vector<Robot> robots;

//Distance where robots hit the cannon
const float despawnThreshold = 5.0f;


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


void updateRobots(GLdouble deltaTime) {
    // Iterate through robots and update their positions
    auto it = robots.begin();
    while (it != robots.end()) {
        GLdouble dx = 0.0 - it->x; // Cannon at origin in X
        GLdouble dz = 10.0 - it->z; // Cannon at Z = 10
        GLdouble length = sqrt(dx * dx + dz * dz);

        //Move robots as long as they have not hit the cannon
        if (length > despawnThreshold) {
            if (length > 0) {
                dx /= length;
                dz /= length;
                it->x += it->speed * dx * deltaTime;
                it->z += it->speed * dz * deltaTime;
            }
            ++it;
        }
        else {
            // Despawn robot that has reached the cannon
            it = robots.erase(it);
        }
    }
}


void drawGround() {

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
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
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, cannonTexture);

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
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    //Loop through each quad
    for (int i = 0; i < cannonNumQuads; i++) {

        //Get the index of the vertex and normal to draw based on indexArray[i]
        cannonIndex = cannonIndexArray[i];
        cannonVertex = cannonVertexArray[cannonIndex];
        cannonNormal = cannonNormalArray[cannonIndex];
        
        glTexCoord2f(i % 2, (i / 2) % 2); // Adjust these values based on the cannon geometry
        glNormal3f(cannonNormal.x, cannonNormal.y, cannonNormal.z);
        glVertex3f(cannonVertex.x, cannonVertex.y, cannonVertex.z);
    }
    glEnd();
    glPopMatrix();
}

void drawRobots() {
    for (const auto& robot : robots) {
        glPushMatrix();
        float adjustedY = robot.y + 3.0f;
        glTranslatef(robot.x, adjustedY, robot.z);

        // Rotate the robot to face the cannon in the middle
        glRotatef(robot.rotationAngle, 0.0, 1.0, 0.0);

        glScalef(2.0f, 2.0f, 2.0f);

        // Draw robot body parts
        drawBody();
        drawHead();
        drawArm(true);
        drawArm(false);
        drawLeg(true);
        drawLeg(false);

        glPopMatrix();
    }
}

void drawProjectiles() {


    GLUquadric* quadric = gluNewQuadric();
    glPushMatrix();
    for (const auto& projectile : projectiles) {
        glPushMatrix();
        glTranslatef(projectile.x, projectile.y, projectile.z);
        glColor3f(0.0f, 1.0f, 0.0f);
        gluSphere(quadric, 0.01, 16, 16);
        glPopMatrix();
    }
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    gluDeleteQuadric(quadric);
}

void updateProjectiles(GLdouble deltaTime) {
    // Move projectiles
    for (auto& projectile : projectiles) {
        projectile.x += projectile.vx * deltaTime;
        projectile.y += projectile.vy * deltaTime;
        projectile.z += projectile.vz * deltaTime;
    }

    // Check for collisions between projectiles and robots
    auto projectileIt = projectiles.begin();
    while (projectileIt != projectiles.end()) {
        bool collisionDetected = false;

        auto robotIt = robots.begin();
        while (robotIt != robots.end()) {
            // Calculate distance
            GLdouble dx = projectileIt->x - robotIt->x;
            GLdouble dy = projectileIt->y - (robotIt->y + 3.0f); // Adjust for size of robot
            GLdouble dz = projectileIt->z - robotIt->z;

            GLdouble distanceSquared = dx * dx + dy * dy + dz * dz;

            // Adjust collision radius based on scaling
            const GLdouble collisionRadius = 1.0;
            const GLdouble scaledRadius = collisionRadius * 8.0; // Scaled size
            if (distanceSquared < scaledRadius * scaledRadius) {
                // Remove the robot when hit
                robotIt = robots.erase(robotIt);
                collisionDetected = true;
                break;
            }
            else {
                ++robotIt;
            }
        }

        if (collisionDetected) {
            projectileIt = projectiles.erase(projectileIt);
        }
        else {
            ++projectileIt;
        }
    }

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
    
    updateRobots(deltaTime);
    drawRobots();

    updateProjectiles(deltaTime);
    drawProjectiles();

    glutSwapBuffers();
}


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
            break;
        case GLUT_MIDDLE_BUTTON:
            break;
        case GLUT_RIGHT_BUTTON:
            break;
        default:
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
        robots.clear();
        printf("Restart");
        break;
    case 's': {
        if (robots.empty()) {
            int maxRobotsToSpawn = 5;

            for (int i = 0; i < maxRobotsToSpawn; ++i) {
                Robot newRobot;
                newRobot.x = static_cast<GLdouble>(rand() % 200 - 100); // Random x between -100 and 100
                newRobot.y = 0.0;
                newRobot.z = static_cast<GLdouble>(rand() % 200 - 100); // Random z between -100 and 100
                newRobot.speed = 3.0;
                newRobot.stepIncrement = 1.0f;
                newRobot.leftHipAngle = 0.0f;
                newRobot.rightHipAngle = 0.0f;
                newRobot.leftKneeAngle = 0.0f;
                newRobot.rightKneeAngle = 0.0f;

                // Calculate the rotation angle to face the cannon
                GLdouble dx = 0.0 - newRobot.x;
                GLdouble dz = 10.0 - newRobot.z; // Cannon position at z = 10
                newRobot.rotationAngle = atan2(dx, dz) * 180.0 / M_PI;

                robots.push_back(newRobot);
            }
        }
        break;
    }
    default:
        break;
    }
}


GLuint loadCannonTexture(const char* filePath) {
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Error: Unable to open texture file %s\n", filePath);
        return 0;
    }

    char header[54];
    fread(header, sizeof(char), 54, file);

    int dataPos = *(int*)&header[0x0A];
    int imageSize = *(int*)&header[0x22];
    int width = *(int*)&header[0x12];
    int height = *(int*)&header[0x16];

    if (imageSize == 0) {
        imageSize = width * height * 3;
    }
    if (dataPos == 0) dataPos = 54;

    // Read image data
    char* data = (char*)malloc(imageSize);
    fread(data, sizeof(char), imageSize, file);

    //Swap blue with red based on how bmp files are formatted
    for (int i = 0; i < imageSize; i += 3) {
        char temp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = temp;
    }
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat horizontally
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat vertically

    return textureID;
}


GLuint loadGroundBMP(const char* filePath) {
    
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Error: Unable to open texture file %s\n", filePath);
        return 0;
    }

    char header[54];
    fread(header, sizeof(char), 54, file);


    int dataPos = *(int*)&header[0x0A];
    int imageSize = *(int*)&header[0x22];
    int width = *(int*)&header[0x12];
    int height = *(int*)&header[0x16];

    if (imageSize == 0) {
        imageSize = width * height * 3;
    }
    if (dataPos == 0) dataPos = 54;

    //Read image data
    char* data = (char*)malloc(imageSize);
    fread(data, sizeof(char), imageSize, file);

    //Swap blue with red based on how bmp files are formatted
    for (int i = 0; i < imageSize; i += 3) {
        char temp = data[i];       
        data[i] = data[i + 2];             
        data[i + 2] = temp;
    }
    fclose(file);

    GLuint groundTextureID;
    glGenTextures(1, &groundTextureID);
    glBindTexture(GL_TEXTURE_2D, groundTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return groundTextureID;
}

//Function to import the .obj mesh file and bind the extracted data to our arrays, note we use the variables vertex, and normal in this function as well as drawQuads
void importMesh() {

    //Open file
    errno_t err = fopen_s(&cannonFile, "assets/meshes/cannon_mesh_export.obj", "r");
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


void animation(int value) {
    // Update robot positions and animations
    for (auto& robot : robots) {
        // Stepping animation logic from assignment 1
        robot.leftHipAngle -= robot.stepIncrement;
        robot.leftKneeAngle += robot.stepIncrement;
        robot.rightHipAngle += robot.stepIncrement;
        robot.rightKneeAngle += robot.stepIncrement;

        // Reverse direction of stepping animation when limits are reached
        if (robot.leftHipAngle >= 20.0f || robot.leftHipAngle <= -20.0f) {
            robot.stepIncrement = -robot.stepIncrement;
        }

        // Movement logic toward the cannon
        GLdouble dx = 0.0 - robot.x;
        GLdouble dz = 10.0 - robot.z;
        GLdouble length = sqrt(dx * dx + dz * dz);

        if (length > 0) {
            dx /= length;
            dz /= length;
            robot.x += robot.speed * dx * deltaTime;
            robot.z += robot.speed * dz * deltaTime;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, animation, 0);
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
    groundTexture = loadGroundBMP("assets/textures/snow.bmp");
    cannonTexture = loadGroundBMP("assets/textures/cannon.bmp");


    //Creating the window
    glEnable(GL_DEPTH_TEST);
    initializeWindow();

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutKeyboardFunc(keyboardHandler);
    glutMouseFunc(mouseButtonHandler);
    glutMotionFunc(mouseMotionHandler);

    glutTimerFunc(16, animation, 0);

    glutMainLoop();

    return 0;
}