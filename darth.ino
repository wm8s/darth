////////////////////////////////////////////////////////////////////////////////
//
//  Darth Vator - Simulate a HiRise elevator
//
//  Copyright (c) 2019 - Robert Bailey [WM8S], 1002 HiRise
//                       MUBASIC user 6F or sometimes 00
//                       RSTS/E user [95,6] or sometimes [1,2]
//                       VAX user [WT121002]
//                       Wylbur user 242F [when I needed to]
//
//  This program simulates an elevator from the HiRise dorm at the
//  West Virginia Institute of Technology in Montgomery, WV.
//
//  The Ground floor held the laundry, the "kitchen" (an old beat-up
//  electric stove that we used to reheat Kroger pizzas), the ice machine,
//  and a secret magical places where I spent a lot of time with a
//  butt set, a punchdown tool, and a spool of made-to-look-old 
//  cross-connect wire.
//
//  The 1st floor held the cafeteria, open for three squares a day,
//  plus very, very late at night and on the weekends if you knew the KGM.
//
//  The 2nd floor held the arcade, mailboxes, office, and RD's apartment.
//
//  This is the world's most useless machine because these elevators are gone.
//
//  These elevators are gone because HiRise is gone.
//
//  HiRise is gone because Tech is gone.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//  VERSION LOG
//
//  yyyy-mm-dd by     description
//  ---------- ------ -----------------------------------------------------------
//  2019-07-08 WM8S   v0.0 - untested code
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//  Possible future enhancements:
//
//  Occasionally, the lamp on a floor would burn out; it would often be
//  a long time before it was fixed.
//
//  Occasionally, an elevator would stop working; it might also be a long time
//  before it would be repaired.
//
//  Occasionally, the elevator car would stop and get stuck between floors.
//
//  Occasionally (although mercifully not often), the elevator would just
//  decide that your underwear were too clean, so it would let go of the car
//  and let you fall a couple of floors under gravity. This was not too bad 
//  if you started on the 3rd floor, but if you started on the 9th or 
//  10th floor, it could get pretty exciting.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  defines
////////////////////////////////////////////////////////////////////////////////

// utilities

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

// some stuff about the Arduino

#define ARDUINO_CLOCK 16000000

// some stuff about my hardware

#define DARTH_TIMER_TIC_FREQ 10

#define DARTH_SPEEDFACTOR 4

#define DARTH_LAMP_TEST_ON          250
#define DARTH_LAMP_TEST_OFF         0
#define DARTH_LAMP_TEST_MAXSWEEP    1
#define DARTH_LAMP_TEST_BLINK_ON    500
#define DARTH_LAMP_TEST_BLINK_OFF   500
#define DARTH_LAMP_TEST_MAXBLINK    2

#define DARTH_PIN_DEMOSW    A5
#define DARTH_PIN_EXTRASW   A4

#define DARTH_PIN_EMPTYA    A0

#define DARTH_PIN_DOWNLED   5
#define DARTH_PIN_UPLED     12

#define DARTH_PIN_GFL       11
#define DARTH_PIN_1FL       10
#define DARTH_PIN_2FL       9
#define DARTH_PIN_3FL       8
#define DARTH_PIN_4FL       7
#define DARTH_PIN_5FL       6
#define DARTH_PIN_6FL       A3
#define DARTH_PIN_7FL       A2
#define DARTH_PIN_8FL       2
#define DARTH_PIN_9FL       3
#define DARTH_PIN_10FL      4

// some stuff about HiRise

// floors

// WARNING: if min floor <> 0, fix array allocation, indexing, etc.
#define HR_MIN_FLOOR      0
#define HR_MAX_FLOOR      10
#define HR_NUM_FLOORS     11
#define HR_INIT_FLOOR     2

// movement delay times

#define HR_MIN_BOARD_TIME     1000
#define HR_MAX_BOARD_TIME     3000
#define HR_MIN_UNBOARD_TIME   1000
#define HR_MAX_UNBOARD_TIME   3000
#define HR_TRAVEL_TIME        1000

// average tics per up-button press

