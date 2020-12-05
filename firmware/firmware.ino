
//declare variables for the motor and IR sensor pins
#define motorPin1 12
#define motorPin2 11
#define motorPin3 10
#define motorPin4 9
#define sensor A0


int motorSpeed = 4000;  //variable to set stepper speed
int count = 0;          // count of steps made
int countsperrev = 50; // number of steps per full revolution
int lookup[8] = {B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001};


//////////////////////////////////////////////////////////////////////////////

void setup() {
  //declare the motor pins as outputs
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  //declare the sensor pin as input
  pinMode(sensor, INPUT);
  //start the serial communication
  Serial.begin(115200);
  //wait for initialization
  delay(20);

}


void loop() {
  if (Serial.available())
  {
    //get the command
    String cmd = Serial.readString();
    if (cmd == "connect")
    {
      //on connection with the PC
      Serial.print("ok");
      home();
    }
    else
    {
      //move to the desired position
      move_pct(cmd.toInt());
      Serial.print(cmd);

    }
  }
}

void move_pct(int pct)
{
  //Rotate to a position with a certain pct of transmission
  int steps = int(map(pct, 0, 100, 39, 0));
  if (steps >= count)
    rotate(steps - count, true);
  else
    rotate(count - steps, false);
  count = steps;
}

void home()
{
  //Find the reference position and go back
  long current, last_val;
  int steps = 1;
  float ratio = 0;
  rotate(1, true);
  last_val = analogRead(sensor);
  while (ratio < 1.15)
  {
    rotate(1, true);
    steps++;

    current = analogRead(sensor);
    delay(2);
    ratio = float(current) / float(last_val);
    last_val = current;
  }
  if (steps > countsperrev)
    steps = steps % countsperrev;
  steps = countsperrev - steps;

  if (steps >= 20)
    steps -= 20;
  else if (steps <= 10)
    steps += 29;
  else
    steps = 0;

  Serial.println(map(steps, 0, 39, 100, 0));
  //go to the origin position
  rotate(20, true);
  //move to the initial pct
  move_pct(map(steps, 0, 39, 100, 0));
}


void rotate(int steps, bool clockwise_rot)
{
  //rotate the disk for a certain nr of steps
  for (int s = 0; s < steps; s++)
  {
    if (!clockwise_rot)
      clockwise();
    else
      anticlockwise();
  }
}


void anticlockwise()
{
  //rotate one step anticlockwise
  for (int i = 0; i < 8; i++)
  {
    setOutput(i);
    delayMicroseconds(motorSpeed);
  }
}

void clockwise()
{
  //rotate one step clockwise
  for (int i = 7; i >= 0; i--)
  {
    setOutput(i);
    delayMicroseconds(motorSpeed);
  }
}

void setOutput(int out)
{
  //low level output assignment
  digitalWrite(motorPin1, bitRead(lookup[out], 0));
  digitalWrite(motorPin2, bitRead(lookup[out], 1));
  digitalWrite(motorPin3, bitRead(lookup[out], 2));
  digitalWrite(motorPin4, bitRead(lookup[out], 3));
}
