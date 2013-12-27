/*RUN*/


a = (); // environment
a.analog = 0 ! 6; // array for analog values
a.digital = 0 ! 14; // array for digital values
a.analogIns = [0];
a.digitalIns = [2,3,4,5];
a.digitalOuts = [];
a.digitalPWM = [];

s.waitForBoot {

a.arduino = SCPyduino.new( SerialPort.devices[4], 57600);

5.wait;

	a.buffers = SoundFile.collectIntoBuffers( "~/pd/labs-Music-250a-2011/lab1/oud-samples/*".standardizePath );


SynthDef( \sampler, {  |speed = 1, bufnum = 0|
	Out.ar( 0, PlayBuf.ar( 1, bufnum, speed, doneAction: 2 ) ! 2 );
} ).add;

s.sync;

Synth(\sampler, [\bufnum, 0, \seed, 1  ] ).play;
Synth(\sampler, [\bufnum, 1, \seed, 1  ] ).play;
Synth(\sampler, [\bufnum, 2, \seed, 1  ] ).play;
Synth(\sampler, [\bufnum, 3, \seed, 1  ] ).play;

s.sync;

a.routine = Routine {
	var lastDigital = -1 ! a.digital.size; 
	var lastAnalog = -1 ! a.digital.size;
	var value; 
	a.analogIns.do{ |i| a.arduino.analog[i].active_(1) }; // activate analog inputs
	a.arduino.digitalPorts[0].active_(1); // activate digital inputs
	a.digitalIns.do{ |i| a.arduino.digital[i].mode_( 0 ) }; // set modes
	loop{
		a.arduino.iterate;
		a.analogIns.do.do{ |i, j| 	
			value = a.arduino.analog[i].value;
			if ( (lastAnalog[i] != value) && (value < 1024) ) { 
				lastAnalog[i] = value;
				a.analog[i] = value;
			} 
		};
		a.digitalIns.do{ |i, j| 
			value = a.arduino.digital[i].value.binaryValue;
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				if ( value == 1 ) {
					Synth( \sampler, [\bufnum, j, \speed, a.analog[0].linlin( 0.1, 1023, 0.3, 3 ) ]);
				}
			}
		}
	}
}.play;

}
