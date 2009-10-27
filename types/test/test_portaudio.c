
#include "al_types.h"
#include "portaudio.h"

PaError err;

AlloLatticeHeader latticetype;
AlloLattice inputlattice, outputlattice;

int phase = 0;
double pincr = 0.06;

int callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) 
{
	/* convert stream data to AlloLatticeTy */
	inputlattice.data.ptr = (void *)input;
	outputlattice.data.ptr = output;
	
	/* do any processing here: */
	assert(frameCount == outputlattice.header.dim[1]);
	
	char * outp = outputlattice.data.ptr;
	int frame_stride = outputlattice.header.stride[1];
	for (int i=0; i<outputlattice.header.dim[1]; i++) {
		float * out = (float *)(outp + frame_stride * i);
		out[1] = sin(pincr * (phase+i));
		phase += 0.01;
	}
	
	phase += frameCount;

	return 0;
}

int main(int argc, char * argv) {

	PaStream* stream;
	
	err = Pa_Initialize();
	if (err != paNoError) goto out;
	err = Pa_OpenDefaultStream( &stream,
                              2,
                              2,
                              paFloat32,
                              44100.0,
                              64,
                              callback,
                              NULL );
	if (err != paNoError) goto out;
	
	/* configure lattice type for audio IO */
	latticetype.type = AlloFloat32Ty;
	/* note that portaudio by default gives us interleaved data */
	latticetype.components = 1; /* single sample units */
	latticetype.dimcount = 2; /* 1 dim */
	latticetype.dim[0] = 2; /* 2 channels */
	latticetype.dim[1] = 64; /* block size */
	latticetype.stride[0] = latticetype.components * allo_type_size(latticetype.type); 
	latticetype.stride[1] = latticetype.stride[0] * latticetype.dim[0]; 
	
	allo_lattice_setheader(&inputlattice, &latticetype);
	allo_lattice_setheader(&outputlattice, &latticetype);
	
	err = Pa_StartStream(stream);
	if (err != paNoError) goto out;
	
	Pa_Sleep(2000);
	
	err = Pa_StopStream(stream);
	if (err != paNoError) goto out;
	err = Pa_CloseStream(stream);
	if (err != paNoError) goto out;
	err = Pa_Terminate();
	if (err != paNoError) goto out;
	return 0;
out:
	printf("PaError %s\n", Pa_GetErrorText(err));
	return -1;
}