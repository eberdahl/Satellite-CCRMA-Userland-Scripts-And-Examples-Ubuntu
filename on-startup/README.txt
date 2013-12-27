HOW TO MAKE pd PATCHES (or other software) START AUTOMATICALLY ON BOOT
----------------------------------------------------------------------
by Edgar Berdahl
Copyright (C) 2012


When your Satellite CCRMA kit boots up, it checks for the existence of
the file ~/on-startup/load_default_patch.  If that file exists, then it
attempts to run it as a script.

To create a simple load_default_patch, you can simply rename the file
load_default_patch_disabled using the following command:
mv ~/on-startup/load_default_patch_disabled ~/on-startup/load_default_patch
Look at that script to understand what it does.


Finally, here is some more information on the demo patches:
default-patch.pd:    This pd patch assumes that an Arduino is connected
                     by USB to your Satellite CCRMA kit and that Firmata 2.1
                     or greater has been installed on the Arduino.  The subpatch
                     get-input-from-arduino is responsible for getting
                     input from the Arduino.  It waits four seconds after
                     the patch is opened for the hardware serial connection
                     to be initialized.  Finally, it enables the
                     first four analog inputs of the Arduino.  The remainder
                     of the patch synthesizes sound as a
                     function of the A0 input from the Arduino.  It waits 8
                     seconds before enabling the audio.

                     In general, the patch is designed so that it can be
                     "played" just by touching the A0 pin of the
                     Arduino with the fingers even if no sensors are wired up.


Please also see ~/pd/labs-Music-250a-2012/lab2 for more tips about how Firmata
is used in these examples.
