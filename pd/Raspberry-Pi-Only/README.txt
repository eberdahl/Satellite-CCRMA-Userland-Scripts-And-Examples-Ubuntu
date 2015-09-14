In my opinion, the object gpio is nicer because it doesn't require pd to be 
started with root priviledges.  However, because of that, the pin modes can't 
be changed from within pd.  However, they can be changed by sending messages 
to the "shell" object, which causes them to get executed in the terminal.  From
there, then it is possible to use Linux's gpio command to manipulate the mode 
of the pins.

In contrast, the newer wiringPi_gpio object can only be used if pd is started 
as root (e.g. first run 'sudo su' from the terminal and then start pd).  
However, it's not so nice to start pd as root, and then the startup scripts 
have to be modified later to get the patch to start on boot and work.

NOTE:  If a pd patch is loaded with the wiringPi_gpio object in it but pd isn't running with root priviledges, pd will quit without reporting an error.


Look at the demo test patches:
test-gpio.pd  wiringPi_gpio-help.pd

Edgar Berdahl, Sept. 2015
