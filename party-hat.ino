
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

class Decay {
    ELWire *wires_;
    int pause;
    int tick;
  public:
    Decay(int _pause, ELWire _wires[]) {      
      wires_ = _wires;
      pause = _pause;
      tick = 0;
    }

    void run(int limit) {
      if(++tick % pause == 0){
        ELWire *wire = wires_ + NUM_WIRES - 1;
        for (int i = 0; i < NUM_WIRES - limit; i++) {
          if (wire->isOn) {
            wire->turnOff();
            break;
          }
          wire--;
        } 
      tick = 0;
      }
    }
};

class Burst {
    ELWire *wires_;
  public:
    Burst(ELWire _wires[]) {
      wires_ = _wires;
    }

    void run(int limit) {
      ELWire *wire = wires_;
      for (int i = 0; i < limit; i++) {
        if (!wire->isOn) {
          wire->turnOn();
          break;
        }
        wire++;
      }
    }
};

const int NUM_SAMPLES = 100;



class AudioSensor
{
  protected:
    int pin;
    int curSample = 0;
    int samples[NUM_SAMPLES];

  public:

    AudioSensor(int input) {
      pin = input;
      pinMode(pin, INPUT);
      for (int i = 0; i < NUM_SAMPLES; i++) {
        samples[i] = 0;
      }
    }

    long fmap(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
    int amplitude() {
      // Reads the analog pin, and saves it localy
      float amp = analogRead(pin);
      
      if (curSample >= NUM_SAMPLES) {
        curSample = 0;
      }
      samples[curSample++] = amp;
      float total = 0.0;
      float macs = 0.0;
      float myn = 9999.0;
      for (int i = 0; i < NUM_SAMPLES; i++) {
        float sample = samples[i];
        if(sample > macs){
          macs = sample;
        }
        if(sample < myn){
          myn = sample;
        }
        total += sample;
      }

      float avg = total/float(NUM_SAMPLES);

      float aDiff = amp - avg;
      return int(fmap(amp, myn, macs, 0, min(aDiff,NUM_WIRES)));  
    }
};


AudioSensor audio = AudioSensor(A2);

Decay decay = Decay(15, wires);

Burst burst = Burst(wires);

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
 // Serial.begin(9600);

  // led
  pinMode(13, OUTPUT);
    
  showBoat(200);
  //Serial.println("Setup complete");

}

/*******************Main Loop***************************/

void loop()
{
  int amp = audio.amplitude();
  burst.run(amp);
  decay.run(amp);
}
