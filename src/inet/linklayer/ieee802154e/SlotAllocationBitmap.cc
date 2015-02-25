/*
 * SlotAllocationBitmap.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: ti5x23
 */

#include "inet/linklayer/ieee802154e/SlotAllocationBitmap.h"

namespace inet {

SlotAllocationBitmap::SlotAllocationBitmap() {
}

SlotAllocationBitmap::SlotAllocationBitmap(uint8_t numSuperframes, uint8_t numSlots, uint8_t numChannels) {
    bitmap.reserve(numSuperframes);
    this->numSlots = numSlots;
    subBlockLength = numSlots * numChannels;
    for (uint8_t i=0; i<numSuperframes; i++) {
        BitVector subBlock = BitVector(subBlockLength);
        bitmap.push_back(subBlock);
    }
}

BitVector SlotAllocationBitmap::getSubBlock(uint8_t superframeID) {
    return bitmap[superframeID];
}

GTS SlotAllocationBitmap::getRandomFreeGTS() {
    uint8_t start = intuniform(0, bitmap.size()-1);
    return getRandomFreeGTS(start);
}

GTS SlotAllocationBitmap::getRandomFreeGTS(uint8_t superframeID) {
    uint8_t start = intuniform(0, bitmap.size()-1);
    uint8_t slot = intuniform(0, subBlockLength-1);
    for (uint8_t i=start; i<bitmap.size()-1; i++) {
        BitVector subBlock = getSubBlock(i);
        for (uint8_t j=slot; j<subBlockLength; j++) {
            if (!subBlock.getBit(j))
                return getGTS(i, j);
        }
    }
    for (uint8_t i=0; i<start; i++) {
        BitVector subBlock = getSubBlock(i);
        for (uint8_t j=0; j<subBlockLength; j++) {
            if (!subBlock.getBit(j))
                return getGTS(i, j);
        }
    }
    return GTS::UNDEFINED;
}

GTS SlotAllocationBitmap::getGTS(uint8_t superframeID, uint8_t offset) {
    return GTS(superframeID, offset/numSlots, offset%numSlots);
}

} /* namespace inet */
