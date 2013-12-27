
//// arduino example / sattelite ccrma

a = (); // environment
a.analog = 0 ! 6; // array for analog values
a.digital = 0 ! 14; // array for digital values
a.analogIns = (0..5);
a.digitalIns = (4..5);
a.digitalOuts = (7..13);
a.digitalPWM = [6];

SerialPort.listDevices
a.arduino = SCPyduino.new( SerialPort.devices[4], 57600);
// wait 5 sec

a.arduino.digitalPorts[1].active_(1);
a.arduino.digital[13].mode_(a.arduino.digital_output );
a.arduino.digital[13].write( 1 );
a.arduino.digital[13].write( 0 );

a.arduino.firmataVersion;

( 
a.routine = Routine {
	var lastDigital = -1 ! a.digital.size; 
	var lastAnalog = -1 ! a.digital.size;
	var value; 
	a.analogIns.do{ |i| a.arduino.analog[i].active_(1) }; // activate analog inputs
	a.arduino.digitalPorts[0].active_(1); // activate digital inputs
	a.arduino.digitalPorts[1].active_(1); 
	a.digitalIns.do{ |i| a.arduino.digital[i].mode_( 0 ) }; // set modes
	a.digitalOuts.do{ |i| a.arduino.digital[i].mode_( 1 ) };
	a.digitalPWM.do{ |i| a.arduino.digital[i].mode_( 3 ) };
	loop{
		a.arduino.iterate;
		a.analogIns.do.do{ |i| 	
			value = a.arduino.analog[i].value;
			if ( (lastAnalog[i] != value) && (value < 1024) ) { 
				lastAnalog[i] = value;
				a.analog[i] = value;
			} 
		};
		a.digitalIns.do{ |i| 
			value = a.arduino.digital[i].value.binaryValue;
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.digital[i] = value;
			}
		};
		a.digitalOuts.do{ |i| 
			value = a.digital[i];
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.arduino.digital[i].write( value );
			} 
		};
		a.digitalPWM.do{ |i| 
			value = a.digital[i];
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.arduino.digital[i].write( value );
			} 
	};
	0.001.wait;
	}
}.play
)


// simple monitoring
Tdef( \monitor, { loop{ a.analog.postln; 0.1.wait } } ).play // analog
Tdef( \monitor, { loop{ a.digital.postln; 0.1.wait } } ).play // digital
Tdef( \monitor ).stop

// do sth
// blink led 13
a.digital[13] = 1;
a.digital[13] = 0;
Tdef( \blink13, { inf.do{ |i| a.digital[13] = i%2; 0.1.wait; } } ).play;
Tdef( \blink13 ).stop;

// pwm on digital 6
Tdef( \pwm, { inf.do{ |i| var val = (i/50).sin.linlin(-1,1,0.05,1); a.digital[6] = val; 0.01.wait; }  } ).play
Tdef( \pwm ).stop
a.digital[6] = 0;

// analog in 0 writes pwm on digital 6                                                                   
Tdef( \analog2pwm, { inf.do { a.digital[6] = a.analog[0]/1024;  0.01.wait; } }  ).play
Tdef( \analog2pwm ).stop



// cleanup
a.routine.stop
a.arduino.close



// sound example --------------------------------

a = (); // environment --- only required i/o s
a.analog = 0 ! 6; // array for analog values
a.digital = 0 ! 14; // array for digital values
a.analogIns = [0];
a.digitalIns = [2];
a.digitalOuts = [];
a.digitalPWM = [];

SerialPort.listDevices
a.arduino = SCPyduino.new( SerialPort.devices[4], 57600);
// wait 5 sec
a.arduino.firmataVersion;

