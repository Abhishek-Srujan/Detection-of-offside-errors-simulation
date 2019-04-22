#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H

#include <vector>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseModule.h"
#include "SimpleAddress.h"

class MIXIM_API TestApplication : public BaseModule {
private:
	/** @brief Copy constructor is not allowed.
	 */
	TestApplication(const TestApplication&);
	/** @brief Assignment operator is not allowed.
	 */
	TestApplication& operator=(const TestApplication&);

public:
	TestApplication()
		: BaseModule()
		, dataOut(-1)
		, dataIn(-1)
		, ctrlOut(-1)
		, ctrlIn(-1)
		, delayTimer(NULL)
		, nbPackets(0)
		, remainingPackets(0)
		, headerLength(0)
		, nodeAddr()
		, dstAddr()
		, trafficParam(0.0)
		, debug(false), stats(false), trace(false)
		, flood(false)
		, isTransmitting(false)
		, INITIAL_DELAY(0)
		, PAYLOAD_SIZE(0)
		, nbPacketsReceived(0)
		, latencies()
		, latenciesRaw()
		, testStat()
        , nodeOne(0.0)
        , nodeTwo(0.0)
        , nodeThree(0.0)
        , nodeOnePrev(0.0)
        , nodeTwoPrev(0.0)
        , nodeThreePrev(0.0)
        , x1(0.0)
        , y1(0.0)
        , x2(0.0)
        , y2(0.0)
        , x3(0.0)
        , y3(0.0)
        , x(0.0)
        , y(0.0)
	{}

	virtual ~TestApplication();

	virtual void initialize(int stage);
	virtual void handleMessage(cMessage* msg);
        virtual void finish();

    protected:

        // gates
    	int dataOut;
    	int dataIn;
        int ctrlOut;
        int ctrlIn;

        // timers
        cMessage* delayTimer;

        // module parameters
        int nbPackets;
        int remainingPackets;
        int headerLength;
        LAddress::L3Type nodeAddr;
        LAddress::L3Type dstAddr;
        double trafficParam;
        bool debug, stats, trace;
        bool flood;
        bool isTransmitting;
        double nodeOne;
        double nodeTwo;
        double nodeThree;
        double nodeOnePrev;
        double nodeTwoPrev;
        double nodeThreePrev;
        double x1;
        double y1;
        double x2;
        double y2;
        double x3;
        double y3;
        double x;
        double y;

        // constants
        int INITIAL_DELAY;
        int PAYLOAD_SIZE;

        // state variables
        int nbPacketsReceived;
        std::vector < cStdDev > latencies;
        cOutVector latenciesRaw;
        cStdDev testStat;
};

#endif // TEST_APPLICATION_H

