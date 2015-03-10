/*
 * SlotAllocationBitmap.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: ti5x23
 */

#include "inet/linklayer/ieee802154e/DSMESlotAllocationBitmap.h"

namespace inet {

DSMESlotAllocationBitmap::DSMESlotAllocationBitmap() {
}

DSMESlotAllocationBitmap::DSMESlotAllocationBitmap(uint16_t numSuperframes, uint8_t numSlots, uint8_t numChannels) {
    bitmap.reserve(numSuperframes);
    this->numSlots = numSlots;
    this->numChannels = numChannels;
    subBlockLength = numSlots * numChannels;
    for (uint16_t i=0; i<numSuperframes; i++) {
        BitVector subBlock = BitVector(0, subBlockLength);
        bitmap.push_back(subBlock);
    }
}

DSME_SAB_Specification DSMESlotAllocationBitmap::allocateSlots(DSME_SAB_Specification sabSpec, uint8_t numSlots, uint16_t preferredSuperframe, uint8_t preferredSlot) {
    // TODO subblock may have different length
    EV_DETAIL << "received subBlock: \t";
    BitVector slotsOccupied = sabSpec.subBlock;
    EV << slotsOccupied.toString() << endl;
    slotsOccupied |= getSubBlock(sabSpec.subBlockIndex);
    EV << "local subBlock intersection: " << slotsOccupied.toString() << endl;

    // TODO assuming only preferred superframe in recieved subblock
    DSME_SAB_Specification replySabSpec;
    replySabSpec.subBlock = BitVector(0, slotsOccupied.getSize());;
    replySabSpec.subBlockIndex = sabSpec.subBlockIndex;
    replySabSpec.subBlockLength = sabSpec.subBlockLength;
    uint8_t numAllocated = 0;
    EV_DETAIL << "allocating slot-channel starting at " << (int)preferredSlot;
    for (uint16_t i = preferredSlot; i < slotsOccupied.getSize(); i++) {
        if(!slotsOccupied.getBit(i)) {
            replySabSpec.subBlock.setBit(i, true);
            numAllocated++;
            EV << ", " << i;
            // TODO pushback GTS to RX/TX lists or on handleNotify?
            if(numAllocated == numSlots) {
                EV << endl;
                return replySabSpec;
            } else {
                // channel in slot selected, check next slot
                i += numChannels - 1; // try to stay on same channel
                EV << " -> " << i;
            }
        }
    }
    EV << endl;

    // TODO further search in following superframes or reply failure? Just if nothin found?
    return replySabSpec;
}

std::list<GTS> DSMESlotAllocationBitmap::getGTSsFromAllocation(DSME_SAB_Specification sabSpec) {
    std::list<GTS> gtss;
    for (uint16_t i=0; i<sabSpec.subBlock.getSize(); i++) {
        if (sabSpec.subBlock.getBit(i))
            gtss.push_back(getGTS(sabSpec.subBlockIndex, i));
    }
    return gtss;
}

void DSMESlotAllocationBitmap::updateSlotAllocation(DSME_SAB_Specification sabSpec) {
    EV_DETAIL << "DSME: Updating neighboring GTS allocation bitmap for superframe(" << sabSpec.subBlockIndex << "): ";
    bitmap[sabSpec.subBlockIndex] |= sabSpec.subBlock;
    EV << bitmap[sabSpec.subBlockIndex].toString() << endl;
}

BitVector DSMESlotAllocationBitmap::getSubBlock(uint8_t superframeID) {
    // TODO subBlockIndex in units or in bits?!
    return bitmap[superframeID];
}

GTS DSMESlotAllocationBitmap::getRandomFreeGTS() {
    uint8_t start = intuniform(0, bitmap.size()-1);
    return getRandomFreeGTS(start);
}

GTS DSMESlotAllocationBitmap::getRandomFreeGTS(uint16_t superframeID) {
    uint16_t start = intuniform(0, bitmap.size()-1);
    uint16_t slot = intuniform(0, subBlockLength-1);
    for (uint16_t i=start; i<bitmap.size()-1; i++) {
        BitVector subBlock = getSubBlock(i);
        for (uint16_t j=slot; j<subBlockLength-1; j++) {
            if (!subBlock.getBit(j))
                return getGTS(i, j);
        }
    }
    for (uint16_t i=0; i<start; i++) {
        BitVector subBlock = getSubBlock(i);
        for (uint16_t j=0; j<subBlockLength-1; j++) {
            if (!subBlock.getBit(j))
                return getGTS(i, j);
        }
    }
    return GTS::UNDEFINED;
}

uint8_t DSMESlotAllocationBitmap::getSubBlockLength() {
    return subBlockLength / 8;  // TODO actually + subBlockLength % 8
}

GTS DSMESlotAllocationBitmap::getGTS(uint16_t superframeID, uint16_t offset) {
    return GTS(superframeID, offset/numChannels, offset%numChannels);
}

uint16_t DSMESlotAllocationBitmap::getSubBlockIndex(GTS& gts) {
    return gts.slotID * numChannels + gts.channel;
}

} /* namespace inet */
