
[\internal, \local].do{ |s|
	s = Server.perform(s);
	s.options.memSize = 2**16;
	s.options.numWireBufs = 1024;
	s.options.numAudioBusChannels = 4096;
	s.options.numControlBusChannels = 8192;
	s.options.numInputBusChannels = 2;
	s.options.numOutputBusChannels = 2;
	s.latency = 0.5;
}


