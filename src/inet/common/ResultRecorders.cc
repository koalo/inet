//==========================================================================
//  RESULTRECORDERS.CC - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2008 Andras Varga
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "ResultRecorders.h"
#include <sstream>

NAMESPACE_BEGIN

Register_ResultRecorder("groupCount", GroupCountRecorder);

void GroupCountRecorder::collect(std::string value) {
    groupcounts[value]++;
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b) {
    collect(b ? "true" : "false");
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, long l) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, double d) {
    std::stringstream s;
    s << d;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v) {
    collect(v.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s) {
    collect(s);
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj) {
    collect(obj->getFullPath());
}

void GroupCountRecorder::finish(cResultFilter *prev) {
    opp_string_map attributes = getStatisticAttributes();

    for(auto i = groupcounts.begin(); i != groupcounts.end(); i++) {
        std::stringstream name;
        name << getResultName().c_str() << ":" << i->first;
        ev.recordScalar(getComponent(), name.str().c_str(), i->second, &attributes); // note: this is NaN if count==0
    }
}

NAMESPACE_END

