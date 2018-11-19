
#include <ThreadController.h>
#include <StaticThreadController.h>
#include <Thread.h>

const int NUM_WIRES = 7;

class ELWire {

    int pin;

  public:

    bool isOn = false;

    ELWire(int input) {
      pin = input;
      pinMode(pin, OUTPUT);
      turnOff();
    }

    void turnOn() {
      digitalWrite(pin, HIGH);
      isOn = true;
    }

    void turnOff() {
      digitalWrite(pin, LOW);
      isOn = false;
    }
};


ELWire wires[] = {
  ELWire(2),
  ELWire(3),
  ELWire(4),
  ELWire(5),
  ELWire(6),
  ELWire(7),
  ELWire(8),
  ELWire(9)
};

class Decay: public Thread {
    ELWire *wires_;
    int limit = 0;
  public:
    Decay(int pause, ELWire _wires[]) {
      setInterval(pause);
      wires_ = _wires;
    }

    void reset(int _limit) {
      limit = _limit;
    }
    void run() {
      ELWire *wire = wires_ + NUM_WIRES - 1;
      for (int i = 0; i < NUM_WIRES - limit; i++) {
        if (wire->isOn) {
          wire->turnOff();
          break;
        }
        wire--;
      }
      runned();
    }
};

class Burst: public Thread {

    int limit = 0;
    ELWire *wires_;
  public:
    Burst(int jump, ELWire _wires[]) {
      wires_ = _wires;
      setInterval(jump);
      limit = 0;
    }

    void reset(int _limit) {
      limit = _limit;
    }
    void run() {
      ELWire *wire = wires_;
      for (int i = 0; i < limit; i++) {
        if (!wire->isOn) {
          //          wire->identify();
          //          Serial.println(": Turning on");
          wire->turnOn();
          break;
        }
        wire++;
      }
      runned();
    }
};

const int AUDIO_SAMPLE_INTERVAL = 10; // millis - ish

const float SAMPLE_PERIOD = 1; // seconds - ish

const int NUM_SAMPLES = SAMPLE_PERIOD * 1000 / AUDIO_SAMPLE_INTERVAL;

class AudioSensor: public Thread
{
  protected:
    int value;
    int pin;
    int curSample = 0;
    int samples[NUM_SAMPLES];

  public:

    AudioSensor(int input) {
      pin = input;
      pinMode(pin, INPUT);
      setInterval(AUDIO_SAMPLE_INTERVAL);
    }

    int amplitude() {
      return value;
    }

    void run() {
      // Reads the analog pin, and saves it localy
      int amp = analogRead(pin);
      Serial.print(amp);
      if (curSample >= NUM_SAMPLES) {
        curSample = 0;
      }
      samples[curSample++] = amp;
      int m = 0;
      int n = 9999;//big
      for (int i = 0; i < NUM_SAMPLES; i++) {
        int sample = samples[i];
        m = max(sample, m);
        n = min(sample, n);
      }

//      if (m - n <= 20) {
//        value = 1;
//      } else {
        value = map(amp, n, m, 0, NUM_WIRES);
//      }

      //      value = map(amp, n, m, 0, NUM_WIRES);
      Serial.print(" ");
      //      Serial.println(value);
      Serial.print(m);
      Serial.print(":");
      Serial.print(n);
      Serial.print("-> ");
      Serial.println(value);
      runned();
    }
};


AudioSensor audio = AudioSensor(A2);
//
Decay decay = Decay(100, wires);

Burst burst = Burst(1, wires);

StaticThreadController<3> threads (&audio, &decay, &burst);

void showBoat(int pause) {

  for (int i = 0; i < NUM_WIRES; i++) {
    digitalWrite (i + 2, HIGH);
    delay(pause);
  }


  for (int i = 0; i < NUM_WIRES; i++) {
    digitalWrite (i + 2, LOW);
    delay(pause);
  }

  for (int i = 0; i < 5; i++) {

    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
}

/*******************Setup Loop***************************/
void setup() {
  Serial.begin(9600);

  // led
  pinMode(13, OUTPUT);

  showBoat(200);
  Serial.println("Setup complete");

}

/*******************Main Loop***************************/

void loop()
{
  threads.run();
  int amp = audio.amplitude();
  burst.reset(amp);
  decay.reset(amp);
}
