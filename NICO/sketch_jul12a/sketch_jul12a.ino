// Pines del ESP32 conectados al DM542
const int dirPin = 19;   // Dirección
const int stepPin = 18;  // Pulsos

// Parámetros del motor y microstepping
const int stepsPerRev = 200;       // Pasos por vuelta del motor (depende del modelo)
const int microstepping = 8;       // Microstepping configurado en el DM542
const int degreesPerStep = 360 / stepsPerRev; // 1.8 normalmente

// Variables de control
int numMovimientos = 0;
const int degreesToMove = 90;

void setup() {
  Serial.begin(115200);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  Serial.println("Introduce cuántas veces quieres mover el motor 90 grados:");
  while (Serial.available() == 0) {
    // Espera entrada serial
  }
  numMovimientos = Serial.parseInt();
  Serial.print("Se moverá 90 grados ");
  Serial.print(numMovimientos);
  Serial.println(" veces.");
  
  delay(1000);  // Espera antes de iniciar movimientos

  digitalWrite(dirPin, HIGH); // Dirección (puedes cambiar a LOW para invertir)
  
  for (int i = 0; i < numMovimientos; i++) {
    move90Degrees();
    delay(500); // Espera entre movimientos
  }

  Serial.println("Movimiento completado.");
}

void loop() {
  // No hace nada en el loop, todo se hace en setup
}

void move90Degrees() {
  int stepsToMove = (degreesToMove * stepsPerRev * microstepping) / 360;

  for (int i = 0; i < stepsToMove; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800); // Tiempo HIGH, ajusta según velocidad deseada
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800); // Tiempo LOW
  }
}
