# FallDetect

A holistic smart cane system to help seniors gain back independence while still being safe and close to loved ones.
See it work here: https://github.com/user-attachments/assets/9c0ba106-aa48-4859-9a3a-d31d51bb05ae
![full_cane_prototype_high_def](https://github.com/user-attachments/assets/1f9e5610-853e-4cff-b74b-5df3ae6e0a21)
![circuit_front](https://github.com/user-attachments/assets/be05aec2-8bfd-4b2f-bd41-456760e4a6ad)
<img width="400" alt="Screenshot 2024-04-18 at 4 28 37 PM" src="https://github.com/user-attachments/assets/3ba1789d-75ba-4eed-b30f-2b168add92a5">

This cane uses 3 sensors:
- a hand-made force sensor
- a gyroscope
- a tilt sensor
to determine when an individual has fallen compared to non-falling gestures like releasing the cane upright or slowly releasing it horizontally using an FSM coded on an Arduino. It relays its state (safely walking, falling, fallen) to an app using Bluetooth UART communication.