#define HR_TPU_GFL    2500
#define HR_TPU_1FL    1500
#define HR_TPU_2FL    1000
#define HR_TPU_3FL    2500
#define HR_TPU_4FL    3000
#define HR_TPU_5FL    3500
#define HR_TPU_6FL    4000
#define HR_TPU_7FL    5000
#define HR_TPU_8FL    8000
#define HR_TPU_9FL    9000
#define HR_TPU_10FL   0

// average tics per down-button press

#define HR_TPD_GFL    0
#define HR_TPD_1FL    8000
#define HR_TPD_2FL    5000
#define HR_TPD_3FL    5000
#define HR_TPD_4FL    4500
#define HR_TPD_5FL    4000
#define HR_TPD_6FL    3000
#define HR_TPD_7FL    2500
#define HR_TPD_8FL    2500
#define HR_TPD_9FL    2500
#define HR_TPD_10FL   2000

// cumulative weight of the p()s that a caller getting into the car
// at the row floor and going up will select the column floor
//
// e.g., a passenger who gets in on 5 going up has a 5% chance of pressing
// the 6 or 7 inside buttons, and a 30% chance of pressing the 8, 9, or 10 
// inside button.

//                     0    5    5   10   10   10   12   12   12   12   12
#define HR_PIU_GFL  {  0,   5,  10,  20,  30,  40,  52,  64,  76,  88, 100}

//                     0    0    5    5    5   14   14   14   14   14   15
#define HR_PIU_1FL  {  0,   0,   5,  10,  15,  29,  43,  57,  71,  85, 100}

//                     0    0    0    5    5   15   15   15   15   15   15
#define HR_PIU_2FL  {  0,   0,   0,   5,  10,  25,  40,  55,  70,  85, 100}

//                     0    0    0    0    5    5   15   15   20   20   20
#define HR_PIU_3FL  {  0,   0,   0,   0,   5,  10,  25,  40,  60,  80, 100}

//                     0    0    0    0    0    5    5   20   20   25   25
#define HR_PIU_4FL  {  0,   0,   0,   0,   0,   5,  10,  30,  50,  75, 100}

//                     0    0    0    0    0    0    5    5   30   30   30
#define HR_PIU_5FL  {  0,   0,   0,   0,   0,   0,   5,  10,  40,  70, 100}

//                     0    0    0    0    0    0    0    5    5   45   45
#define HR_PIU_6FL  {  0,   0,   0,   0,   0,   0,   0,   5,  10,  55, 100}

//                     0    0    0    0    0    0    0    0   10   30   60
#define HR_PIU_7FL  {  0,   0,   0,   0,   0,   0,   0,   0,  10,  40, 100}

//                     0    0    0    0    0    0    0    0    0   30   70
#define HR_PIU_8FL  {  0,   0,   0,   0,   0,   0,   0,   0,   0,  30, 100}

//                     0    0    0    0    0    0    0    0    0    0  100
#define HR_PIU_9FL  {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 100}

//                   can't go up from 10
#define HR_PIU_10FL {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}

// cumulative weight of the p()s that a caller getting into the car
// at the row floor and going down will select the column floor

//                   can't go down from G
#define HR_PID_GFL  {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}

//                   100    0    0    0    0    0    0    0    0    0    0
#define HR_PID_1FL  {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}

//                    50   50    0    0    0    0    0    0    0    0    0
#define HR_PID_2FL  { 50, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}

//                    35   35   30    0    0    0    0    0    0    0    0
#define HR_PID_3FL  { 35,  70, 100, 100, 100, 100, 100, 100, 100, 100, 100}

//                    25   25   46    4    0    0    0    0    0    0    0
#define HR_PID_4FL  { 25,  50,  96, 100, 100, 100, 100, 100, 100, 100, 100}

//                    20   20   52    4    4    0    0    0    0    0    0
#define HR_PID_5FL  { 20,  40,  92,  96, 100, 100, 100, 100, 100, 100, 100}

//                    18   18   52    4    4    4    0    0    0    0    0
#define HR_PID_6FL  { 18,  36,  88,  92,  96, 100, 100, 100, 100, 100, 100}

//                    16   16   50    6    4    4    4    0    0    0    0
#define HR_PID_7FL  { 16,  32,  82,  88,  92,  96, 100, 100, 100, 100, 100}

