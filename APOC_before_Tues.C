
/*********************************************************/
/******************* FREE-TIME SYSTEM ********************/
/*********************************************************/
/******************** EME154 UCDAVIS *********************/
/*********************************************************/
# include <8052.h>
# include <stdio.h>
# include <stdlib.h>
# include <MORPH.h>
typedef int bool;
#define true 1
#define false 0     //define boolean *NEW


//NOTES: ERT error flag?(when dial_pos input is out of bounds)
//need to make sure that we can exit from auto, man, and msd mode with keyboard input of '5'
//changed everything to unsigned int, make sure it works

/**** FUNCTION PROTOTYPES ********************************/
void OCSfunction();
void MOSfunction(char selection);
void ACSfunction();
void MSDfunction(char selection);
void SACSfunction(char selection); //semi automatic

/*********************************************************/
/***** START OF FTS **************************************/
/*********************************************************/

/***** REMARKS SECTION ***********************************
 FTS - Free time System
 INZ - Initialization
 DIG - Diagnostics
 ERH - Error Treatment Supervisor
 MSS - Machine Status Scan Supervisor
 MCS - Mode Control Supervisor
 ACS - Automatic Control Supervisor
 MOS - Manual Operation Supervisor
 PGS - Programming Supervisor
 MSD - Machine Setup Data Supervisor
 OCS - Output Control Supervisor
 ***** END OF REMARKS SECTION *****************************/


/*********************************************************/
/***** FTS PROGRAM SECTION *******************************/
/*********************************************************/

/***** START OF INZ **************************************/
unsigned char OMD, ERR1, FFRA, samplingTime, dialPositions;
unsigned int feedRate; //set speed in RPM
unsigned int encoderResolution; //set encoder resolution

__data __at (0x52) unsigned int speed;
__data __at (0x56) unsigned long distance;
__data __at (0x23) unsigned int counter;


//motion register:: XGO(0th bit), INPOS(1st bit), direction(4th bit)
__data __at (0x21) unsigned char motionRegister;

char string[5];
char *machineMessage, *menuTitle, *action1, *action2, *action3;

unsigned int input1, input2, input3, distance2, distance3;
unsigned int dial_pos;
char str[5];

bool isSetup;   //*NEW

/***** Initilization function ****************************/
void INZfunction(){
    printMode = 0; //Output is routed to the MPS Console
    OMD=0;// REM Operation Mode is Idle
    ERR1=0;//REM Level 1 Error Flag of System
    FFRA=1;//REM SET First Run Flag
    P1 = 0x00;//Solenoid port initialization
	counter = 0; //timer counter initialization 5_16_2014
    clrUDCounter();
    
    //initialize added variables
    input1 = 0;
    input2 = 0;
    input3 = 0;
    distance2 = 0;
    distance3 = 0;
    dial_pos = 0;

    isSetup = false; //*NEW
    
    motionRegister = 0x00; //CW direction, !inPOS, !XGO
    encoderResolution = 2048*4;
    dialPositions = 40;
    samplingTime = 10; //sampling time is 10ms
    feedRate = 25;//50; //set motor speed in RPM
    speed = (feedRate*samplingTime*(float)encoderResolution)/60000+0.5;//set speed
    machineMessage = " ";
    
    //initialize Real Time System
    __asm
    lcall 0x9F00
    __endasm;
    
}
/***** END OF INZ ****************************************/

/***** START of MESSAGE AND CONSTANTS DEFINITION *********/
void printHeaderAndMenu(){
    clrPC();
    printf("EME-154 Mechatronics\n");
    printf("Free Time System\n");
    printf("Brian Mar/Max Toback\n\n");
    printf("****************************************\n");
    printf("%s\n", menuTitle);
    printf("****************************************\n\n");
    printf("1. %s\n", action1);
    printf("2. %s\n", action2);
    printf("3. %s\n", action3);
    printf("5. Exit\n\n\n\n");
}

void setIdleMenu(){
    menuTitle = " OPERATION MENU";
    action1 = "Manual Operation";
    action2 = "Automatic Operation";
    action3 = "Machine Data Set-Up";
}

void setMsdMenu()
{
    menuTitle = "MSD OPERATION Menu";
    action1 = "Machine Data Set-Up";
    action2 = "";
    action3 = "";
}

void setManualMenu(){
    menuTitle = " MANUAL OPERATION MENU";
    action1 = "Move One Tick CCW";
    action2 = "Move One Tick CW";
    action3 = " ";
}

void setAutomaticMenu(){
    menuTitle = " AUTOMATIC OPERATION MENU";
    action1 = "";
    action2 = "";
    action3 = "";
}

void setSemiAutomaticMenu(){
    menuTitle = " Semi-AUTOMATIC OPERATION MENU"
    action1 = "Set Clockwise Direction";
    action2 = "Set CounterClockwise Direction";
    action3 = "";
}

