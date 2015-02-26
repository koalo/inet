/*
 * SlotAllocationBitmap.h
 *
 *  Created on: Feb 25, 2015
 *      Author: ti5x23
 */

#ifndef SLOTALLOCATIONBITMAP_H_
#define SLOTALLOCATIONBITMAP_H_

#include "inet/common/BitVector.h"
#include "inet/linklayer/ieee802154e/GTS.h"

namespace inet {

class SlotAllocationBitmap {
protected:

    std::vector<BitVector> bitmap;
    uint8_t numSlots;
    uint8_t subBlockLength;

public:
    SlotAllocationBitmap();
    SlotAllocationBitmap(uint8_t numSuperframes, uint8_t numSlots, uint8_t numChannels);

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
    GTS getRandomFreeGTS(uint8_t superframeID);

    uint8_t getSubBlockLength();

protected:
    GTS getGTS(uint8_t superframeID, uint8_t slotID);
};

} /* namespace inet */

#endif /* SLOTALLOCATIONBITMAP_H_ */
