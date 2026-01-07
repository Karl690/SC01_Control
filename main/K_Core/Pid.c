// pid.c
#include "pid.h"

void pid_init(PID *pid, float Kp, float Ki, float Kd, float dt, float integral_max, float integral_min) {
	pid->Kp = Kp;
	pid->Ki = Ki;
	pid->Kd = Kd;
	pid->dt = dt;
	pid->setpoint = 0.0f;
	pid->integral = 0.0f;
	pid->prev_error = 0.0f;
	pid->integral_max = integral_max;
	pid->integral_min = integral_min;
}

float pid_update(PID *pid, float input) {
	float error = pid->setpoint - input;
    
	// Accumulate integral
	pid->integral += error * pid->dt;
    
	// Anti-windup: Clamp integral if limits are set
	if (pid->integral_max != 0.0f || pid->integral_min != 0.0f) {
		if (pid->integral > pid->integral_max) {
			pid->integral = pid->integral_max;
		}
		else if (pid->integral < pid->integral_min) {
			pid->integral = pid->integral_min;
		}
	}
    
	// Calculate derivative
	float derivative = (error - pid->prev_error) / pid->dt;
    
	// Compute output
	float output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);
    
	// Update previous error
	pid->prev_error = error;
    
	return output;
}

void pid_set_setpoint(PID *pid, float setpoint) {
	pid->setpoint = setpoint;
}