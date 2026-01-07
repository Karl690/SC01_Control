#pragma once
// pid.h
#ifndef PID_H
#define PID_H

typedef struct {
	float Kp; // Proportional gain
	float Ki; // Integral gain
	float Kd; // Derivative gain
	float setpoint; // Desired value
	float integral; // Accumulated integral
	float prev_error; // Previous error for derivative
	float dt; // Time step (sampling period in seconds)
	float integral_max; // Max limit for integral to prevent windup (set to 0 to disable)
	float integral_min; // Min limit for integral to prevent windup (set to 0 to disable)
} PID;

void pid_init(PID *pid, float Kp, float Ki, float Kd, float dt, float integral_max, float integral_min);
float pid_update(PID *pid, float input);
void pid_set_setpoint(PID *pid, float setpoint);

#endif // PID_H