//                    14   14   46    6    6    6    4    4    0    0    0
#define HR_PID_8FL  { 14,  28,  74,  80,  86,  92,  96, 100, 100, 100, 100}

//                    12   12   42   10    8    6    4    4    4    0    0
#define HR_PID_9FL  { 12,  24,  64,  74,  82,  88,  92,  96, 100, 100, 100}

//                    10   10   38    9    8    7    6    4    4    4    0
#define HR_PID_10FL { 10,  20,  58,  67,  75,  82,  88,  92,  96, 100, 100}

////////////////////////////////////////////////////////////////////////////////
//  typedefs
////////////////////////////////////////////////////////////////////////////////

typedef uint8_t pin_t;                      // an Arduino pin
typedef int8_t floor_t;                     // a Darth floor #
typedef unsigned long delay_t;              // a time delay in ms for delay()

////////////////////////////////////////////////////////////////////////////////
//  enums
////////////////////////////////////////////////////////////////////////////////

// possible car directions

enum CarDirection
{
  Up,
  Down,
  Stopped
};

//// possible LED states        ///ENHANCEMENT?
//
//enum LedState
//{
//  On, 
//  Off
//};

////////////////////////////////////////////////////////////////////////////////
//  System state
////////////////////////////////////////////////////////////////////////////////

//bool systemDown = false;               // whole system down?  ///ENHANCEMENT?
bool btnsChanged = false;                // did the ISR push any buttons?
floor_t currFloor = HR_INIT_FLOOR;       // current floor
CarDirection currDirection = Up;    // current direction
//bool stuckMidFloor = false;            // stuck mid-floor?    ///ENHANCEMENT?

////////////////////////////////////////////////////////////////////////////////
//  Floors setup
////////////////////////////////////////////////////////////////////////////////

// LED pins

const pin_t ledPin[HR_NUM_FLOORS]
{
  DARTH_PIN_GFL,
  DARTH_PIN_1FL,
  DARTH_PIN_2FL,
  DARTH_PIN_3FL,
  DARTH_PIN_4FL,
  DARTH_PIN_5FL,
  DARTH_PIN_6FL,
  DARTH_PIN_7FL,
  DARTH_PIN_8FL,
  DARTH_PIN_9FL,
  DARTH_PIN_10FL
};

// average tics per up-button press

const unsigned long ticsPerUp[HR_NUM_FLOORS]
{
  HR_TPU_GFL,
  HR_TPU_1FL,
  HR_TPU_2FL,
  HR_TPU_3FL,
  HR_TPU_4FL,
  HR_TPU_5FL,
  HR_TPU_6FL,
  HR_TPU_7FL,
  HR_TPU_8FL,
  HR_TPU_9FL,
  HR_TPU_10FL
};

// average tics per down-button press

const unsigned long ticsPerDown[HR_NUM_FLOORS]
{
  HR_TPD_GFL,
  HR_TPD_1FL,
  HR_TPD_2FL,
  HR_TPD_3FL,
  HR_TPD_4FL,
  HR_TPD_5FL,
  HR_TPD_6FL,
  HR_TPD_7FL,
  HR_TPD_8FL,
  HR_TPD_9FL,
  HR_TPD_10FL
};

// cumulative weight of the p()s that a caller getting into the car
// at the row floor and going up will select the column floor

const unsigned int probInsideUp[HR_NUM_FLOORS][HR_NUM_FLOORS]
{
  HR_PIU_GFL,
  HR_PIU_1FL,
  HR_PIU_2FL,
  HR_PIU_3FL,
  HR_PIU_4FL,
  HR_PIU_5FL,
  HR_PIU_6FL,
  HR_PIU_7FL,
  HR_PIU_8FL,
  HR_PIU_9FL,
  HR_PIU_10FL
};

// cumulative weight of the p()s that a caller getting into the car
// at the row floor and going down will select the column floor

