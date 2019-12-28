
#ifndef PID_H
#define PID_H

#include <stdbool.h>
#include <stdint.h>

typedef float float32_t;

/**
 * @brief Instance structure for the floating-point PID Control.
 */
typedef struct
{
	float32_t A0;          /**< The derived gain, A0 = Kp + Ki + Kd . */
	float32_t A1;          /**< The derived gain, A1 = -Kp - 2Kd. */
	float32_t A2;          /**< The derived gain, A2 = Kd . */
	float32_t state[3];    /**< The state array of length 3. */
	float32_t Kp;          /**< The proportional gain. */
	float32_t Ki;          /**< The integral gain. */
	float32_t Kd;          /**< The derivative gain. */
} s_pid_instance_f32;

typedef struct
{
	int use_limits;
	float32_t lim_low;
	float32_t lim_high;
} s_pid_config_f32;

/*-------------------------------------------------------------*/
/*		Function prototypes				*/
/*-------------------------------------------------------------*/
#ifdef	__cplusplus
extern "C" {
#endif

void pid_init_f32(s_pid_instance_f32 * S, int32_t resetStateFlag);

/**
* @brief  Reset function for the floating-point PID Control.
* @param[in] *S	Instance pointer of PID control data structure.
* @return none.
* \par Description:
* The function resets the state buffer to zeros.
*/
void pid_reset_f32( s_pid_instance_f32 * S);

#ifdef	__cplusplus
}
#endif


/**
 * @brief  Process function for the floating-point PID Control.
 * @param[in,out] S   is an instance of the floating-point PID Control structure
 * @param[in]     in  input sample to process
 * @return out processed output sample.
 */
static inline float32_t pid_f32(
		s_pid_instance_f32 * S,
		s_pid_config_f32 * C,
		float32_t in)
{
	float32_t out;

	/* y[n] = y[n-1] + A0 * x[n] + A1 * x[n-1] + A2 * x[n-2]  */
	out = (S->A0 * in) +
			(S->A1 * S->state[0]) + (S->A2 * S->state[1]) + (S->state[2]);

	/* Update state */
	S->state[1] = S->state[0];
	S->state[0] = in;
	S->state[2] = out;

	if (C->use_limits) {
		if (out < C->lim_low) out = C->lim_low;
		if (out > C->lim_high) out = C->lim_high;
	}

	/* return to application */
	return (out);

}


#endif
// End of Header file
