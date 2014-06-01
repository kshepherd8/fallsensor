/*
   Memsic2125
   
   Read the Memsic 2125 two-axis accelerometer.  Converts the
   pulses output by the 2125 into milli-g's (1/1000 of earth's
   gravity) and prints them over the serial connection to the
   computer.
   
   The circuit:
	* X output of accelerometer to digital pin 2
	* Y output of accelerometer to digital pin 3
	* +V of accelerometer to +5V
	* GND of accelerometer to ground
  
   http://www.arduino.cc/en/Tutorial/Memsic2125

   created 6 Nov 2008
   by David A. Mellis
   modified 30 Aug 2011
   by Tom Igoe

   This example code is in the public domain.

 */

/* PIN DEFINITIONS */

const int xPin = 2;		// X output of the accelerometer
const int yPin = 3;		// Y output of the accelerometer
const int buzzerPin = 4;     //Input pin of the buzzer (to activate it)

/* THINGS THAT SHOULD BE #defines */

const int MAX_SAMPLE_BUF_LEN = 255;

/* GLOBAL VARIABLES */

//x sample and y sample ring buffers
int x_sample_buf[MAX_SAMPLE_BUF_LEN];
int y_sample_buf[MAX_SAMPLE_BUF_LEN];

//size and current index of the ring buffers
int sample_buf_len;
int sample_buf_index;
int sample_buf_end;

int sampling_interval;

//current samples
int x_sample;
int y_sample;

//averages
double x_avg;
double x_stdev;
double y_avg;
double y_stdev;

//Serial initialization
int serial_init;

void setup()
{
    // initialize the pins connected to the accelerometer
    // as inputs:
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
    // initialize the pin connected to the buzzer
    pinMode(buzzerPin, OUTPUT);
    //signal_proc_init(10, 8);
}

void loop()
{   
    // initialize serial communications
    if (serial_init != 1)
    {
      Serial.begin(9600);
      serial_init = 1;
    }
    sample_accelerometer();
    signal_proc_tick();

    delay(sampling_interval);
}

void sample_accelerometer()
{
    int pulseX, pulseY;

    // read pulse from x- and y-axes:
    pulseX = pulseIn(xPin,HIGH);  
    pulseY = pulseIn(yPin,HIGH);

    // convert the pulse width into acceleration
    // accelerationX and accelerationY are in milli-g's: 
    // earth's gravity is 1000 milli-g's, or 1g.
    x_sample = ((pulseX / 10) - 500) * 8;
    y_sample = ((pulseY / 10) - 500) * 8;

    // print the acceleration
    Serial.print(x_sample);
    // print a tab character:
    Serial.print("\t");
    Serial.print(y_sample);
    Serial.println();
}

/*
FUNC: Signal Processor Initializer
PARAM: sample_rate - the frequency in samples per second at which
                     the tick function will be called.
PARAM: avg_time - the time interval over which to average the readings
                  from the accelerometer.
NOTE: This function initializes the averaging filter.
*/
void signal_proc_init(int sampling_rate, int avg_time)
{
    int i;

    sample_buf_len = sampling_rate*avg_time;

    sample_accelerometer();

    //initialize the running average ring buffer to the first reading
    //from the accelerometer
    for(i=0; i<sample_buf_len; i++)
    {
        x_sample_buf[i] = x_sample;
        y_sample_buf[i] = y_sample;
    }

    x_avg = x_sample;
    y_avg = y_sample;
    x_stdev = 0;
    y_stdev = 0;

    sample_buf_index = sample_buf_len - 1;
    sample_buf_end = 0;
}

void signal_proc_tick()
{
    int i;
    double sum_x_dev_sq = 0;
    double sum_y_dev_sq = 0;

    sample_buf_index = (sample_buf_index + 1) % sample_buf_len;

    x_sample_buf[sample_buf_index] = x_sample;
    y_sample_buf[sample_buf_index] = y_sample;

    x_avg += (x_sample - x_sample_buf[sample_buf_end])/sample_buf_len;
    y_avg += (y_sample - y_sample_buf[sample_buf_end])/sample_buf_len;

    sample_buf_end = (sample_buf_end + 1) % sample_buf_len;

    for(i = sample_buf_end; i <= sample_buf_index; i++)
    {
        sum_x_dev_sq += (x_avg - x_sample_buf[i])*(x_avg - x_sample_buf[i]);
        sum_y_dev_sq += (y_avg - y_sample_buf[i])*(y_avg - y_sample_buf[i]);
    }

    x_stdev = sqrt(sum_x_dev_sq/sample_buf_len);
    y_stdev = sqrt(sum_y_dev_sq/sample_buf_len);

}


