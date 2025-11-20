#include "sequencer.h"
#include "K_Core/execution/cmdprocessor.h"
#include "K_Core/communication/parser.h"
#include "K_Core/execution/M_Codes.h"

int CurrentCommandPointer=1;
int NextCommandPointer=1;
int waitingForHotbedTemp=0;
int waitingForExtruderTemp=0;
int abortInProgress=0;
uint32_t Tim1reload=3000;
uint32_t Acceleration=1;

GMCommandStructure *ExecutionPtr;
GMCommandStructure cmdQue[SIZE_OF_COMMAND_QUEUE];
GMCommandStructure *ExecutionPtrToDisplay = &cmdQue[1];

void Sequencer()
  {//this will execute the command stored in the que, if there are any waiting to get executed

	  if(CommandsInUrgentQue)
	  {//urgent commands are allowed to pass thru even though we are waiting for normal events to finish
		 // processHotCommand();//fake for now
		  CommandsInUrgentQue--;
		  return;						//no commands to proces, so return
	  }
	//   if (G4DwellTimer>0){G4DwellTimer--; return;} //do nothing during a dwell time period
//      if (waitingForHotbedTemp) return;//heating up the bed, so wait please
//      if (waitingForExtruderTemp) return;//heating up the head, so wait
//      if (WaitForOperatorResonse) return;   //waiting for the operator to puse the response button
//      if (WaitForHeadResonse) return;      //used when the head is doing something, like homing the motor.....x1
//      if (waitForExternalTrigger)return;   //waiting for a condition to be met, like a certain reading on an analog pin.
      	  	  	  	  	  	  	  	  	  	  //this will be augmented later with new mcodes
      if (abortInProgress) return;					//no new commands while we are aborting motion

	  if(!CommandsInQue)return;						//no commands to proces, so return
	  //if(motionQue>15)return;					//look ahead que for motion
	  //if(!CheckMotionComplete()) return;         //wait for motor to finish
  		CommandsInQue--;							//count down the number of commands
		//return;
  		CurrentCommandPointer = NextCommandPointer;	//set the working pointer to the right spot in the execution que
  		NextCommandPointer++;						//point forward to the next command for the next time.abortInProgress
  		if (NextCommandPointer>(SIZE_OF_COMMAND_QUEUE-1))NextCommandPointer=1;//circular que so make sure you do not overrun

	  
  	  ExecutionPtr = &cmdQue[CurrentCommandPointer];//now we are pointing to the right command[] in the instruction que

  	  if(ExecutionPtr->N  < 0)
  		  {gcodeLineNumber=ExecutionPtr->N; }//if there is a N argument, reset the line counter to this value
  	  else
  	  {ExecutionPtr->N = ++gcodeLineNumber;}//otherwise, count up one line of gcode

  	  if(ExecutionPtr->M != INVALID_ARG_VALUE)
  	  {//if the M code 0 or more then try to jump to a routine
  		switch ((int) ExecutionPtr->M)                  /* select the type of calculation */
	    {

//  		  case 0 :    M_Code_M0(); 	return;//program pause
//  		  case 1 :    M_Code_M1(); 	return;//program pause
//  		  case 2 :    M_Code_M2(); 	return;//program END
//  		  case 3 :    M_Code_M3(); 	return;//Spindle ON CW
//  		  case 4 :    M_Code_M4(); 	return;//spindle on CCW
//  		  case 5 :    M_Code_M5(); 	return;//spindle OFF
//        case 6:     M_Code_M6(); 	return;//ToolChange
//  		  case 7 :    M_Code_M7(); 	return;//coolant1 /  heat control on
//  		  case 8 :    M_Code_M8(); 	return;//coolant2 / heat conrol on
//  		  case 9 :    M_Code_M9(); 	return;//all coolant/ heat off
//
//
//  		  case 17:  M_Code_M17(); 	return;//Enable stepping motors
//  		  case 18:  M_Code_M84(); 	return;//release stepping motors
//  		  case 30:  M_Code_M30(); 	return;//notify start and end of program
//  		  case 84:  M_Code_M84(); 	return;//release stepping motors
	    // case 91: M_Code_M91(); return; // set Max travel distance
	    // case 92: M_Code_M92(); return; // set axis steps per unit
	    // case 93: M_Code_M93(); return; // sets home sensor polarity 
	    // case 94: M_Code_M94(); return; // sets the default motor direction 
	    // case 95: M_Code_M95(); return; // sets stall sensor polarity
	    // case 96: M_Code_M96(); return; // sets the enable bit polarity 
	    // case 97: M_Code_M97(); return; // sets the step bit polarity 
	    // case 98: M_Code_M98(); return; // sets limit1 sensor polarity 
	    // case 99: M_Code_M99(); return; // sets limit2 sensor polarity 
//  		  case 101: M_Code_M101();  return;//turn extruder on
//  		  case 102: M_Code_M102();  return;//turn extruder on, reverse
//  		  case 103:	M_Code_M103();	return;//turn extruder off
 		case 104:	M_Code_M104();	return;//set desired temperature to Sxxx.x
//  		  case 105:	M_Code_M105();	return;//set desired temperature to Sxxx.x immediate
//  		case 106:	M_Code_M106();	return;//turn fan on
//  		  case 107:	M_Code_M107();	return;//turn fan off
//  		  case 108:	M_Code_M108();	return;//set extruder speed to Sxx.x  rpm
//  		  case 109: M_Code_M109();	return;//wait for extruder to reach temp
//  		 // case 115: M_Code_M115();	return;//tell them what firmware revision level we are using
//          case 140: M_Code_M140();  return;//Set bed temperature
//          case 190: M_Code_M140();  return;//Wait for bed temperature
        // case 203:M_Code_M203();   return;//set rapid feed rate
	    // case 204:M_Code_M204(); return; // Sets the no ramp speed (UNITS/MIN) 
	    // case 205:M_Code_M205(); return; // Sets the homing speed (UNITS/MIN) 
	    // case 206:M_Code_M206(); return; // Sets the homing routine hysteresis (inch/mm/deg) 
	    // case 208:M_Code_M208(); return; // Sets the acceleration constant
        // case 210:M_Code_M210();   return;// Sets the maximum axis speed (UNITS/MIN) (uses X, Y, Z, A, B, C)
	    // case 216:M_Code_M216(); return; // Sets the home position in units 
	    // case 217:M_Code_M217(); return; // sets the max deceleration rate (for abort) (mm/sec/sec)
	        
//  		  case 220: M_Code_M220();	return;//Turn off AUX
//        case 221: M_Code_M221();	return;//set extrude factor override percentage
//  		  case 222: M_Code_M222();	return;//M122 S500 Set speed of fast XY moves  //500fast - 2000slow
//  		  case 223: M_Code_M223();	return;//M123 S500 Set speed of fast Z moves  //500fast - 2000slow
//  		  case 224: M_Code_M224();	return;//Enable extruder motor during fast move
//  		  case 225: M_Code_M225();	return;//Disable extruder motor during fast move
//  		  case 226: M_Code_M226();	return;//Issue a Pause command from G-Code
//  		  case 227: M_Code_M227();	return;//Enable Extruder Reverse S=Reverse time in ms P=Prime time in ms
//  		  case 228: M_Code_M228();	return;//Disable Extruder Reverse
//        case 223:M_Code_M223(); return; // Sets the re-homing speed (UNITS/MIN)
//	    case 233:M_Code_M233(); return; // set homing pop-off distance (inch/mm/deg)
		
//	    case 304:M_Code_M304(); return;//upload the Acceleration table -> Phoenix -> Repetrel
	    //case 305:M_Code_M305(); return;//download the Acceleration table -> Repetrel -> Phoneix
//  		case 613: M_Code_M613();return;// set valves
 		// case 623: M_Code_M623();return;// set valves	    
//	    case 660: M_Code_M660(); return;//set tool diameter and length (uses H, D, Z)


//  		  case 682:  M_Code_M682(); return;//issue z axis calibration request, this is so we can do a seek and loop
//  	      case 701 : M_Code_M701(); return; // set auto status update rate and page selection (uses S, P, E)
//  	      case 702 : M_Code_M702(); return; // turn fan on based on temperature (ie, motor cooling) (uses S, P)
//  	      case 703 : M_Code_M703(); return; // add device to Group (uses S, P)
//  	      case 704 : M_Code_M704(); return; // remove device to Group (uses S, P)
//  	      case 705 : M_Code_M705(); return; // reset HH (uses P)
//  	      case 706 : M_Code_M706(); return; // sync HH (uses P)
//  	      case 707 : M_Code_M707(); return; // STOP HH (uses P)
//  	      case 708 : M_Code_M708(); return; // pause HH (uses P)
//  	      case 709 : M_Code_M709();	return; // resume device (from pause or stop) (uses T)
//  	      case 710 : M_Code_M710(); return; // enable/disable RTD1 (uses S, P)
//  	      case 711 : M_Code_M711(); return; // enable/disable RTD2 (uses S, P)
//  	      case 712 : M_Code_M712(); return; // enable/disable RTD3 (uses S, P)
//  	      case 713 : M_Code_M713(); return; // set default P value for missing argP
//  	      case 714 : M_Code_M714(); return; // update devicePosition alias table
//  	      case 715 : M_Code_M715(); return; // set LED display selection
//  	      case 716 : M_Code_M716(); return; // register HH (uses P)
//  	      case 717 : M_Code_M717(); return; // force send of outgoing mail
//  	      case 718 : M_Code_M718(); return; // set auto status update rate and page selection for Hotbed (uses S, E)
//  	      case 719 : M_Code_M719(); return; // set reporting rate and data for host traffic (uses S, E)
//  	      case 720 : M_Code_M720();	return; // direct MAIN extrusion control (uses P, F, E, S)
//        case 721 : M_Code_M721();	return; // direct UNPRIME extrusion control (uses P, F, E, S)
//        case 722 : M_Code_M722();	return; // direct PRIME extrusion control (uses P, F, E, S)
//        case 723 : M_Code_M723();	return; // direct MANUAL extrusion control (uses P, F, E, S)
//        case 724 : M_Code_M724();	return; // direct DWELL (no stepping) extrusion control (uses P, F, E, S)
//	      case 725 : M_Code_M725(); return; // set the Karl Factors for controlling the heater switch (uses T, P, E, S)
//	      case 726 : M_Code_M726(); return; // set the Karl Factors for controlling the fan switch (uses T, P, E, S)
//	      case 727 : M_Code_M727(); return; // set LED override values and mask (uses T, S, P)
//	      case 728 : M_Code_M728(); return; // set motor current boost control (uses T, S)
//	      case 729 : M_Code_M729(); return; // set motor microsteps control (uses T, S)
//	      case 730 : M_Code_M730(); return; // set not to exceed temp for motor (uses T, S)
//	      case 731 : M_Code_M731(); return; // set not to exceed temp for heater (uses T, S)
//	      case 732 : M_Code_M732(); return; // set maximum step rate for motor (microsteps/sec) (uses T, F)
//	      case 733 : M_Code_M733(); return; // set maximum allowable RTD temperature delta (uses T, S)
//	      case 734 : M_Code_M734(); return; // set HH error reporting rate for redundant error codes (uses T, F)
//	      case 735 : M_Code_M735(); return; // fill the incoming page data buffer with S (uses S)
//	      case 736 : M_Code_M736(); return; // fill the outgoing page data buffer with S (uses S)
//	      case 737 : M_Code_M737(); return; // erase flash page in selected (physical) device (uses T, I, P)
//	      case 738 : M_Code_M738(); return; // transfer data page from (physical) device to incoming buffer (uses T, I, P, S)
//	      case 739 : M_Code_M739(); return; // transfer data page from incoming to outgoing buffer
//	      case 740 : M_Code_M740(); return; // transfer data page from outgoing buffer to (physical) device (uses T, I, P, S)
//	      //case 741 : M_Code_M741(); return; // transfer incoming page related data from the device to the to host (uses S)
//		  case 742 : M_Code_M742(); return; // transfer data page in ASCII text from host to outgoing buffer (uses S, P, comment)
//	      case 743 : M_Code_M743(); return; // transfer generally (read only) device info from inbox to host (uses T, S)
//	      case 744 : M_Code_M744(); return; // transfer alias list from device to inbox (uses T)
//  	      case 745 : M_Code_M745(); return; // force HH to invert polarity of direction pin (uses P)
//  	      //case 746 : M_Code_M746(); return;	// start the bootloader for the selected physical device (uses T)
//  	      case 747 : M_Code_M747(); return;	// prepare device for download (uses P)
//  	      case 748 : M_Code_M748(); return;	// process next line of intel hex format bootloader data (uses P, comment)
//  	      //case 749 : M_Code_M749(); return;	// exit the device bootloader
//  	      case 750 : M_Code_M750(); return;	// unlock flash for erase/write access for the selected physical device (uses T)
//  	      case 751 : M_Code_M751(); return;	// lock flash to prevent erase/write access for the selected physical device (uses T)
//  	      //case 752 : M_Code_M752(); return;	// write hardware type to flash (option bytes) using device bootloader (uses S)
//  	      //case 753 : M_Code_M753(); return;	// write hardware revision to flash (option bytes) using device bootloader (uses  S)
//  	      //case 754 : M_Code_M754(); return;	// write hardware key to flash (option bytes) using device bootloader (uses S)
//  	      case 755 : M_Code_M755(); return;	// set extruder heater pwm (uses T, S)
//  	      case 756 : M_Code_M756(); return;	// set layer height (mm) (uses S)
//  	      case 757 : M_Code_M757(); return;	// set layer/path weight (mm) (uses S)
//  	      case 758 : M_Code_M758(); return;	// set extrusion step to volume conversion (steps per nL) (uses T, S)
//  	      case 759 : M_Code_M759(); return;	// enable temperature calibration (uses T, S)
//  	      case 760 : M_Code_M760();	return; // disable temperature calibration
		//   case 761 : M_Code_M761();	return; // transfer system info in ASCII text from main board to host (uses S, P)
  	    //   case 762 : M_Code_M762();	return; // transfer system info in ASCII text from host to main board (uses S, P, comment)
//  	      case 763 : M_Code_M763(); return;	// clear error on selected device(s) (uses T)
//  	      case 766 : M_Code_M766(); return;	// start the system bootloader process
//  	      //case 767 : M_Code_M767(); return;	// prepare system  for download (uses P)
//  	      //case 768 : M_Code_M768(); return;	// process next line of intel hex format of system bootloader data (uses P, comment) <-- can't go in queue because of needing comment
//  	      //case 769 : M_Code_M769(); return;	// exit the system bootloader
//  	      //case 770 : M_Code_M770(); return;	// leave system bootloader and jump to application main()
  	    // case 777 : M_Code_M777(); return;	// testing CAN message between Repetrel, Pheonix and HEAD.
//	    case 780 : M_Code_M780(); return;	// enable/disable auto XYZABC position reporting
//  	      case 790 : M_Code_M790(); return;	//layer increment, non move command, just has printer echo when it is ready to make this move
//  	      case 796 : M_Code_M796(); return;	// DEBUG: kill return OK's bakc to repetier for testing flow control
//          case 797 : M_Code_M797(); return;	// enable/disable debug reporting strings ">GB:" (uses S)
//          case 798 : M_Code_M798(); return; // dump strings to host (debug MCODE) -- warning, must reset after using this MCODE (uses T)
//	      case 799 : M_Code_M799(); return; // get PLL and Clock status for 407
	    //   case 799 : M_Code_M799(); return; // this is a test for fault .
		    
	    // case 800:	M_Code_M800(); return;
	    // case 801:	M_Code_M801(); return;
	    // case 802:	M_Code_M802(); return;
	    // case 803:	M_Code_M803(); return;
	    // case 804:	M_Code_M804(); return;
	    // case 805:	M_Code_M805(); return;
	    // case 806:	M_Code_M806(); return;
	    // case 807:	M_Code_M807(); return;
	    // case 808:	M_Code_M808(); return;
	    // case 809:	M_Code_M809(); return;//upload process variables
  	    //   case 999 : M_Code_M999(); return; // this is a test for fault .
  		      }
//  		  sprintf(_errorStr, "Unsupported MCode M%d", (int) ExecutionPtr->M);
//  		  sendError(_errorStr);

  		  return;
  	  }

  	  if(ExecutionPtr->G != INVALID_ARG_VALUE)
  	  {
  		 // jumpVector=(int) ExecutionPtr->G;
  		  switch ((int) ExecutionPtr->G)                  /* select the type of calculation */
  		      {
//  		  case 0:	G_Code_G0();	return;//move rapid
//  		  case 1:	G_Code_G1();	return;//move at feed rate
//  		  case 2:	G_Code_G2();	return;//move arc cw
//  		  case 3:	G_Code_G3();	return;//move arc ccw
  			//   case 4:	G_Code_G4();	return;//Wait in ms
  		    //   case 6:	G_Code_G6();	return;//button event
//  		  case 20:	G_Code_G20();	return;//inch dimensions
//  		  case 21:	G_Code_G21();	return;//mm dimension
//  		  case 28:	G_Code_G28();	return;//home axis, responds to individual axis if args are passed, X0Y0 homes only x and y not Z
//  		  case 29:	G_Code_G29();	return;//special exercise move
 // 		  case 90:	G_Code_G90();	return;//absolute programming coordinates
 //		  case 91:	G_Code_G91();	return;//incremental programming
//  		  case 92:	G_Code_G92();	return;//not sure???
  	 	  default :
  		  		break;
  		      }
  		  	  	  	  //anything not in the table gets ignored and the
  		  	  	  	  //next command is asked for

//  			sprintf(_errorStr, "Unsupported GCode G%d", (int) ExecutionPtr->G);
//  			sendError(_errorStr);
  		  return;
  	  }
  }

