#ifndef ROBOT_H
#define ROBOT_H

// Declarations for robot drawing functions
void drawBody();
void drawHead();
void drawArm(bool isLeft);
void drawLeg(bool isLeft);

extern float leftElbowAngle, rightElbowAngle;
extern float leftKneeAngle, rightKneeAngle;
extern float leftHipAngle, rightHipAngle;

#endif
