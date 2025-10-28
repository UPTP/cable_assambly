#define ENA_PIN 2
#define DIR_PIN 4
#define PUL_PIN 5

#define STEPS_PER_REV 200
#define MICROSTEPPING 16
#define DEG_PER_STEP (360.0 / (STEPS_PER_REV * MICROSTEPPING))

float A = 250;       // ángulo total a girar
float Vmax = 180.0;  // deg/s máxima velocidad
float a = 360.0;     // deg/s² aceleración

float Amin, t_accel, t_const, t_total;
bool isTrapezoidal;

unsigned long startTime;
int totalSteps;
int stepCount;

unsigned long lastPulseTime = 0;
unsigned long pulseInterval = 1000;  // microsegundos entre pulsos (inicial)

bool pulseState = false;

void setup() {
  pinMode(ENA_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PUL_PIN, OUTPUT);

  digitalWrite(ENA_PIN, LOW);  // habilitar motor
  digitalWrite(DIR_PIN, HIGH); // dirección

  Serial.begin(115200);

  computeMotionProfile(A, Vmax, a);
  totalSteps = round(A / DEG_PER_STEP);
  stepCount = 0;
  startTime = micros();
}

void loop() {
  if (stepCount >= totalSteps) {
    return;  // movimiento completado
  }

  unsigned long now = micros();
  float t = (now - startTime) / 1e6;  // tiempo en segundos

  float v = getAngularSpeed(t);       // velocidad en deg/s
  float stepsPerSec = v / DEG_PER_STEP;

  if (stepsPerSec < 1e-3) return; // velocidad casi cero, no avanzar

  pulseInterval = 1000000UL / stepsPerSec;

  if (now - lastPulseTime >= pulseInterval / 2) {
    pulseState = !pulseState;
    digitalWrite(PUL_PIN, pulseState);
    lastPulseTime = now;

    if (!pulseState) {
      stepCount++;
      Serial.print("Step ");
      Serial.println(stepCount);
    }
  }
}

void computeMotionProfile(float A, float Vmax, float a) {
  Amin = (Vmax * Vmax) / a;
  if (A >= Amin) {
    isTrapezoidal = true;
    t_accel = Vmax / a;
    t_const = (A - Amin) / Vmax + t_accel;
    t_total = t_const + t_accel;
  } else {
    isTrapezoidal = false;
    float Vpeak = sqrt(a * A);
    t_accel = Vpeak / a;
    t_total = 2 * t_accel;
  }
  Serial.print("Perfil: ");
  Serial.println(isTrapezoidal ? "Trapezoidal" : "Triangular");
}

float getAngularSpeed(float t) {
  if (isTrapezoidal) {
    if (t < t_accel) return a * t;
    else if (t < t_const) return Vmax;
    else if (t < t_total) return a * (t_total - t);
    else return 0;
  } else {
    if (t < t_accel) return a * t;
    else if (t < t_total) return a * (t_total - t);
    else return 0;
  }
}