void moveServo(char ticks, char direction){
    distance = (float)ticks*(float)(encoderResolution / dialPositions);
    //the 0x01 is the XGO bit and 0x10 is the directional bit
    //direction 0 & 1 equate CW & CCW, respectively
    motionRegister = 0x01+0x10*direction;
}

int getABSposition(){
    __data __at (0x60) unsigned long position;
    return position;
}
/***** END of MESSAGE AND CONSTANTS DEFINITION ***********/

/***** START OF DIG **************************************/
int diagnostics(){
    if(ERR1) {
        return 0;//for error
    }else{
        return 1;//successful diagnostics
    }
}
/***** END OF DIG ****************************************/

/***** ERH ***********************************************/
void ERHfunction(){
    //take actions here to treat errors detected
    if ((dial_pos > 39) || (dial_pos < 0)){
        machineMessage = "dial position must be 0 - 39";
        ERR1 = 1; //*NEW
    }
    //WARNING: might need to set ERT flag here *NEW
    
}
/***** END OF ERH ****************************************/


/***** START OF MSS **************************************/
void MSSfunction(){
    char selection;
    selection = key();
    
    if (FFRA) OCSfunction();//skip this for the first run
    //Scan Keyboard for input
    switch (selection){
            
            case '1':
            if (OMD == 1)
                MOSfunction(selection);
//            if (OMD == 2)
//                //do nothing
            else if (OMD == 3)
                MSDfunction(selection);
            else if (OMD == 4)
                SACSfunction(selection);
            else if (OMD == 0){
                if (isSetup){ //*NEW isSetup
                    OMD = 1;
                    setManualMenu();
                    machineMessage = "Manual Mode Accepted";
                    printHeaderAndMenu();
                }
                else
                    machineMessage = "Machine Setup Data not ready"; //*NEW make sure whatever happens after this message is appropriate
                //should still be in idle mode after this
            }
            break;
            
            
            case '2':
            if (OMD == 1)
                MOSfunction(selection);
//            if (OMD == 2)
//                //do nothing
            else if (OMD == 3)
                MSDfunction(selection);
            else if (OMD == 4)
                SACSfunction(selection);
            else if (OMD == 0){
                if (isSetup) { //*NEW isSetup
                    OMD = 2;
                    setAutomaticMenu();
                    machineMessage = "Automatic Mode Accepted";
                    printHeaderAndMenu();
                }
                else
                    machineMessage = "Machine Setup Data not ready"; //*NEW make sure whatever happens after this message is appropriate
            }
            break;
            
            
            case '3':
            if (OMD == 1)
                MOSfunction(selection);
//            if (OMD == 2)
//                //do nothing
            else if (OMD == 3)
                MSDfunction(selection);
            else if (OMD == 4)
                SACSfunction(selection);
            else if (OMD == 0){
                OMD = 3;
                setAutomaticMenu();
                printHeaderAndMenu();
            }
            break;
            
            
            case '4':
            if (OMD==1)
                MOSfunction(selection);
//            if (OMD == 2)
//                //do nothing
            else if (OMD==3)
                MSDfunction(selection);
            else if (OMD == 4)
                SACSfunction(selection);
            else if (OMD == 0){
                if (isSetup) { //*NEW isSetup
                    OMD = 4;
                    setAutomaticMenu();
                    machineMessage = "Semi-Automatic Mode Accepted";
                    printHeaderAndMenu();
                }
                else
                machineMessage = "Machine Setup Data not ready"; //*NEW make sure whatever happens after this message is appropriate
            }
            break;
            
            
            case '5': //this key will...
            //in idle mode and 5 is pressed
            if (OMD == 0){
                OMD = 5;
                machineMessage = "Exit Command Accepted";
            } else {
                setIdleMenu();
                printHeaderAndMenu();
                OMD = 0;
            }
            break;
            
    }
}
/***** END OF MSS ****************************************/

/***** START OF MCS **************************************/
void MCSfunction(){
    if (OMD==1) MOSfunction(0);
    if (OMD==2) ACSfunction();
    if (OMD==3) MSDfunction(1);
    if (OMD==4) SACSfunction(1);
    
}
/***** END OF MCS ****************************************/

/***** START OF MOS **************************************/
void MOSfunction(char selection){
    if (selection){
        switch (selection){
                case '1': //this key will...
                moveServo(1,1); //move 1 tick CCW
                break;
                case '2': //this key will...
                moveServo(1,0); //zero is CW
                break;
        }
    }
}
/***** END OF MOS ****************************************/