( 
a.routine = Routine {
	var lastDigital = -1 ! a.digital.size; 
	var lastAnalog = -1 ! a.digital.size;
	var value; 
	a.analogIns.do{ |i| a.arduino.analog[i].active_(1) }; // activate analog inputs
	a.arduino.digitalPorts[0].active_(1); // activate digital inputs
	a.arduino.digitalPorts[1].active_(1); 
	a.digitalIns.do{ |i| a.arduino.digital[i].mode_( 0 ) }; // set modes
	a.digitalOuts.do{ |i| a.arduino.digital[i].mode_( 1 ) };
	a.digitalPWM.do{ |i| a.arduino.digital[i].mode_( 3 ) };
	loop{
		a.arduino.iterate;
		a.analogIns.do.do{ |i| 	
			value = a.arduino.analog[i].value;
			if ( (lastAnalog[i] != value) && (value < 1024) ) { 
				lastAnalog[i] = value;
				a.analog[i] = value;
			} 
		};
		a.digitalIns.do{ |i| 
			value = a.arduino.digital[i].value.binaryValue;
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.digital[i] = value;
			}
		};
		a.digitalOuts.do{ |i| 
			value = a.digital[i];
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.arduino.digital[i].write( value );
			} 
		};
		a.digitalPWM.do{ |i| 
			value = a.digital[i];
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.arduino.digital[i].write( value );
			} 
		};
	0.001.wait;
	}
}.play
)





s.boot;
s.quit

// s.volume.volume = -40;
Ndef( \saw, { |freq = 100| Saw.ar( freq ) ! 2 } ).play;
Ndef( \saw ).stop;

Tdef( \setSaw, { // analog in 1
	loop{ 
		Ndef( \saw ).set( \freq, a.analog[0].linexp(0,1023,20,400) ); 
		0.1.wait; 
	} 
} ).play

Ndef( \saw, { |freq = 100| Saw.ar( freq.lag(0.2) ) ! 2 } ).play; // smoother interpolation


Tdef( \setSaw, { // lowpass filter
	var flt = 0 ! 12;
	loop{ 
		flt = flt.shift(1);
		flt[0] = a.analog[0];
		Ndef( \saw ).set( \freq, ( flt.sum / flt.size ).linexp(0,1023,20,400) ); 
		0.1.wait; 
	} 
} ).play

Tdef( \setSaw, { // median filter
	var flt = 0 ! 5;
	loop{ 
		flt = flt.shift(1);
		flt[0] = a.analog[0];
		Ndef( \saw ).set( \freq, ( flt.copy.sort[2] ).linexp(0,1023,20,400) ); 
		0.1.wait; 
	} 
} ).play


Tdef( \setSaw, { // switch on/off digital 2
	var flt = 0 ! 12;
	var isPlaying = false;
	loop{ 
		flt = flt.shift(1);
		flt[0] = a.analog[0];
		Ndef( \saw ).set( \freq, ( flt.sum / flt.size ).linexp(0,1023,20,400) ); 
		if ( a.digital[2] == 1 && isPlaying.not ) { Ndef( \saw ).play; isPlaying = true };
		if ( a.digital[2] == 0 && isPlaying ) { Ndef( \saw ).stop; isPlaying = false };		
		0.1.wait; 
	} 
} ).play


// cleanup
Tdef( \setSaw ).stop
Ndef( \saw ).clear
s.quit
a.routine.stop
a.arduino.close



// sampler - combining the tasks to make it more efficient

a = (); // environment
a.analog = 0 ! 6; // array for analog values
a.digital = 0 ! 14; // array for digital values
a.analogIns = [0];
a.digitalIns = [2,3,4,5];
a.digitalOuts = [];
a.digitalPWM = [];

SerialPort.listDevices
a.arduino = SCPyduino.new( SerialPort.devices[4], 57600);
// wait 5 sec
a.arduino.firmataVersion;

s.boot;
a.switchBus = Bus.control( s, 4 );
a.potBus = Bus.control( s, 1 );
a.buffers = SoundFile.collectIntoBuffers( "~/pd/labs-Music-250a-2011/lab1/oud-samples/*".standardizePath );



