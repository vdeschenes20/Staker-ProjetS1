#include "ServoPince.h"

void setupServoPince() {
    pince.attach(SERVO_PIN);
    pince.write(angleOuvert);
}