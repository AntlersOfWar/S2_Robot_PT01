#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <FEHRPS.h>
#include <FEHServo.h>
#define WHEEL_RADIUS 1.375
#define COUNTS_PER_REV 48
#define PI 3.14159265
#define ROBOT_RADIUS 4.0
#define OFFSET 5.0

//Starting orientation of robot is going to be the default.
bool defaultOrientation = true;
float redThresh = 0.7;

/* TODO: Make sure you have the right number of motors/encoders declared. */
FEHServo main_servo(FEHServo::Servo7);
FEHServo lever_servo(FEHServo::Servo6);

//P3_6, and P3_7 cannot be used for digital encoders.
//fl means front-left, br means back-right.
DigitalEncoder fl_encoder(FEHIO::P0_0);
DigitalEncoder fr_encoder(FEHIO::P0_2);
DigitalEncoder bl_encoder(FEHIO::P0_3);
DigitalEncoder br_encoder(FEHIO::P0_1);

FEHMotor fl_motor(FEHMotor::Motor0, 5.0);
FEHMotor fr_motor(FEHMotor::Motor2, 5.0);
FEHMotor bl_motor(FEHMotor::Motor3, 5.0);
FEHMotor br_motor(FEHMotor::Motor1, 5.0);

AnalogInputPin cds(FEHIO::P0_4);

int theoreticalCounts(int inches) {
    int counts = (inches * COUNTS_PER_REV) / (2 * PI * WHEEL_RADIUS);
    return counts;
}

int theoreticalDegree(int degrees){
    /*
    int counts = (degrees * COUNTS_PER_REV * ROBOT_RADIUS) / (2 * PI * WHEEL_RADIUS);
    */
    float radians = degrees * PI / 180;
    int arclength = ROBOT_RADIUS * radians;
    return theoreticalCounts(arclength);
}

//Here 'move_forward' means positive movement.
/* IMPORTANT: Our coordinate system for this program is a top-down view of the course, with the starting point as the origin.
 * DDR is in positive X and lever is in positive Y. */
void move_forward(int percent, int inches)
{
    //Reset all encoder counts
    fl_encoder.ResetCounts();
    fr_encoder.ResetCounts();
    bl_encoder.ResetCounts();
    br_encoder.ResetCounts();

    int counts = theoreticalCounts(inches);
    int actualCts = 0;

    //Set motors to desired percent. Some motors have to turn backwards, so make percent negative.
    //Some motors have to turn backwards depending on the orientation, also.

    //While the average of the left or right encoders is less than theoretical counts,
    //keep running motors

    if (defaultOrientation) {
    fl_motor.SetPercent(percent);
    br_motor.SetPercent(-1 * percent);
    fr_motor.SetPercent(-1 * percent);
    bl_motor.SetPercent(percent);
    } else {
        fl_motor.SetPercent(percent);
        br_motor.SetPercent(-1 * percent - OFFSET);
        fr_motor.SetPercent(percent);
        bl_motor.SetPercent(-1 * percent - OFFSET);
    }

    while(fr_encoder.Counts() < counts || bl_encoder.Counts() < counts){
        LCD.Clear();
        LCD.Write("Moving forward ");
        LCD.Write(inches);
        LCD.WriteLine(" inches");
        LCD.Write("THEORETICAL COUNTS: ");
        LCD.WriteLine(counts);
        LCD.Write("Actual BLE Counts: ");
        LCD.WriteLine(bl_encoder.Counts());
        LCD.Write("Actual BRE Counts: ");
        LCD.WriteLine(br_encoder.Counts());
        LCD.Write("Actual FLE Counts: ");
        LCD.WriteLine(fl_encoder.Counts());
        LCD.Write("Actual FRE Counts: ");
        LCD.WriteLine(fr_encoder.Counts());

        if(fr_encoder.Counts() > bl_encoder.Counts() + 20){
            fr_motor.SetPercent(-percent - OFFSET + 10);
            br_motor.SetPercent(-percent - OFFSET + 10);
        }
        else if(fr_encoder.Counts() < bl_encoder.Counts() - 20){
            fr_motor.SetPercent(-percent - OFFSET - 10);
            br_motor.SetPercent(-percent - OFFSET - 10);
        }
        else{
            fr_motor.SetPercent(-percent - OFFSET);
            br_motor.SetPercent(-percent - OFFSET);
        }
/*
        if(fr_encoder.Counts() > bl_encoder.Counts() + 150){
            fr_motor.SetPercent(-percent - OFFSET + 10);
            br_motor.SetPercent(-percent - (OFFSET/3.0) + 10);
        }
        else if(fr_encoder.Counts() < bl_encoder.Counts() - 150){
            fr_motor.SetPercent(-percent - OFFSET - 10);
            br_motor.SetPercent(-percent - (OFFSET/3.0) - 10);
        }
        else{
            fr_motor.SetPercent(-percent - OFFSET);
            br_motor.SetPercent(-percent - (OFFSET/3.0));
        }
*/
    }

    fr_motor.Stop();
    bl_motor.Stop();
    fl_motor.Stop();
    br_motor.Stop();
}

