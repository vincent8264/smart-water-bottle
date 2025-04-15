# Introduction - Smart Water Bottle
This system integrates sensors in a water bottle cap to record the user's daily water intake and remind them to drink water as needed. 

## Features
1. Real-time water intake tracking using a flow meter installed in the bottle cap.
2. Temperature monitoring of the water inside the bottle.
3. Live information display on an OLED screen.
4. Daily drinking goal alerts if the intake is below a predefined threshold.


## System and sensors
1. Arduino UNO – Main platform
2. Water Flow Sensor (G3/4) – Measures water flow per second to determine drinking events.
3. Real-Time Clock Module (DS1302) – Keeps track of current time to schedule reminders.
4. Temperature Sensor (DS18B20) – Measures water temperature inside the bottle.
5. OLED Display (SSD1306) – Shows current water temperature and daily intake.
6. Buzzer – Alerts user if they haven’t drunk enough water.

# System behavior
The system runs a loop every second and operates in three states:

## 1. Idle state
When no water is flowing:

Displays total water intake for the day and current water temperature.

```
Total today: XX mL
Temperature: XX °C
```
## 2. Logging state
When water is detected:

Accumulates the amount of water being consumed.

## 3. Displaying state
Once drinking stops (flow = 0), logs the time and updates the total intake.

Displays a message for 7 seconds:

```
Drank XX mL at HH:MM:SS
Total today: XX mL
```

## Reminder System
Every minute, the system checks whether the user's water intake meets the set criteria.

If the goal is not met, the system displays a reminder and sounds the buzzer:

```
Drink Water!!
So far: XX mL
You should drink: XX mL
```

# Flow Chart
![flowchart](/flow.jpg)
