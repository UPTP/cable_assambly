// Pines del ESP32 conectados al DM542
const int dirPin = 19;   // Dirección
const int stepPin = 18;  // Pulsos

// Parámetros del motor y microstepping
const int stepsPerRev = 200;       // Pasos por vuelta del motor (sin microstepping)
const int microstepping = 8;       // Microstepping configurado en el DM542

// Variables de control
int numMovimientos = 0;
const int gradosPorMovimiento = 90;

void setup() {
  Serial.begin(115200);

  // Esperar conexión del monitor serial (puede omitirse si no aplica en tu entorno)
  while (!Serial) {
    delay(10);
  }

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  Serial.println("Introduce cuántas veces quieres mover el motor 90 grados:");

  // Esperar entrada del usuario
  while (Serial.available() == 0) {
    delay(10);
  }

  numMovimientos = Serial.parseInt();
  Serial.print("Se moverá ");
  Serial.print(gradosPorMovimiento);
  Serial.print(" grados, ");
  Serial.print(numMovimientos);
  Serial.println(" veces.");

  delay(1000);  // Pequeña pausa

  digitalWrite(dirPin, HIGH); // Dirección: HIGH o LOW

  for (int i = 0; i < numMovimientos; i++) {
    moverGrados(gradosPorMovimiento);
    delay(500);  // Espera entre movimientos
  }

  Serial.println("Movimiento completado.");
}

void loop() {
  // Nada en el loop
}

void moverGrados(int grados) {
  int pasosTotales = (stepsPerRev * microstepping * grados) / 360;

  for (int i = 0; i < pasosTotales; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800); // Ajusta para velocidad
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }
}