//Same as move_forward, but percents are multiplied by -1 (reverse).
void move_backward(int percent, int inches)
{
    //Reset all encoder counts
    fl_encoder.ResetCounts();
    fr_encoder.ResetCounts();
    bl_encoder.ResetCounts();
    br_encoder.ResetCounts();

    int counts = theoreticalCounts(inches);

    //Set motors to desired percent. Some motors have to turn backwards, so make percent negative.
    //Some motors have to turn backwards depending on the orientation, also.
    if (defaultOrientation) {
    fl_motor.SetPercent(-1 * percent);
    br_motor.SetPercent(percent + OFFSET);
    fr_motor.SetPercent(percent + OFFSET / 3.0);
    bl_motor.SetPercent(-1 * percent);
    } else {
        fl_motor.SetPercent(-1 * percent);
        br_motor.SetPercent(percent);
        fr_motor.SetPercent(-1 * percent);
        bl_motor.SetPercent(percent);
    }

    //While the average of the left and right encoders is less than theoretical counts,
    //keep running motors
    while(fr_encoder.Counts() < counts || bl_encoder.Counts() < counts){
        LCD.Clear();
        LCD.Write("Moving backwards ");
        LCD.Write(inches);
        LCD.WriteLine(" inches");
        LCD.Write("THEORETICAL COUNTS: ");
        LCD.WriteLine(theoreticalCounts(24));
        LCD.Write("Actual BLE Counts: ");
        LCD.WriteLine(bl_encoder.Counts());
        LCD.Write("Actual BRE Counts: ");
        LCD.WriteLine(br_encoder.Counts());
        LCD.Write("Actual FLE Counts: ");
        LCD.WriteLine(fl_encoder.Counts());
        LCD.Write("Actual FRE Counts: ");
        LCD.WriteLine(fr_encoder.Counts());
    }

    //Turn off motors
    fr_motor.Stop();
    bl_motor.Stop();
    fl_motor.Stop();
    br_motor.Stop();
}

void turnLeft(int percent, int degrees){
    //Reset all encoder counts
    fl_encoder.ResetCounts();
    fr_encoder.ResetCounts();
    bl_encoder.ResetCounts();
    br_encoder.ResetCounts();

    int counts = theoreticalDegree(degrees);

    //Set motors to desired percent. Some motors have to turn backwards, so make percent negative.
    //Some motors have to turn backwards depending on the orientation, also.
    if (defaultOrientation) {
    fl_motor.SetPercent(-percent);
    br_motor.SetPercent(-percent);
    fr_motor.SetPercent(-percent);
    bl_motor.SetPercent(-percent);
    } else {
        fl_motor.SetPercent(percent);
        br_motor.SetPercent(percent);
        fr_motor.SetPercent(percent);
        bl_motor.SetPercent(percent);
    }

    //While the average of the left and right encoders is less than theoretical counts,
    //keep running motors
    while(fr_encoder.Counts() < counts || bl_encoder.Counts() < counts){
        LCD.Clear();
        LCD.Write("Turning left ");
        LCD.Write(degrees);
        LCD.WriteLine(" degrees");
        LCD.Write("THEORETICAL COUNTS: ");
        LCD.WriteLine(theoreticalCounts(degrees));
        LCD.Write("Actual BLE Counts: ");
        LCD.WriteLine(bl_encoder.Counts());
        LCD.Write("Actual BRE Counts: ");
        LCD.WriteLine(br_encoder.Counts());
        LCD.Write("Actual FLE Counts: ");
        LCD.WriteLine(fl_encoder.Counts());
        LCD.Write("Actual FRE Counts: ");
        LCD.WriteLine(fr_encoder.Counts());
    }

    //Turn off motors
    fr_motor.Stop();
    bl_motor.Stop();
    fl_motor.Stop();
    br_motor.Stop();
}