const unsigned int probInsideDown[HR_NUM_FLOORS][HR_NUM_FLOORS]
{
  HR_PID_GFL,
  HR_PID_1FL,
  HR_PID_2FL,
  HR_PID_3FL,
  HR_PID_4FL,
  HR_PID_5FL,
  HR_PID_6FL,
  HR_PID_7FL,
  HR_PID_8FL,
  HR_PID_9FL,
  HR_PID_10FL
};

////////////////////////////////////////////////////////////////////////////////
//  Floor state
////////////////////////////////////////////////////////////////////////////////

//// is this floor's lamp burned out?
//
//bool ledBurnedOut[HR_NUM_FLOORS] {};  ///ENHANCEMENT?

// inside passengers stopping here

unsigned int numPassengersGoingHere[HR_NUM_FLOORS] {};

// outside callers waiting to go up

unsigned int numCallersGoingUp[HR_NUM_FLOORS] {};

// outside callers waiting to go down

unsigned int numCallersGoingDown[HR_NUM_FLOORS] {};

////////////////////////////////////////////////////////////////////////////////
//  methods:
////////////////////////////////////////////////////////////////////////////////

unsigned int demoAdj(unsigned int pUnadjusted)
{
  //////////////////////////////////////////////////////////////////////////////
  //  return pUnadjusted if the demo switch is on,
  //  else return (pUnadjusted / DARTH_SPEEDFACTOR)
  //
  //  remember that the demo switch is pulled high and grounded for on
  //////////////////////////////////////////////////////////////////////////////

  return ((digitalRead(DARTH_PIN_DEMOSW) == HIGH) ?
    pUnadjusted : 
    (pUnadjusted / DARTH_SPEEDFACTOR));
}

void delayWithInterrupts(delay_t pDelay)
{
  //////////////////////////////////////////////////////////////////////////////
  //  delay() with interrupts on
  //
  //  honor the demo switch by dividing the delay time by DARTH_SPEEDFACTOR
  //  [NOTE: the demo switch is INPUT_PULLUP, so the logic is reversed]
  //////////////////////////////////////////////////////////////////////////////

  interrupts();
  delay(demoAdj(pDelay));
  noInterrupts();
}

void refreshLeds(void)
{
  //////////////////////////////////////////////////////////////////////////////
  //  refresh LEDs to match model state
  //
  //  call every time I:
  //    change the current floor
  //    change the current direction
  //////////////////////////////////////////////////////////////////////////////

  // direction LEDs
  
  if (currDirection == Up) digitalWrite(DARTH_PIN_UPLED, HIGH);
  else                     digitalWrite(DARTH_PIN_UPLED, LOW);

  if (currDirection == Down) digitalWrite(DARTH_PIN_DOWNLED, HIGH);
  else                       digitalWrite(DARTH_PIN_DOWNLED, LOW);

  // floor LEDs

  for (floor_t f = HR_MIN_FLOOR; f <= HR_MAX_FLOOR; ++f)
  {
    ///ENHANCEMENT? handle mulitple floor LEDs being on (e.g., stuck b/w floors)
    if (currFloor == f) digitalWrite(ledPin[f], HIGH);
    else                digitalWrite(ledPin[f], LOW);

    ///ENHANCEMENT? handle burned-out LEDs
//    if ((currFloor == f) && (!ledBurnedOut[f])) digitalWrite(ledPin[f], HIGH);
//    else                                        digitalWrite(ledPin[f], LOW);
  }
}