/***** START OF ACS **************************************/
void ACSfunction(){
    //prompt before starting ACS
	printf("Press Enter to Begin Automatic Mode\n");
	getchar(); //prompts user to press enter
	counter = 0;
    
    //might need to declare str for each number
    printf("Enter the first number >");
    gets(str);
    input1 = atoi(str);
    while ((input1 > 39) || (input1 < 0)){
        printf("input must be 0 - 39, Enter input again> ");
        gets(str);
        input1 = atoi(str);
	}
    
    printf("Enter the second number >");
    gets(str);
    input2 = atoi(str);
    while ((input2 > 39) || (input2 < 0)){
        printf("input must be 0 - 39, Enter input again> ");
        gets(str);
        input2 = atoi(str);
	}
	
    printf("Enter the third number");
    gets(str);
    input3 = atoi(str);
    while ((input3 > 39) || (input3 < 0)){
        printf("input must be 0 - 39, Enter input again> ");
        gets(str);
        input3 = atoi(str);
	}
	
    //calculate distance for first number
	if (input1 > dial_pos)
    distance1 = 80 + (input1 - dial_pos);
	else if (input1 < dial_pos)
    distance1 = 80 + (40 - dial_pos + input1);
    
    //calculate distance for second number
    if (input1 > input2)
    distance2 = input1 - input2 + 40;
    else if (input1 < input2)
    distance2 = (40 - input2) + input1 + 40;
    
    //calculate distance for third number
    if (input2 > input3)
    distance3 = 40 - (input2 - input3);
    else if (input2 < input3)
    distance3 = input3 - input2;
    
    
    printf("Moving to First Number: %u \n", input1);
    moveServo(distance1, 1);
    
    while (!(motionRegister & 02));
	printf("Moving to Second Number: %u \n", input2);
	moveServo(distance2, 0);
	
    while (!(motionRegister & 02));
	printf("Moving to Third Number: %u \n", input3);
    moveServo(distance3, 1);
//	while (!(motionRegister & 02)); //don't need this? *NEW
    
    //pop the lock with the solenoid connected to port 1
    printf("Press Enter to Release Solenoid\n");
    P1 = 0xFF;
	printf("Time in ms: %u", 10*counter);
    gets(string);
    P1 = 0x00;
    
    //no longer in automatic mode
    setIdleMenu();
    printHeaderAndMenu();
    OMD = 0;
}
/***** END OF ACS ****************************************/


/***** START OF SACS **************************************/
void SACSfunction(char selection){
    //prompt before starting ACS
	printf("Press Enter to Begin Semi-Automatic Mode\n");
	getchar(); //prompts user to press enter

    if (selection){
        switch (selection){
                case '1': //this key will...
                    printf("How many ticks clockwise?")
                    gets(str);
                    input1 = atoi(str);
                    moveServo(input1, 0);
                    break;
                case '2': //this key will...
                    printf("How many ticks counterclockwise?")
                    gets(str);
                    input1 = atoi(str);
                    moveServo(input1, 1);
                    break;
        }
}
/***** END OF SACS ****************************************/


/***** START OF MSD **************************************/
void MSDfunction(char selection){
    printf("Press Enter to Begin Machine Setup Data Mode\n");
    getchar();
    if (selection){
        switch (selection){
                case '1': //this key will...
                machineMessage = "Machine Set Up Mode Activated";
                printf("Enter the initial dial position(1-39)> ");
                gets(str);
                dial_pos = atoi(str); //dial_pos is declared globally
                printf("dial position set to %u", dial_pos);
                isSetup = true;
                
                break;
                case '2': //this key will...
                printf("hi, this does nothing");
                break;
        }
    }
    
}
/***** END OF MSD ****************************************/

/***** Function gets mode name from OMD ******************/
char *getCurrentMode(){
    switch (OMD){
            case 1:
            return "Manual";
            case 2:
            return "Automatic"; //*NEW
            case 3:
            return "MSD";   //*NEW
            case 4:
            return "Semi-Automatic";
            case 5:
            return "System Turned Off";
        default:
            return "Idle Mode";
    }
}

//*NEW
char *getStatus(){
    if (isSetup)
        return "Ready";
    else
        return "Not Ready";
}

char *getServoStatus(){
    if (motionRegister & 02){
        return "Stopped ";
    } else{
        return "Moving... ";
    }
}

/***** START OF OCS **************************************/
void OCSfunction(){
    if(FFRA) {
        FFRA = 0;//Clear first run flag
        setIdleMenu();
        printHeaderAndMenu();
    }else{
        setCur(0,13);
        printf("%s \n", machineMessage);
        printf("Servo Status: %s     %u         \n", getServoStatus(), getABSposition());
        printf("Current Mode: %s\n\n", getCurrentMode());
        //*NEW
        printf("Status: %s\n", getStatus());
    }
}
/***** END OF OCS ****************************************/


/***** START OF MAIN *************************************/
void main(){
    
    INZfunction();
    
    while(OMD != 5 && !microButtons){
        //run diagnostics
        if (diagnostics()){
            //if diagnostics succeed
            MSSfunction();
            MCSfunction();
            OCSfunction();
        }else{
            //go to ERH if diagnostics fail
            ERHfunction();
            OCSfunction();
        }
    }
}
/***** END OF MAIN ***************************************/