Ndef( \sampler, {  
	var switches = In.kr( a.switchBus, 4 );
	var pot = In.kr( a.potBus );
	pot = pot.linlin( 0.1, 1023, 0.3, 3 );
	pot = LPF.kr( pot.lag(0.3), 1 );
	Mix(  PlayBuf.ar( 1, a.buffers, pot, switches ) ) ! 2;
} ).play
	

(
a.routine = Routine {
	var lastDigital = -1 ! a.digital.size; 
	var lastAnalog = -1 ! a.digital.size;
	var value; 
	a.analogIns.do{ |i| a.arduino.analog[i].active_(1) }; // activate analog inputs
	a.arduino.digitalPorts[0].active_(1); // activate digital inputs
//	a.arduino.digitalPorts[1].active_(1); 
	a.digitalIns.do{ |i| a.arduino.digital[i].mode_( 0 ) }; // set modes
//	a.digitalOuts.do{ |i| a.arduino.digital[i].mode_( 1 ) };
//	a.digitalPWM.do{ |i| a.arduino.digital[i].mode_( 3 ) };
	loop{
		a.arduino.iterate;
		a.analogIns.do.do{ |i, j| 	
			value = a.arduino.analog[i].value;
			if ( (lastAnalog[i] != value) && (value < 1024) ) { 
				lastAnalog[i] = value;
				a.potBus.set( value );
			} 
		};
		a.digitalIns.do{ |i, j| 
			value = a.arduino.digital[i].value.binaryValue;
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.switchBus.subBus(j).set( value );
			}
		};
/*		a.digitalOuts.do{ |i| 
			value = a.digital[i];
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.arduino.digital[i].write( value );
			} 
		};
		a.digitalPWM.do{ |i| 
			value = a.digital[i];
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.arduino.digital[i].write( value );
			} 
		};
*/	}
}.play
)


// cleanup	
Ndef( \sampler ).clear
a.routine.stop
a.arduino.close



// sampler with multiple instances of a synthdef

a = (); // environment
a.analog = 0 ! 6; // array for analog values
a.digital = 0 ! 14; // array for digital values
a.analogIns = [0];
a.digitalIns = [2,3,4,5];
a.digitalOuts = [];
a.digitalPWM = [];

SerialPort.listDevices
a.arduino = SCPyduino.new( SerialPort.devices[4], 57600);
// wait 5 sec
a.arduino.firmataVersion;

s.boot;
a.buffers = SoundFile.collectIntoBuffers( "~/pd/labs-Music-250a-2011/lab1/oud-samples/*".standardizePath );


SynthDef( \sampler, {  |speed = 1, bufnum = 0|
	Out.ar( 0, PlayBuf.ar( 1, bufnum, speed, doneAction: 2 ) ! 2 );
} ).add

Synth( \sampler )	
Synth( \sampler, [ \bufnum, 1] )
Synth( \sampler, [ \bufnum, 2, \speed, 0.3] )
Synth( \sampler, [ \bufnum, 3, \speed, 10] )

(
a.routine = Routine {
	var lastDigital = -1 ! a.digital.size; 
	var lastAnalog = -1 ! a.digital.size;
	var value; 
	a.analogIns.do{ |i| a.arduino.analog[i].active_(1) }; // activate analog inputs
	a.arduino.digitalPorts[0].active_(1); // activate digital inputs
//	a.arduino.digitalPorts[1].active_(1); 
	a.digitalIns.do{ |i| a.arduino.digital[i].mode_( 0 ) }; // set modes
//	a.digitalOuts.do{ |i| a.arduino.digital[i].mode_( 1 ) };
//	a.digitalPWM.do{ |i| a.arduino.digital[i].mode_( 3 ) };
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
		};
/*		a.digitalOuts.do{ |i| 
			value = a.digital[i];
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.arduino.digital[i].write( value );
			} 
		};
		a.digitalPWM.do{ |i| 
			value = a.digital[i];
			if ( lastDigital[i] != value ) {
				lastDigital[i] = value;
				a.arduino.digital[i].write( value );
			} 
		};
*/	}
}.play
)


// cleanup	
a.routine.stop
a.arduino.close