void lampTest(void)
{
  //////////////////////////////////////////////////////////////////////////////
  //  lamp test
  //////////////////////////////////////////////////////////////////////////////

  // sweep the floor LEDs a couple of times

  for (int i = 0; i < DARTH_LAMP_TEST_MAXSWEEP; ++i)
  {
    // left to right
  
    digitalWrite(DARTH_PIN_UPLED, HIGH);
    delay(demoAdj(DARTH_LAMP_TEST_ON));
    digitalWrite(DARTH_PIN_UPLED, LOW);
    delay(demoAdj(DARTH_LAMP_TEST_OFF));
    
    for (floor_t f = HR_MIN_FLOOR; f <= HR_MAX_FLOOR; ++f)
    {
      digitalWrite(ledPin[f], HIGH);
      delay(demoAdj(DARTH_LAMP_TEST_ON));
  
      digitalWrite(ledPin[f], LOW);
      delay(demoAdj(DARTH_LAMP_TEST_OFF));
    }

    digitalWrite(DARTH_PIN_DOWNLED, HIGH);
    delay(demoAdj(DARTH_LAMP_TEST_ON));
    digitalWrite(DARTH_PIN_DOWNLED, LOW);
    delay(demoAdj(DARTH_LAMP_TEST_OFF));
    
    // then right to left
  
    digitalWrite(DARTH_PIN_DOWNLED, HIGH);
    delay(demoAdj(DARTH_LAMP_TEST_ON));
    digitalWrite(DARTH_PIN_DOWNLED, LOW);
    delay(demoAdj(DARTH_LAMP_TEST_OFF));
    
    for (floor_t f = HR_MAX_FLOOR; f >= HR_MIN_FLOOR; --f)
    {
      digitalWrite(ledPin[f], HIGH);
      delay(demoAdj(DARTH_LAMP_TEST_ON));
  
      digitalWrite(ledPin[f], LOW);
      delay(demoAdj(DARTH_LAMP_TEST_OFF));
    }

    digitalWrite(DARTH_PIN_UPLED, HIGH);
    delay(demoAdj(DARTH_LAMP_TEST_ON));
    digitalWrite(DARTH_PIN_UPLED, LOW);
    delay(demoAdj(DARTH_LAMP_TEST_OFF));
  }

  // blink all floor LEDs a couple of times

  for (int i = 0; i < DARTH_LAMP_TEST_MAXBLINK; ++i)
  {
    for (floor_t f = HR_MAX_FLOOR; f >= HR_MIN_FLOOR; --f)
    {
      digitalWrite(ledPin[f], HIGH);
    }
    delay(demoAdj(DARTH_LAMP_TEST_BLINK_ON));

    for (floor_t f = HR_MAX_FLOOR; f >= HR_MIN_FLOOR; --f)
    {
      digitalWrite(ledPin[f], LOW);
    }
    delay(demoAdj(DARTH_LAMP_TEST_BLINK_OFF));
  }
  
  // refresh LEDs to model

  refreshLeds();
}

void setCurrFloor(floor_t pFloor)
{
  //////////////////////////////////////////////////////////////////////////////
  //  set the current floor; use a setter to ensure the LEDs get refreshed
  //////////////////////////////////////////////////////////////////////////////

  currFloor = pFloor;
  refreshLeds();
}

void setCurrDirection(CarDirection pDirection)
{
  //////////////////////////////////////////////////////////////////////////////
  //  set the current direction; use a setter to ensure the LEDs get refreshed
  //////////////////////////////////////////////////////////////////////////////

  currDirection = pDirection;
  refreshLeds();
}

floor_t randomFloorByWeight(const unsigned int pWeight[], int pNum)
{
  //////////////////////////////////////////////////////////////////////////////
  //  return a random floor according to the passed cumulative weights
  //  return -1 if I couldn't find a match [should never happen]
  //////////////////////////////////////////////////////////////////////////////
  
  unsigned int r = random(100);
  bool found = false;
  floor_t f = 0;

  while (!found && (f < pNum))
  {
    found = (r < pWeight[f]);
    if (!found) ++f;
  }

  if (!found) f = -1;

  return f;
}

void travelToFloor(floor_t pFloor)
{
  //////////////////////////////////////////////////////////////////////////////
  //  travel to the new current floor
  //////////////////////////////////////////////////////////////////////////////

  // handle travel delay

  delayWithInterrupts(HR_TRAVEL_TIME);
  setCurrFloor(pFloor);

  // randomly drop some poor slob
  
  ///ENHANCEMENT?

  // randomly get stuck between floors
  
  ///ENHANCEMENT?

  // service this floor if necessary

  if (currFloorNeedsService())
  {
    // unboard any passengers

    unboard();

    // board any callers going my way

    board();
  }

  // clear out the service requests:

  numPassengersGoingHere[currFloor] = 0;
  numCallersGoingUp[currFloor] = 0;
  numCallersGoingDown[currFloor] = 0;
}

