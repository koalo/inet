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

DSME_SAB_Specification DSMESlotAllocationBitmap::allocateSlots(DSME_SAB_Specification sabSpec, uint8_t numSlots, uint16_t preferredSuperframe, uint8_t preferredSlot, const gts_allocation& allocatedGTS) {
    // if superframe is fully allocated try next, stop if all superframes were tried
    static unsigned numSuperframesTried = 0;

    // TODO subblock may have different length
    EV_DETAIL << "received subBlock: \t";
    BitVector slotsOccupied = sabSpec.subBlock;
    EV << slotsOccupied.toString() << endl;
    slotsOccupied |= getSubBlock(sabSpec.subBlockIndex);
    EV << "local subBlock intersection: " << slotsOccupied.toString() << endl;

    // TODO assuming only preferred superframe in recieved subblock
    DSME_SAB_Specification replySabSpec;
    replySabSpec.subBlock = BitVector(0, slotsOccupied.getSize());;
    replySabSpec.subBlockIndex = preferredSuperframe;
    replySabSpec.subBlockLength = sabSpec.subBlockLength;
    unsigned slotOffset = preferredSlot * numChannels;
    unsigned channel = 0;
    if (numSuperframesTried == 0) {             // random channel on first try
        channel = intuniform(0, numChannels-1);
        for (unsigned i=0; i<numChannels; i++) { // if possible use preferredSlot (check every channel)
            if (slotsOccupied.getBit(slotOffset + channel))
                channel = (channel < numChannels) ? channel + 1 : 0;
            else
                break;
        }
    }

    unsigned numAllocated = 0;
    EV_DETAIL << "allocating slot with preferred slot " << (int)preferredSlot << " and random channel: " << channel;
    for (unsigned i = slotOffset + channel; i < slotsOccupied.getSize(); i++) {
        if(GTS::UNDEFINED == allocatedGTS[preferredSuperframe][i/numChannels] &&
                !slotsOccupied.getBit(i)) {
            replySabSpec.subBlock.setBit(i, true);
            numAllocated++;
            EV << ", " << i;
            // TODO pushback GTS to RX/TX lists or on handleNotify?
            if(numAllocated == numSlots) {
                EV << " => " << numAllocated << " allocated" << endl;
                return replySabSpec;
            } else {
                // channel in slot selected, check next slot
                i += numChannels - 1; // try to stay on same channel
                EV << " -> " << i;
            }
        }
    }
    EV << endl;

    // if not found any free slot, try next superframe
    EV_DEBUG << "numAllocated: " << numAllocated << ", superframesTried: " << numSuperframesTried << " of " << bitmap.size() << endl;
    numSuperframesTried++;
    if (numAllocated == 0 && numSuperframesTried < bitmap.size()) {
        EV_DETAIL << "DSME allocateSlots: did not find any free slot in superframe " << preferredSuperframe << endl;
        return allocateSlots(sabSpec, numSlots, (preferredSuperframe<bitmap.size()-1)?preferredSuperframe+1:0, 0, allocatedGTS);
    }
    else {
        numSuperframesTried = 0;
    }

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

void DSMESlotAllocationBitmap::updateSlotAllocation(DSME_SAB_Specification sabSpec, short manType) {
    EV_DETAIL << "DSME: Updating(" << manType << ") neighboring GTS allocation bitmap for superframe(" << sabSpec.subBlockIndex << "): ";
    if (manType == ALLOCATION)
        bitmap[sabSpec.subBlockIndex] |= sabSpec.subBlock;
    else if (manType == DEALLOCATION)
        bitmap[sabSpec.subBlockIndex] &= ~sabSpec.subBlock;
    else
        EV_ERROR << "UNKOWN MANAGEMENT TYPE" << endl;
    EV << bitmap[sabSpec.subBlockIndex].toString() << endl;
}

BitVector DSMESlotAllocationBitmap::getSubBlock(uint8_t superframeID) {
    // TODO subBlockIndex in units or in bits?!
    return bitmap[superframeID];
}

GTS DSMESlotAllocationBitmap::getRandomFreeGTS(const gts_allocation& allocatedGTS) {
    uint8_t start = intuniform(0, bitmap.size()-1);
    return getRandomFreeGTS(start, allocatedGTS);
}

GTS DSMESlotAllocationBitmap::getRandomFreeGTS(uint16_t superframeID, const gts_allocation& allocatedGTS) {
    uint16_t start = intuniform(0, bitmap.size()-1);
    uint16_t slot = intuniform(0, subBlockLength-1);
    for (uint16_t i=start; i<bitmap.size()-1; i++) {
        BitVector subBlock = getSubBlock(i);
        for (uint16_t j=slot; j<subBlockLength-1; j++) {
            if (GTS::UNDEFINED == allocatedGTS[i][j/numChannels]
                    && !subBlock.getBit(j))
                return getGTS(i, j);
        }
    }
    for (uint16_t i=0; i<start; i++) {
        BitVector subBlock = getSubBlock(i);
        for (uint16_t j=0; j<subBlockLength-1; j++) {
            if (GTS::UNDEFINED == allocatedGTS[i][j/numChannels]
                    && !subBlock.getBit(j))
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

uint16_t DSMESlotAllocationBitmap::getSubBlockIndex(const GTS& gts) {
    return gts.slotID * numChannels + gts.channel;
}

} /* namespace inet */
