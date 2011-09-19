#ifndef INCLUDE_AL_FRUSTUM_HPP
#define INCLUDE_AL_FRUSTUM_HPP

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
	This is a rectangular frustum useful for computer graphics

	File author(s):
	Lance Putnam, 2011, putnam.lance@gmail.com
*/

#include "allocore/math/al_Plane.hpp"
#include "allocore/math/al_Vec.hpp"


namespace al {

template <class T> class Frustum;
typedef Frustum<double> Frustumd;	///< Double precision frustrum


/// Rectangular frustum

/// A frustum has the shape of a four-sided pyramid truncated at the top.
/// For graphics, this specifies the viewing frustum of a camera.
///
/// Source code adapted from:
/// "OpenGL @ Lighthouse 3D - View Frustum Culling Tutorial",
/// http://www.lighthouse3d.com/opengl/viewfrustum/index.php?intro
template <class T>
class Frustum{
public:

	enum{ TOP=0, BOTTOM, LEFT, RIGHT, NEARP, FARP };
	enum{ OUTSIDE=0, INTERSECT, INSIDE };

	Vec<3,T> ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;	///< Corners
	Plane<T> pl[6];										///< Faces

	const Vec<3,T>& corner(int i0, int i1, int i2) const {
		return (&ntl)[i2<<2 | i1<<1 | i0];
	}

	/// Get point in frustum corresponding to fraction along edges
	template <class U>
	Vec<3,T> getPoint(const Vec<3,U>& frac) const {
		return 
		lerp(frac[2],
			lerp(frac[1], 
				lerp(frac[0], corner(0,0,0), corner(1,0,0)),
				lerp(frac[0], corner(0,1,0), corner(1,1,0))
			),
			lerp(frac[1],
				lerp(frac[0], corner(0,0,1), corner(1,0,1)),
				lerp(frac[0], corner(0,1,1), corner(1,1,1))
			)
		);
	}

	/// Get point in frustum corresponding to fraction along edges
	template <class U>
	Vec<3,T> getPoint(const U& fracx, const U& fracy, const U& fracz) const {
		return getPoint(Vec<3,U>(fracx,fracy,fracz));
	}

	/// Test whether point is in frustum
	int testPoint(const Vec<3,T>& p) const;
	
	/// Test whether sphere is in frustum
	int testSphere(const Vec<3,T>& center, float radius) const;
	
	/// Test whether axis-aligned box is in frustum
	
	/// Note: this can return a false positive
	///
	int testBox(const Vec<3,T>& xyz, const Vec<3,T>& dim) const;

	/// Returns center of frustum
	Vec<3,T> center() const { return (ntl+ntr+nbl+nbr+ftl+ftr+fbl+fbr)*0.125; }

	/// Compute planes based on frustum corners (planes face to inside)
	
	///	The plane normals are computed assuming a right-hand coordinate system.
	///
	void computePlanes();

private:
	template <class Tf, class Tv>
	static Tv lerp(Tf f, const Tv& x, const Tv& y){
		return (y - x) * f + x;
	}
};




template <class T>
void Frustum<T>::computePlanes(){
	pl[TOP   ].from3Points(ntr,ntl,ftl);
	pl[BOTTOM].from3Points(nbl,nbr,fbr);
	pl[LEFT  ].from3Points(ntl,nbl,fbl);
	pl[RIGHT ].from3Points(nbr,ntr,fbr);
	pl[NEARP ].from3Points(ntl,ntr,nbr);
	pl[FARP  ].from3Points(ftr,ftl,fbl);
}

template <class T>
int Frustum<T>::testPoint(const Vec<3,T>& p) const {
	for(int i=0; i<6; ++i){
		if(pl[i].inNegativeSpace(p)) return OUTSIDE;
	}
	return INSIDE;
}

template <class T>
int Frustum<T>::testSphere(const Vec<3,T>& c, float r) const {
	int result = INSIDE;
	for(int i=0; i<6; ++i){
		float distance = pl[i].distance(c);
		if(distance < -r)		return OUTSIDE;
		else if(distance < r)	result = INTERSECT;
	}
	return result;
}

template <class T>
int Frustum<T>::testBox(const Vec<3,T>& xyz, const Vec<3,T>& dim) const {
	int result = INSIDE;
	for(int i=0; i<6; ++i){
		const Vec3d& plNrm = pl[i].normal();

/*
		The positive vertex is the vertex from the box that is further along 
		the normal's direction. The negative vertex is the opposite vertex.

		If the p-vertex is on the wrong side of the plane, the box can be 
		immediately rejected, as it falls completely outside the frustum. On the 
		other hand, if the p-vertex is on the right side of the plane, then 
		testing the whereabouts of the n-vertex tells if the box is totally on 
		the right side of the plane, or if the box intersects the plane.
*/		
		// Is positive vertex outside?
		Vec<3,T> vp = xyz;		
		if(plNrm[0] > 0) vp[0] += dim[0];
		if(plNrm[1] > 0) vp[1] += dim[1];
		if(plNrm[2] > 0) vp[2] += dim[2];
		if(pl[i].inNegativeSpace(vp)) return OUTSIDE;

		// Is negative vertex outside?
		Vec<3,T> vn = xyz;
		if(plNrm[0] < 0) vn[0] += dim[0];
		if(plNrm[1] < 0) vn[1] += dim[1];
		if(plNrm[2] < 0) vn[2] += dim[2];
		if(pl[i].inNegativeSpace(vn)) result = INTERSECT;
	}
	return result;
}

} // al::

#endif