bool currFloorNeedsService(void)
{
  //////////////////////////////////////////////////////////////////////////////
  //  return whether current floor needs service:
  //
  //  - floor's inside button is pushed
  //  - outside button is pushed in the direction we're going
  //////////////////////////////////////////////////////////////////////////////
  
  return ((numPassengersGoingHere[currFloor] > 0) ||
          ((currDirection == Up) ? (numCallersGoingUp[currFloor] > 0) :
                                   (numCallersGoingDown[currFloor] > 0)));
}

void unboard(void)
{
  //////////////////////////////////////////////////////////////////////////////
  //  unboard a passenger
  //////////////////////////////////////////////////////////////////////////////

  // delay
  
  for (unsigned int i = 0; i < numPassengersGoingHere[currFloor]; ++i)
  {
    delayWithInterrupts(random(HR_MIN_UNBOARD_TIME, HR_MAX_UNBOARD_TIME));
  }

  // reset passenger count

  numPassengersGoingHere[currFloor] = 0;
}

void board(void)
{
  //////////////////////////////////////////////////////////////////////////////
  //  board a caller
  //////////////////////////////////////////////////////////////////////////////

  // delay, then let each boarder press exactly one inside button
  
  int callers = ((currDirection == Up) ? numCallersGoingUp[currFloor] :
                                         numCallersGoingDown[currFloor]);
  
  for (int i = 0; i < callers; ++i)
  {
    // delay
    
    delayWithInterrupts(random(HR_MIN_BOARD_TIME, HR_MAX_BOARD_TIME));

    // let this boarder press an inside button
    
    floor_t calledFloor;
    if (currDirection == Up)
    {
      calledFloor = 
        randomFloorByWeight(probInsideUp[currFloor],
          ARRAYSIZE(probInsideUp[currFloor]));
    }
    else
    {
      calledFloor = 
        randomFloorByWeight(probInsideDown[currFloor],
          ARRAYSIZE(probInsideDown[currFloor]));
    }
    if (calledFloor >= 0) ++numPassengersGoingHere[calledFloor];
  }

  // reset caller count
  //
  // if the ISR presses this floor's button in this direction
  // while we were boarding, this will wipe it out, 
  // but that's ok because we can ignore if someone pushes the button 
  // when the car is already there
  
  if (currDirection == Up) numCallersGoingUp[currFloor] = 0;
  else                     numCallersGoingDown[currFloor] = 0;
}

////////////////////////////////////////////////////////////////////////////////
//  ISRs
////////////////////////////////////////////////////////////////////////////////

ISR(TIMER1_COMPA_vect)
{
  //////////////////////////////////////////////////////////////////////////////
   //  TIMER1 COMPA ISR
  //
  //  Although I expect that callers are Poisson distributed, because
  //  lambda is so small, p(k>0) is close to 1/lambda,
  //  so I'm just using 1/(tics per call) for the probability that
  //  a caller will push an outside button during any given ISR tic.
  //////////////////////////////////////////////////////////////////////////////

  // heartbeat
  
  digitalWrite(LED_BUILTIN, HIGH);

  // simulate outside button pushes

  for (floor_t f = HR_MIN_FLOOR; f <= HR_MAX_FLOOR; ++f)
  {
    // going up?
    
    if ((f < HR_MAX_FLOOR) && (random(demoAdj(ticsPerUp[f])) == 0))
    {
      ++numCallersGoingUp[f];
      btnsChanged = true;
    }
    
    // going down?
    
    if ((f > HR_MIN_FLOOR) && (random(demoAdj(ticsPerDown[f])) == 0))
    {
      ++numCallersGoingDown[f];
      btnsChanged = true;
    }
  }

  // simulate lamp burnout and repair

  ///ENHANCEMENT?

  // simulate system failure and repair
  
  ///ENHANCEMENT?
  
  digitalWrite(LED_BUILTIN, LOW); ///TESTING
}

////////////////////////////////////////////////////////////////////////////////
//  setup
////////////////////////////////////////////////////////////////////////////////

void setup()
{
  //////////////////////////////////////////////////////////////////////////////
   //  set everything up
  //////////////////////////////////////////////////////////////////////////////

  interrupts();
  setupIO();

  noInterrupts();
  randomSeed(analogRead(DARTH_PIN_EMPTYA));
  setupTimerInterrupts();
}

