/*
 * Copyright (C) 2013 Florian Meier <florian.meier@koalo.de>
 *
 * Based on:
 * Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
 * Copyright (C) 2013 OpenSim Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "StaticConcentricMobility.h"

namespace inet {

Define_Module(StaticConcentricMobility);

void StaticConcentricMobility::setInitialPosition()
{
    int numHosts = par("numHosts");
    double distance = par("distance");

    /*
	double r = 0; // will be more!
	int Concentrics = 0;

	cout << nodes << endl;
	nodes--; // tower

	vector<pair<float,float> > mvec;
	mvec.reserve(nodes);

	double xmin, xmax, ymin, ymax;
	xmin = xmax = ymin = ymax = 0;

	int n = 1;
	while(nodes > 0) {
		r += mdia;

		cerr << "On " << Concentrics << " Concentrics: " << mvec.size() << " mirrors" << endl;

		double perimeter = 2*M_PI*r;
		int nodesOnPerimeter = perimeter/mdia;
		//cout << Concentrics << " " << nodesOnPerimeter << endl;

		double angstep = 2*M_PI/nodesOnPerimeter;

		bool lastConcentric = false;
		if(nodes <= nodesOnPerimeter) {
			lastConcentric = true;
		}

		for(int i = 0; i < nodesOnPerimeter && nodes > 0; i++, nodes--, n++) {
			double ang = i*angstep;
			double x = r*cos(ang);
			double y = r*sin(ang); 
			if(x < xmin) xmin = x;
			if(x > xmax) xmax = x;
			if(y < ymin) ymin = y;
			if(y > ymax) ymax = y;
			mvec.push_back(make_pair(x,y));
			if(lastConcentric) {
				measuredNodes << n << endl;
			}
		}

		Concentrics++;
	}


	for(vector<pair<float,float> >::iterator i = mvec.begin(); i != mvec.end(); i++) {
		cout << i->first << " " << i->second << endl;
	}
*/


    int index = visualRepresentation->getIndex();

    int myCircle = 0;
    int totalCircles = 0;
    int nodesHandled = 1;
    int nodesOnInnerCircles = 0;

    do {
        if(nodesHandled <= index) {
            nodesOnInnerCircles = nodesHandled;
            myCircle = totalCircles+1;
        }

        totalCircles++;
        nodesHandled += (int)(2*M_PI*totalCircles);
    } while(nodesHandled < numHosts);


    lastPosition.x = constraintAreaMin.x+distance*totalCircles;
    lastPosition.y = constraintAreaMin.y+distance*totalCircles;

    if(index > 0) {
        double radius = distance*myCircle;
        double angularStep = 2.0*M_PI/(int)(2*M_PI*myCircle);
        double angle = angularStep*(index-nodesOnInnerCircles);
        lastPosition.x += radius*cos(angle);
        lastPosition.y += radius*sin(angle);
    }

    lastPosition.z = par("initialZ");
    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
    recordScalar("z", lastPosition.z);
}

} // namespace inet

