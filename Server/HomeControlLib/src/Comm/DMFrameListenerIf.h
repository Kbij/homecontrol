/*
 * DMFrameListenerIf.h
 *
 *  Created on: Jul 7, 2016
 *      Author: koen
 */

#ifndef COMM_DMFRAMELISTENERIF_H_
#define COMM_DMFRAMELISTENERIF_H_
#include <string>
#include <vector>

namespace CommNs {

class DMFrameListenerIf
{
public:
	virtual ~DMFrameListenerIf() {};

	virtual void receiveFrame(const std::vector<uint8_t>& data) = 0;
};

} /* namespace CommNs */




#endif /* COMM_DMFRAMELISTENERIF_H_ */
