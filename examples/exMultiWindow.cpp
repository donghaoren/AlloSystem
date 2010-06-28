
#include <stdio.h>
#include <stdlib.h>

#include "types/al_MsgQueue.hpp"
#include "system/al_MainLoop.hpp"
#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"
#include "math/al_Random.hpp"

using namespace al;

Camera camera;
gfx::Graphics gl;
double eyesep = 1;
		

// invent some kind of scene:
struct vertex {
	float x, y, z;
	float r, g, b;
};
#define NUM_VERTICES (256)
static vertex vertices[NUM_VERTICES];

		
static void drawScene(void * self) {

	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
//	gl.begin(gl.LINES);
//		for (double z = -10; z<10; z += 1) {
//		for (double y = 0; y<1; y += .1) {
//		for (double x = 0; x<1; x += .1) {
//			gl.color(abs(cos(M_PI * (x-y))), 0.5, abs(sin(M_PI * (x+y))));
//			gl.vertex(cos(M_PI * (x-y)), sin(M_PI * (x+y)), z*8);
//		}}}
//	gl.end();
	
	gl.begin(gl.LINES);
		for (int i=0; i<NUM_VERTICES; i++) {
			vertex& v = vertices[i];
			gl.color(v.r, v.g, v.b, 0.5);
			gl.vertex(v.x, v.y, v.z);
		}
	gl.end();
}



struct MyWindow : WindowGL{

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	printf("onKeyDown    "); printKey(); 
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
		}
	}
	void onKeyUp(const Keyboard& k){	printf("onKeyUp      "); printKey(); }
	
	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
	
	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	void onFrame(){
		//drawScene(this);
		Context::draw(drawScene, dimensions().w, dimensions().h, this);
	}
};

MyWindow win1;
MyWindow win2;
MyWindow win3;
MyWindow win4;

/*
	Here's one way to skip processing if the engine can't keep up.
	The actual time taken to render is measured against the desired frame rate
	If the last frame overflowed (cpu > 1.0), the current frame will be skipped
	(if frames were not skipped, the scheduler could end up clogging up and freezing the app)
*/
al_sec step_period = 1./200;
al_sec overtime = 0;
al_sec lastframe = 0;
void step(al_sec t) {
	// logical time of next step
	al_sec next_t = t+step_period;				
	
//	al_sec fps = 0;

	win3.viewport().userProjectionTransform().set(
		//Matrix4d::ShearYZ(0, sin(al_time())) 
		//* 
		Matrix4d::Scale(2, 1, 1) 
		* Matrix4d::Translate(-1, 0, 0)
	);	
	
	// only render if we are keeping to our deadlines:
	//if (overtime <= 0) {		///< the right way to do it
	//if (MainLoop::cpu() < 1) {	///< the lazy way to do it
		
		camera.vec()[2] = sin(t);
		camera.step();
		/* we set fps to zero for both windows, so that we can drive them from here instead */
//		win1.doFrame();
//		win2.doFrame();
//		win3.doFrame();
//		win4.doFrame();
		
//		al_sec frt = al_time() - MainLoop::T0();		// actual time now
//		fps = 1./(frt-lastframe);
//		lastframe = frt;
	//} 
	
	// for the right way:
	// test whether we are keeping to our deadlines:
	al_sec rt = al_time() - MainLoop::T0();		// actual time now
	// overtime<0 indicates that we are keeping up
	// overtime>0 indicates that we cannot keep up
	overtime = rt - next_t;						
	// note: this measurement is also affected by any other processes in the scheduler!
	al_sec cpu = (overtime/step_period)+1.;
	printf("step/frame cpu: %6.2f%%, main cpu: %6.2f%%\n", cpu*100, MainLoop::cpu()*100);
	

	MainLoop::queue().send(rt + step_period, step);
}

double fps = 200;

int main (int argc, char * argv[]) {

	rnd::Random<> rng;

	/// define the scene - a semi random walk
	for (int i=0; i<NUM_VERTICES; i++) {
	
		double r, g, b, x, y, z;
		
		double p = i/(double)NUM_VERTICES;
		r = p;
		g = 1-p;
		b = sin(p * 6 * M_PI);
	
		y = b;
		z = 5 + 2*sin(p * 2 * M_PI);
		
		vertices[i].x = -1.5;
		vertices[i].y = y;
		vertices[i].z = z;
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;
		
		i++;
		
		vertices[i].x = 1.5;
		vertices[i].y = y;
		vertices[i].z = z;
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;

	} 
	
	// set fps == 0, and drive frame rendering manually instead
	win1.create(WindowGL::Dim(320,240,0,40), "win1", fps);
	win1.mode(WindowGL::Dual);
	win1.stereo(true);
	win1.viewport().camera(&camera);
	
	win2.create(WindowGL::Dim(320,240,0,320), "win2", fps);
	win2.mode(WindowGL::Anaglyph);
	//win2.anaglyphMode(WindowGL::RedCyan);
	win2.stereo(true);
	win2.viewport().camera(&camera);
	
	win3.create(WindowGL::Dim(320,240,0,560), "win3", fps);
	win3.mode(WindowGL::LeftEye);
	win3.stereo(true);
	win3.viewport().camera(&camera);
	//win3.viewport().userProjectionTransform().set(Matrix4d::Scale(1, 0.5, 1));
//	win3.viewport().userProjectionTransform().set(
//		//Matrix4d::Scale(0.5, 1, 1) *
//		Matrix4d::ShearZ(0.5, 0.) *
//		Matrix4d::Translate(0.5, 0, 0)
//	);
	
	win4.create(WindowGL::Dim(320,240,320,560), "win4", fps);
	win4.mode(WindowGL::RightEye);
	win4.stereo(true);
	win4.viewport().camera(&camera);

	
	camera.turn(Quatd::fromEuler(0., 0., 0.1));
	MainLoop::queue().send(0., step);
	MainLoop::interval(0.01);
	MainLoop::start();
	return 0;
}
