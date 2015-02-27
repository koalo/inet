/*
 * SlotAllocationBitmap.h
 *
 *  Created on: Feb 25, 2015
 *      Author: ti5x23
 */

#ifndef SLOTALLOCATIONBITMAP_H_
#define SLOTALLOCATIONBITMAP_H_

#include "inet/common/BitVector.h"
#include "inet/linklayer/ieee802154e/DSME_GTSRequestCmd_m.h"
#include "inet/linklayer/ieee802154e/GTS.h"

namespace inet {

class DSMESlotAllocationBitmap {
protected:

    std::vector<BitVector> bitmap;
    uint8_t numSlots;
    uint8_t numChannels;
    uint8_t subBlockLength;

public:
    DSMESlotAllocationBitmap();
    DSMESlotAllocationBitmap(uint16_t numSuperframes, uint8_t numSlots, uint8_t numChannels);

    /**
     * select numSlots free slots from intersection of received subBlock and local subBlock
     */
    BitVector allocateSlots(DSME_SAB_Specification sabSpec, uint8_t numSlots, uint16_t preferredSuperframe, uint8_t preferredSlot);

    /**
     * Get sub block for superframe
     */
    BitVector getSubBlock(uint8_t superframeID);

    /**
     * Get random free GTSlot
     */
    GTS getRandomFreeGTS();

    /**
     * Get random free GTSlot, starting from specified superframe
     */
    GTS getRandomFreeGTS(uint16_t superframeID);

    /**
     * Returns the subblock length unit in bytes which is numSlots*numChannels / 8
     */
    uint8_t getSubBlockLength();

protected:
    GTS getGTS(uint16_t superframeID, uint16_t slotID);
};

} /* namespace inet */

#endif /* SLOTALLOCATIONBITMAP_H_ */
