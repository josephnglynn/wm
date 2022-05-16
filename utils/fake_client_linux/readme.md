False client Linux
Essentially this is the "fake" version of the window which the user sees if they move it to a different device running our os.

How will this work?
Move window off screen
Well depending on whether or not windows stop rendering when unmapped we will either: Unmap the window or move window of the monitor by moving its X, y to bottom right corner.

Start broadcasting window content
We then get the array of pixels of the window and send that everytime the window updates.

Create "fake" window
Then we will send a request to the other display and tell them to start a "fake" window. Fake window starts server That fake window will get the data when it is sent to it

Resize events and buttons and other events
When events occur on the "fake" window, we will send said event back to real window.

Considerations
If the monitors frame rates, resolutions are different we will have to scale the value when sent from "fake" window to real window Sound???