void turnRight(int percent, int degrees){
    //Reset all encoder counts
    fl_encoder.ResetCounts();
    fr_encoder.ResetCounts();
    bl_encoder.ResetCounts();
    br_encoder.ResetCounts();

    int counts = theoreticalDegree(degrees);

    //Set motors to desired percent. Some motors have to turn backwards, so make percent negative.
    //Some motors have to turn backwards depending on the orientation, also.
    if (defaultOrientation) {
    fl_motor.SetPercent(percent);
    br_motor.SetPercent(percent);
    fr_motor.SetPercent(percent);
    bl_motor.SetPercent(percent);
    } else {
        fl_motor.SetPercent(percent);
        br_motor.SetPercent(percent);
        fr_motor.SetPercent(percent);
        bl_motor.SetPercent(percent);
    }

    //While the average of the left and right encoders is less than theoretical counts,
    //keep running motors
    while(fr_encoder.Counts() < counts || bl_encoder.Counts() < counts){
        LCD.Clear();
        LCD.Write("Turning right ");
        LCD.Write(degrees);
        LCD.WriteLine(" degrees");
        LCD.Write("THEORETICAL COUNTS: ");
        LCD.WriteLine(counts);
        LCD.Write("Actual BLE Counts: ");
        LCD.WriteLine(bl_encoder.Counts());
        LCD.Write("Actual BRE Counts: ");
        LCD.WriteLine(br_encoder.Counts());
        LCD.Write("Actual FLE Counts: ");
        LCD.WriteLine(fl_encoder.Counts());
        LCD.Write("Actual FRE Counts: ");
        LCD.WriteLine(fr_encoder.Counts());
    }

    //Turn off motors
    fr_motor.Stop();
    bl_motor.Stop();
    fl_motor.Stop();
    br_motor.Stop();
}

/* TODO: Make sure to find out what angle the servo turns between orientations. */
//Sets robot's wheel orientation so it can move in the direction of the Y-axis
void setVerticalOrientation() {
    main_servo.SetDegree(0.);
    defaultOrientation = true;
}

//Sets robot's wheel orientation so it can move in the direction of the X-axis
void setHorizontalOrientation() {
    main_servo.SetDegree(90.);
    defaultOrientation = false;
}

//Controls the lever servo's angle.
void pushLever(float degree) {
    lever_servo.SetDegree(degree);
}

/* TODO: Find correct number of counts and determine ideal motor speed. */
//Start function, which will move the robot from the start position and into a correct orientation.
void start() {
    move_forward(20, 50);
    Sleep(250);

    fl_encoder.ResetCounts();
    fr_encoder.ResetCounts();
    bl_encoder.ResetCounts();
    br_encoder.ResetCounts();

    //Turns the robot to the left about the general center point of the robot.
    fr_motor.SetPercent(20);
    bl_motor.SetPercent(-20);
    fl_motor.SetPercent(-20);
    br_motor.SetPercent(20);

    while((fl_encoder.Counts() + bl_encoder.Counts()) / 2. < 140 ||
          (fr_encoder.Counts() + br_encoder.Counts()) / 2. < 140 );

    fr_motor.Stop();
    bl_motor.Stop();
    fl_motor.Stop();
    br_motor.Stop();

}

int main()
{
    // min for main servo: 720
    // max for main servo: 2475
    // min for lever servo: 514
    // max for lever servo: 2430
    int percent = 50;
    //Initialize the screen
    LCD.Clear(BLACK);
    LCD.SetFontColor(WHITE);

    //Initialize lever servo
    lever_servo.SetMin(720);
    lever_servo.SetMax(2430);

    lever_servo.SetDegree(120.0);

    while(cds.Value() > redThresh) {
        LCD.Clear();
        LCD.WriteLine("Looking for Red Light...");
        LCD.WriteLine(cds.Value());
    }
    Sleep(50);
    move_forward(50, 4);
    Sleep(100);
    turnRight(50, 45);
    Sleep(100);
    move_forward(50, 20);
    Sleep(100);
    move_backward(50, 2);
    Sleep(100);
    turnLeft(50, 105);
    Sleep(100);
    move_forward(50, 45);
    Sleep(100);
    turnLeft(50, 90);
    Sleep(100);
    move_backward(50, 5);
    Sleep(100);
    move_forward(50, 15);
    Sleep(100);
    lever_servo.SetDegree(40.0);
    Sleep(2000);
    lever_servo.SetDegree(120.0);
    Sleep(1000);

    /*
    setHorizontalOrientation();
    Sleep(500);
    //Move to the right a bit. Line up with the lever.
    move_forward(50, 800);
    Sleep(500);
    */

    /*setVerticalOrientation();
    Sleep(500);
    //Move up toward the lever.
    move_forward(20, 1000);
    Sleep(500);
    pushLever(110.);
    Sleep(500);
    */
}