void setupIO()
{
  //////////////////////////////////////////////////////////////////////////////
  // setup I/O pins
  //////////////////////////////////////////////////////////////////////////////

  // set demo switch to input, pulled-up

  pinMode(DARTH_PIN_DEMOSW, INPUT_PULLUP);

  // set all LEDs to output and turn off:

  pinMode(DARTH_PIN_UPLED, OUTPUT);
  digitalWrite(DARTH_PIN_UPLED, LOW);
  
  pinMode(DARTH_PIN_DOWNLED, OUTPUT);
  digitalWrite(DARTH_PIN_DOWNLED, LOW);

  for (floor_t f = HR_MIN_FLOOR; f <= HR_MAX_FLOOR; ++f)
  {
    pinMode(ledPin[f], OUTPUT);
    digitalWrite(ledPin[f], LOW);
  }

  // do a lamp test:

  lampTest();  
}

void setupTimerInterrupts()
{
  //////////////////////////////////////////////////////////////////////////////
  //  setup time interrupts
  //    Timer 1 to CTC at tic frequency
  //////////////////////////////////////////////////////////////////////////////

  noInterrupts();
  
  // set mode 4 (CTC on OCR1A), prescaler=1024 + start
  
  TCCR1A = 0;              // Make sure it is zero
  
  TCCR1B = (1 << WGM12);   // Configure for CTC mode (Set it; don't OR stuff into it)
  TCCR1B |= ((1 << CS10) | (1 << CS12)); // Prescaler @ 1024
  
  TIMSK1 = (1 << OCIE1A);  // Enable interrupt
  
  // set compare value:
  
  unsigned long t = ARDUINO_CLOCK;
  t /= 1024;
  t /= DARTH_TIMER_TIC_FREQ;
  OCR1A = t;

  interrupts();
}

////////////////////////////////////////////////////////////////////////////////
//  loop
////////////////////////////////////////////////////////////////////////////////

void loop()
{
  //////////////////////////////////////////////////////////////////////////////
  //  the main run loop
  //
  //  interrupts are always enabled coming into this
  //////////////////////////////////////////////////////////////////////////////

  // disable interrupts until we're done (delays will enable and disable them)

  noInterrupts();
  
  // any buttons above us awaiting service in either direction?

  bool waitingAbove = false;
  for (floor_t f = currFloor + 1; f <= HR_MAX_FLOOR; ++f)
  {
    waitingAbove |= ((numCallersGoingUp[f] > 0) ||
                     (numCallersGoingDown[f] > 0));
  }

  // any buttons below us awaiting service in either direction?

  bool waitingBelow = false;
  for (floor_t f = currFloor - 1; f >= HR_MIN_FLOOR; --f)
  {
    waitingBelow |= ((numCallersGoingUp[f] > 0) ||
                     (numCallersGoingDown[f] > 0));
  }

  // start moving if necessary

  setCurrDirection((currDirection == Stopped) ?
    (waitingAbove ? Up :
      (waitingBelow ? Down : Stopped)) : 
        currDirection);
  
  // handle movement

  if (currDirection == Up)
  {
    if (waitingAbove)
    {
      // we're going up and we're needed above, so go up a floor
      
      travelToFloor(currFloor + 1);
    }
    else
    {
      // we're going up but we're not needed above
      
      if (waitingBelow)
      {
        // but we're needed below, so turn around
        
        setCurrDirection(Down);
      }
      else
      {
        // and we're not needed below, so stop
        
        setCurrDirection(Stopped);
      }
    }
  }
  else if (currDirection == Down)
  {
    if (waitingBelow)
    {
      // we're going down and we're needed below, so go down a floor
      
      travelToFloor(currFloor - 1);
    }
    else
    {
      // we're going down but we're not needed below
      
      if (waitingAbove)
      {
        // but we're needed above, so turn around
        
        setCurrDirection(Up);
      }
      else
      {
        // and we're not needed anywhere, so stop
        
        setCurrDirection(Stopped);
      }
    }
  }

  // reenable interrupts

  interrupts();
}
