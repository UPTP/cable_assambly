// Pines conectados al DM542
const int dirPin = 9;   // Dirección
const int stepPin = 8;  // Pulsos

// Configura tu motor
const int stepsPerRev = 200;       // Por ejemplo, 1.8° por paso → 200 pasos por vuelta
const int microstepping = 8;       // Ajusta según el DIP switch del DM542 (1, 2, 4, 8, 16, etc.)
const int degreesToMove = 90;      // Cuántos grados por movimiento

int numMovimientos = 0;

void setup() {
  Serial.begin(9600);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  Serial.println("¿Cuántas veces deseas mover el motor 90 grados?");
  while (Serial.available() == 0) {
    // Espera entrada por el monitor serial
  }

  numMovimientos = Serial.parseInt();
  Serial.print("Moviendo motor 90° ");
  Serial.print(numMovimientos);
  Serial.println(" veces...");

  delay(1000); // Pequeña pausa

  digitalWrite(dirPin, HIGH); // Dirección hacia la derecha (cambia a LOW para otro sentido)

  for (int i = 0; i < numMovimientos; i++) {
    move90Degrees();
    delay(500); // Pausa entre movimientos
  }

  Serial.println("Movimiento completado.");
}

void loop() {
  // Nada aquí
}

void move90Degrees() {
  int stepsToMove = (degreesToMove * stepsPerRev * microstepping) / 360;

  for (int i = 0; i < stepsToMove; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800);  // Ajusta según velocidad
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }
}

