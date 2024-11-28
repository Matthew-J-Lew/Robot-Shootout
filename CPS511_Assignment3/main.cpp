#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/freeglut.h>
#include <utility>
#include <vector>

GLint windowWidth = 1280;
GLint windowHeight = 720;
GLint viewportWidth = windowWidth;
GLint viewportHeight = windowHeight;

// Variables for joint angles
float botAngle = 0.0;
float leftElbowAngle = 0.0, rightElbowAngle = 0.0;
float leftKneeAngle = 0.0, rightKneeAngle = 0.0;
float leftHipAngle = 0.0, rightHipAngle = 0.0;
char activeJoint = ' '; // Currently selected joint

// States for cannon and step animation
bool cannonSpinning = false;
float cannonAngle = 0.0f;
bool botStepping = false;
float stepIncrement = 1.0f;

// Astro Bot body, along with body plate for colour
void drawBody() {
    // Body
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(0.7f, 1.0f, 0.4f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Body plate
    glPushMatrix();
    glTranslatef(0.0f, 0.10f, 0.08f);
    glScalef(0.4f, 0.4f, 0.3f);
    glColor3f(5.0f / 255.0f, 68.0f / 255.0f, 177.0f / 255.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// Asro Bot head, along with face plate, eyes, and antenna
void drawHead() {
    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f);
    glScalef(0.7f, 0.7f, 0.7f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Face plate
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.35f);
    glScalef(0.5f, 0.5f, 0.05f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left eye
    glPushMatrix();
    glTranslatef(-0.11f, 0.82f, 0.35f);
    glScalef(0.1f, 0.1f, 0.1f);
    glColor3f(5.0f / 255.0f, 68.0f / 255.0f, 177.0f / 255.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right eye
    glPushMatrix();
    glTranslatef(0.11f, 0.82f, 0.35f);
    glScalef(0.1f, 0.1f, 0.1f);
    glColor3f(5.0f / 255.0f, 68.0f / 255.0f, 177.0f / 255.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Antenna stalk
    glPushMatrix();
    glTranslatef(0.0f, 1.1f, 0.0f);
    glRotatef(-80.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    gluCylinder(gluNewQuadric(), 0.05f, 0.05f, 0.4f, 20, 20);
    glPopMatrix();

    // Antenna tip
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.07f);
    glColor3f(5.0f / 255.0f, 68.0f / 255.0f, 177.0f / 255.0f);
    glutSolidSphere(0.1f, 20, 20);
    glPopMatrix();
}

// Astro bot arms (upper arm and lower arm) with elbow movement
void drawArm(bool isLeft) {
    glPushMatrix();

    // Upper Arm
    if (isLeft) {
        glTranslatef(-0.50f, 0.2f, 0.0f);
    }
    else {
        glTranslatef(0.50f, 0.2f, 0.0f);
    }

    glRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    gluCylinder(gluNewQuadric(), 0.15f, 0.15f, 0.5f, 20, 20);
    glTranslatef(0.0f, 0.0f, 0.5f);

    // Lower arm with elbow rotation and constraints to mimic real arm movement
    if (isLeft) {
        leftElbowAngle = fmax(-90.0f, fmin(leftElbowAngle, 0.0f));
        glRotatef(leftElbowAngle, 1.0f, 0.0f, 0.0f);
        glColor3f(5.0f / 255.0f, 68.0f / 255.0f, 177.0f / 255.0f);
    }
    else {
        rightElbowAngle = fmax(-90.0f, fmin(rightElbowAngle, 0.0f));
        glRotatef(rightElbowAngle, 1.0f, 0.0f, 0.0f);
        glColor3f(5.0f / 255.0f, 68.0f / 255.0f, 177.0f / 255.0f);
    }

    gluCylinder(gluNewQuadric(), 0.1f, 0.1f, 0.5f, 20, 20);

    glPopMatrix();
}

// Astro Bot legs (upper leg and lower leg) with hip and knee movement
void drawLeg(bool isLeft) {
    glPushMatrix();

    // Upper leg with hip rotation and constraints to mimic real leg movement
    if (isLeft) {
        glTranslatef(-0.22f, -0.5f, 0.0f);
    }
    else {
        glTranslatef(0.22f, -0.5f, 0.0f);
    }

    if (isLeft) {
        leftHipAngle = fmax(-90.0f, fmin(leftHipAngle, 20.0f));
        glRotatef(leftHipAngle, 1.0f, 0.0f, 0.0f);
    }
    else {
        rightHipAngle = fmax(-90.0f, fmin(rightHipAngle, 20.0f));
        glRotatef(rightHipAngle, 1.0f, 0.0f, 0.0f);
    }

    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    gluCylinder(gluNewQuadric(), 0.2f, 0.2f, 0.7f, 20, 20);
    glTranslatef(0.0f, 0.0f, 0.7f);

    // Lower leg with knee rotation and constraints to mimic real leg movement
    if (isLeft) {
        leftKneeAngle = fmax(0.0f, fmin(leftKneeAngle, 90.0f));
        glRotatef(leftKneeAngle, 1.0f, 0.0f, 0.0f);
        glColor3f(5.0f / 255.0f, 68.0f / 255.0f, 177.0f / 255.0f);
    }
    else {
        rightKneeAngle = fmax(0.0f, fmin(rightKneeAngle, 90.0f));
        glRotatef(rightKneeAngle, 1.0f, 0.0f, 0.0f);
        glColor3f(5.0f / 255.0f, 68.0f / 255.0f, 177.0f / 255.0f);
    }

    gluCylinder(gluNewQuadric(), 0.15f, 0.15f, 0.5f, 20, 20);

    glPopMatrix();
}

// Attached cannon with small sub-parts and spinning capabilities
void drawCannon() {
    glPushMatrix();

    // Cannon
    glTranslatef(0.0f, 0.1f, 0.2f);
    glRotatef(cannonAngle, 0.0f, 0.0f, 1.0f);

    glColor3f(0.5f, 0.5f, 0.5f);
    gluCylinder(gluNewQuadric(), 0.1f, 0.1f, 0.5f, 20, 20);

    // Small cube sub-parts
    glPushMatrix();
    glTranslatef(0.15f, 0.0f, 0.45f);
    glScalef(0.1f, 0.1f, 0.1f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.15f, 0.0f, 0.45f);
    glScalef(0.1f, 0.1f, 0.1f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}

// Building Astro Bot
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(botAngle, 0.0f, 1.0f, 0.0f);

    glPushMatrix();

    // Body parts and cannon
    drawBody();
    drawHead();
    drawArm(true);
    drawArm(false);
    drawLeg(true);
    drawLeg(false);
    drawCannon();

    glPopMatrix();

    glutSwapBuffers();
}


void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)width / (float)height, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Input handler for letter keys
void keyboard(unsigned char key, int x, int y) {
    // Select joint to control and/or stop/start cannon and stepping animations
    // Left arm
    if (key == 'q') {
        activeJoint = 'q';
    }
    // Right arm
    else if (key == 'e') {
        activeJoint = 'e';
    }
    // Left leg
    else if (key == 'a') {
        activeJoint = 'a';
    }
    // Right leg
    else if (key == 'd') {
        activeJoint = 'd';
    }
    // Left knee
    else if (key == 'z') {
        activeJoint = 'z';
    }
    // Right knee
    else if (key == 'x') {
        activeJoint = 'x';
    }
    // Start cannon spin
    else if (key == 'c') {
        cannonSpinning = true;
    }
    // Stop cannon spin
    else if (key == 'C') {
        cannonSpinning = false;
    }
    // Start step animation
    else if (key == 'w') {
        botStepping = true;
    }
    // Stop step animation and reset all the angles
    else if (key == 'W') {
        botStepping = false;
        leftHipAngle = 0.0f;
        leftKneeAngle = 0.0f;
        rightHipAngle = 0.0f;
        rightKneeAngle = 0.0f;
        stepIncrement = 1.0f;
    }

    glutPostRedisplay();
}

// Input handler for directional keys
void specialKeyboard(int key, int x, int y) {
    // Control movement based on the active joint
    if (key == GLUT_KEY_UP) {
        if (activeJoint == 'q') {
            leftElbowAngle -= 5.0;
        }
        else if (activeJoint == 'e') {
            rightElbowAngle -= 5.0;
        }
        else if (activeJoint == 'a') {
            leftHipAngle -= 5.0;
        }
        else if (activeJoint == 'd') {
            rightHipAngle -= 5.0;
        }
        else if (activeJoint == 'z') {
            leftKneeAngle -= 5.0;
        }
        else if (activeJoint == 'x') {
            rightKneeAngle -= 5.0;
        }
    }
    else if (key == GLUT_KEY_DOWN) {
        if (activeJoint == 'q') {
            leftElbowAngle += 5.0;
        }
        else if (activeJoint == 'e') {
            rightElbowAngle += 5.0;
        }
        else if (activeJoint == 'a') {
            leftHipAngle += 5.0;
        }
        else if (activeJoint == 'd') {
            rightHipAngle += 5.0;
        }
        else if (activeJoint == 'z') {
            leftKneeAngle += 5.0;
        }
        else if (activeJoint == 'x') {
            rightKneeAngle += 5.0;
        }
    }
    // Viewing angle of the bot
    else if (key == GLUT_KEY_LEFT) {
        botAngle -= 5.0;
    }
    else if (key == GLUT_KEY_RIGHT) {
        botAngle += 5.0;
    }

    glutPostRedisplay();
}

// Implement logic for cannon spinning and bot stepping animation
void animation(int value) {
    // Update the cannon angle if spinning is active
    if (cannonSpinning) {
        cannonAngle += 2.0f;
        if (cannonAngle >= 360.0f) {
            cannonAngle -= 360.0f;
        }
    }

    // Update the step animation if stepping is active
    if (botStepping) {
        // Increment hip and knee angles for the left and right legs
        leftHipAngle -= stepIncrement;
        leftKneeAngle += stepIncrement;
        rightHipAngle += stepIncrement;
        rightKneeAngle += stepIncrement;
    }

    glutPostRedisplay();
    // Animation speed
    glutTimerFunc(16, animation, 0);
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);

    if (glutCreateWindow("Assignment 1 - Astro Bot") == 0) {
        printf("Error: Window creation failed.\n");
        return 1;
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    glutTimerFunc(0, animation, 0);

    glutMainLoop();

    return 0;
}