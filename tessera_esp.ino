float last_alt = 0;
float velocity_integral = 0;

float Kp_pos = 1.2;    
float Kp_vel = 180.0;  
float Ki_vel = 8.0;    
float Kd_vel = 12.0;   

void setup() {
  Serial.begin(921600); 
}

void loop() {
  if (Serial.available() > 0) {
    // 1. Flush old data if the buffer gets backed up
    while (Serial.available() > 64) { Serial.read(); }

    // 2. Unpack the packet "alt:vel:target"
    String data = Serial.readStringUntil('\n');
    int firstColon = data.indexOf(':');
    int secondColon = data.lastIndexOf(':');
    
    if (firstColon != -1 && secondColon != -1) {
      float current_alt = data.substring(0, firstColon).toFloat();
      float current_vel = data.substring(firstColon + 1, secondColon).toFloat();
      float target_h    = data.substring(secondColon + 1).toFloat();

      /
      float pos_error = target_h - current_alt;
      float target_vel = pos_error * Kp_pos; 
      target_vel = constrain(target_vel, -2.0, 2.0); 

      float vel_error = target_vel - current_vel;
      velocity_integral += vel_error;
      velocity_integral = constrain(velocity_integral, -100, 100);

      float thrust = 780.0 + (Kp_vel * vel_error) + (Ki_vel * velocity_integral);

      // 3. Send result back to PC
      thrust = constrain(thrust, 0, 1200);
      Serial.println(thrust);
      
      last_alt = current_alt;
    }
  }
}