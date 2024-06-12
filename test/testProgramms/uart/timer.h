#ifndef TIMER_H
#define TIMER_H

#define TIMER_START_VAL (0xFFFFUL-200UL)
#define TIMER1 0
#define TIMER2 1
#define TIMER3 2
#define TIMER4 3
#define TIMER5 4

inline void handleTimer();

// Typedef for custom timer types
typedef uint8_t timerType;

typedef struct {
	bool running;
	void (*callback)(void);
	uint32_t interval;
	uint32_t timeLeft;
} timerObject;

// Create Timer Array for timer handling
volatile timerObject timerArray[5] = {{0}};

// Timer Interrupts are thrown every 100 microsecond
ISR(TIMER1_OVF_vect){
	TCNT1 = TIMER_START_VAL;
	handleTimer();
}

// Configure registers for timer interrupt and prescaler
void timer_init() {
	// Timer1: Prescaler CLK/8 --> 0,5 mikroseconds
	TCCR1B |= (1 << CS11);
	// Set start value for timer --> timer interrupt every 100 microseconds
	TCNT1 = TIMER_START_VAL;
	// Enable Timer Overflow Interrupt
	TIMSK1 |= (1 << TOIE1);
}

inline bool isRunning(timerType timer) {
	return timerArray[timer].running;
}

inline void handleTimer(){
	// Local array for single callback handle if duplicate call to handleTimer occurs (fast interrupts)
	bool executeCallback[5] = {0};
	for(uint8_t i = 0; i<5; i++) {
		if(isRunning(i)) {
			if (timerArray[i].timeLeft <= 100) {
				timerArray[i].timeLeft = timerArray[i].interval;
				executeCallback[i] = true;
			} else {
				timerArray[i].timeLeft -= 100;
			}
		}
	}
	sei();
	for(uint8_t i = 0; i<5; i++) {
		if(executeCallback[i]) {
			timerArray[i].callback();
		}
	}
}

void declareTimer(timerType timer, uint32_t interval, void (*callback)(void)) {
	if(!isRunning(timer)) {
		timerArray[timer].interval = interval;
		timerArray[timer].timeLeft = interval;
		timerArray[timer].callback = callback;
	}
}

void startTimer(timerType timer) {
	if(!isRunning(timer)) {
		timerArray[timer].running = 1;
	}
}

void cancelTimer(timerType timer) {
	timerArray[timer].running = 0;
}

#endif