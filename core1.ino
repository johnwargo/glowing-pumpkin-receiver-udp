/*********************************************************
* Core 1
* 
* This is the code that interacts with the LED array, 
* looping infinitely doing the random color thing 
* if enabled.
**********************************************************/

void Task1code(void* pvParameters) {

  int randomInt;

  Serial.print("LED Management running on core ");
  Serial.println(xPortGetCoreID());

  // Repeat the following infinitely
  for (;;) {
    if (doRandom) {
      randomInt = (int)random(1, numColors + 1);
      fadeColor(colors[randomInt]);
    }
    // Add a small delay to let the watchdog process
    //https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time
    delay(25);
  }
}