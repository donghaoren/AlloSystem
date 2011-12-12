#ifndef INCLUDE_AL_UTIL_NAVIGATOR_HPP
#define INCLUDE_AL_UTIL_NAVIGATOR_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Utilities for 3D navigation

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
	Pablo Colapinto, 2010, wolftype@gmail.com
*/

#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Quat.hpp"
#include "allocore/spatial/al_Pose.hpp"

namespace al {

/*
	Navigator stores temporal changes to be applied to a Pose
		changes are relative the the Pose's local coordinate frame
		
	example:
	
	Pose eye;
	Navigator nav;
	SmoothPose navsmooth;
	
	// in response to UI input etc.:
	nav.turnU(1);	// turn 1 degree rightwards per frame
	nav.moveZ(1);	// move forward 1 unit per frame
	
	// update eye per frame; and return a smoothed Pose:
	navsmooth(nav(eye));
	
*/
class Navigator {
public:
	Navigator() /*, mDPose(Pose(), 0.1, 0.1)*/ {}
	
	// apply stored velocities to input pose 
	// (modifies in-place, but also returns it for convenience)
	Pose operator()(Pose& pose) {
		mQuat.fromEuler(mEuler + mEulerI);
		Pose p(pose);
		// integrate stored local rotation:
		p.quat() *= mQuat;
		// integrated mRelVel (unprojected from from local to global frame):
		p.pos() += p.quat().rotate(mRelVel + mRelVelI + mNudge);
		mNudge.set(0);
		mRelVelI.set(0);
		mEulerI.set(0);
		return p;
	}
	
	// projects forward by some estimated quantity. non-destructive (const)
	Pose extrapolate(Pose& pose, double fraction) const {
		Pose target(pose);
		
		// integrate stored local rotation:
		target.quat() *= Quatd().fromEuler(mEuler + mEulerI);
		target.pos() += target.quat().rotate(mRelVel + mRelVelI + mNudge);
		
		// interpolation is extrapolation:
		return pose.lerp(target, fraction);
	}
	
	// set stored local frame rotations (more naturally expressed in Euler angles):
	void turn(const Vec3d& v) { mEuler.set(v);  }
	void turnU(double degrees) {
		mEuler[0] = M_DEG2RAD * degrees; //mQuat.fromEuler(mEuler);
	}
	void turnR(double degrees) {
		mEuler[1] = M_DEG2RAD * degrees; //mQuat.fromEuler(mEuler);
	}
	void turnF(double degrees) {
		mEuler[2] = M_DEG2RAD * degrees; //mQuat.fromEuler(mEuler);
	}
	
	void rotate(const Vec3d& v) { mEulerI = v; }
	void rotateU(double degrees) {
		mEulerI[0] = M_DEG2RAD * degrees; //mQuat.fromEuler(mEuler);
	}
	void rotateR(double degrees) {
		mEulerI[1] = M_DEG2RAD * degrees; //mQuat.fromEuler(mEuler);
	}
	void rotateF(double degrees) {
		mEulerI[2] = M_DEG2RAD * degrees; //mQuat.fromEuler(mEuler);
	}
	
	// set stored local frame velocity:
	void move(const Vec3d& v) { mRelVel.set(v); }
	void moveR(double amt) {
		mRelVel[0] = amt;
	}
	void moveU(double amt) {
		mRelVel[1] = amt;
	}
	void moveF(double amt) {
		mRelVel[2] = amt;
	}
	
	// set instantaneous local frame velocity:
	void push(const Vec3d& v) { mRelVelI.set(v); }
	void pushR(double amt) {
		mRelVelI[0] = amt;
	}
	void pushU(double amt) {
		mRelVelI[1] = amt;
	}
	void pushF(double amt) {
		mRelVelI[2] = amt;
	}
	
	
	void lookat(Pose& self, const Vec3d& target) {
		
		// local unit Z vector of self, in global coordinate frame
		Vec3d uz;
		self.quat().toVectorZ(uz);
	
		// unit vector in path from self to target, in global coordinate frame
		Vec3d aim = Vec3d(target - self.pos()).normalize();
		
		// get rotation from current view to desired aim:
		Quatd rot = Quatd::getRotationTo(uz, aim);
		
		// rotate this into current view, and set as new current view:
		// (the inverse rotation helps to avoid 'wobble')
		self.quat() = rot * self.quat();
		// appears to be necessary to avoid crunchy animation:
		self.quat().normalize();	
		
	}
	
	void lookat(Pose& self, const Vec3d& target, double amt, double distance=0) {
		
		// local unit Z vector of self, in global coordinate frame
		Vec3d uz;
		self.quat().toVectorZ(uz);
	
		// unit vector in path from self to target, in global coordinate frame
		Vec3d aim = Vec3d(target - self.pos());
		double magSqr = aim.magSqr();
		aim.normalize();
		
		// get rotation from current view to desired aim:
		Quatd rot = Quatd::getRotationTo(uz, aim);
		
		// rotate this into current view, and set as new current view:
		// (the inverse rotation helps to avoid 'wobble')
		self.quat().slerp(rot * self.quat(), amt);
		
		if (distance != 0) {
			// try to move towards/away from it to keep a similar distance:
			double distSqr = distance*distance;
			//magSqr, distSqr. if magSqr is bigger, move in uz, else in -uz. 
			double factor = (magSqr - distSqr) / distSqr;
			//if (factor>0)
				mNudge[2] += factor;
		}
	}

protected:	
	// current navigation velocity as Cartesian derivative, relative to own coordinate frame
	Vec3d mRelVel, mRelVelI, mNudge;
	// current navigation as Euler rotation, relative to own coordinate frame
	Vec3d mEuler, mEulerI;
	// rotation is cached as quaternion for efficiency
	Quatd mQuat;	
};



} // al::

#endif

