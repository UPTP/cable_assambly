// Pines de control del driver DM542 (reconfigurados para ESP32 NodeMCU-32)
#define ENA_PIN 2    // ENA: Habilita motor
#define DIR_PIN 4    // DIR: Dirección
#define PUL_PIN 5    // PUL: Pulso de paso (PWM)

#define STEPS_PER_REV 200  // pasos por vuelta sin microstepping
#define MICROSTEPPING 16   // ajusta según la configuración de tu driver
#define TOTAL_STEPS (STEPS_PER_REV * MICROSTEPPING)
#define QUARTER_STEPS (TOTAL_STEPS / 4)

void setup() {
  pinMode(ENA_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PUL_PIN, OUTPUT);

  digitalWrite(ENA_PIN, LOW);  // Activar motor (ENA en LOW)
  digitalWrite(DIR_PIN, HIGH); // Dirección: HIGH (puedes cambiar a LOW si gira al revés)

  Serial.begin(115200);
}

void loop() {
  for (int quarter = 0; quarter < 4; quarter++) {
    moveWithAccelerationProfile(QUARTER_STEPS);
    delay(1000);  // Pausa de 1 segundo después de cada 90°
  }

  delay(3000); // Pausa larga antes de reiniciar ciclo
}

// Función para mover una cierta cantidad de pasos con 3 aceleraciones y 3 desaceleraciones
void moveWithAccelerationProfile(int steps) {
  int accelSteps = steps / 6; // 3 aceleración + 3 desaceleración
  int constSteps = steps - (accelSteps * 6); // pasos a velocidad constante

  // Fases de aceleración
  for (int i = 1; i <= 3; i++) {
    int delayMicros = map(i, 1, 3, 800, 300); // más rápido cada etapa
    pulseSteps(accelSteps, delayMicros);
  }

  // Pasos constantes (si hay)
  if (constSteps > 0) {
    pulseSteps(constSteps, 300); // velocidad estable
  }

  // Fases de desaceleración
  for (int i = 3; i >= 1; i--) {
    int delayMicros = map(i, 1, 3, 800, 300); // más lento cada etapa
    pulseSteps(accelSteps, delayMicros);
  }
}

// Función para enviar 'n' pulsos con un cierto delay (velocidad)
void pulseSteps(int numSteps, int delayMicros) {
  for (int i = 0; i < numSteps; i++) {
    digitalWrite(PUL_PIN, HIGH);
    delayMicroseconds(delayMicros);
    digitalWrite(PUL_PIN, LOW);
    delayMicroseconds(delayMicros);
  }
}

