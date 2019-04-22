#include "TestApplication.h"

#include "BaseMacLayer.h"
#include "NetwControlInfo.h"
#include "ApplPkt_m.h"

using std::endl;

Define_Module(TestApplication);

void TestApplication::initialize(int stage) {
    BaseModule::initialize(stage);


    if (stage == 0) {
        // begin by connecting the gates
        // to allow messages exchange
        dataOut = findGate("lowerLayerOut");
        dataIn = findGate("lowerLayerIn");
        ctrlOut = findGate("lowerControlOut");
        ctrlIn  = findGate("lowerControlIn");

        // Retrieve parameters
        debug = par("debug").boolValue();
        stats = par("stats").boolValue();
        trace = par("trace").boolValue();

        isTransmitting    = false;

        nbPackets         = par("nbPackets");
        trafficParam      = par("trafficParam").doubleValue();
        nodeAddr          = LAddress::L3Type( par("nodeAddr").longValue() );
        dstAddr           = LAddress::L3Type( par("dstAddr").longValue() );
        //dstAddr           = par("dstAddr");
        flood             = par("flood").boolValue();
        PAYLOAD_SIZE      = par("payloadSize"); // data field size
        PAYLOAD_SIZE      = PAYLOAD_SIZE * 8; // convert to bits
        // Configure internal state variables and objects
        nbPacketsReceived = 0;
        remainingPackets  = nbPackets;

        INITIAL_DELAY     = 5; // initial delay before sending first packet

        nodeOne = 0.0;
        nodeTwo = 0.0;
        nodeThree = 0.0;

        nodeOnePrev = 0.0;
        nodeTwoPrev = 0.0;
        nodeThreePrev = 0.0;

        // Trilateration Init

        x1 = 30;
        y1 = 0;
        x2 = 0;
        y2 = 30;
        x3 = 0;
        y3 = 0;
        x = 0;
        y = 0;

        // start timer if needed
        if (nodeAddr != 0 && remainingPackets > 0) {
            delayTimer = new cMessage("app-delay-timer");
            scheduleAt(simTime() + INITIAL_DELAY +uniform(0,0.001), delayTimer); // we add a small shift to avoid systematic collisions
        } else {
            delayTimer = 0;
        }

        if (stats) {
            // we should collect statistics
            int nbNodes = getNode()->size();
            for (int i = 0; i < nbNodes; i++) {
                std::ostringstream oss;
                oss << "latency";
                oss << i;
                cStdDev aLatency(oss.str().c_str());
                latencies.push_back(aLatency);
            }
        }

        if (trace) {
            // record all packet arrivals
            latenciesRaw.setName("rawLatencies");
        }

    }

}

TestApplication::~TestApplication() {
	if (delayTimer)
        cancelAndDelete(delayTimer);
}

void TestApplication::finish() {

    if (stats) {
        recordScalar("Packets received", nbPacketsReceived);
        recordScalar("Aggregate latency", testStat.getMean());
        for (unsigned int i = 0; i < latencies.size(); i++) {
            cStdDev aLatency = latencies[i];
            aLatency.record();
        }
    }
}

void TestApplication::handleMessage(cMessage * msg) {
    long double timeConstant = 0.000531873600;
    long double distanceConstant = 0.000000003335;
    long double distance;


    double a[1],b[1],c,d;

	debugEV << "In TestApplication::handleMessage" << endl;
    if (msg == delayTimer) {
        if (debug) {
        	debugEV << "  processing application timer." << endl;
        }
        if (! isTransmitting) {
			// create and send a new message
            //simtime_t a = simTime();
			ApplPkt* msg2 = new ApplPkt("SomeData");
			msg2->setBitLength(PAYLOAD_SIZE);
			msg2->setSrcAddr(nodeAddr);
			msg2->setDestAddr(dstAddr);
			msg2->setTimestamp(simTime());
			NetwControlInfo::setControlInfo(msg2, dstAddr);
			if (debug) {
				debugEV << " sending down new data message to Aloha MAC layer for radio transmission." << endl;
			}
			send(msg2, dataOut);
			isTransmitting = true;
			// update internal state
			remainingPackets--;
		}
        // reschedule timer if appropriatetrue
        if (remainingPackets > 0) {
            if (!flood && !delayTimer->isScheduled()) {
                scheduleAt(simTime() + exponential(trafficParam) + 0.001, delayTimer);
            }
        } else {
            cancelAndDelete(delayTimer);
            delayTimer = 0;
        }
    } else if (msg->getArrivalGateId() == dataIn) {
        // we received a data message from someone else !
        ApplPkt* m = dynamic_cast<ApplPkt*>(msg);

        simtime_t temp = ((simTime() - m->getTimestamp()) - timeConstant) / distanceConstant;
        distance = temp.dbl();
        EV <<"Distance ="<< distance << endl;

        switch(m->getSrcAddr()){
            case 1:
               nodeOne = distance;
               break; //optional
            case 2:
               nodeTwo = distance;
               break; //optional
            case 3:
                nodeThree = distance;
                break; //optional
            default :
               EV << "Not supposed to come here"<< endl;
        }


        if(nodeOnePrev != nodeOne && nodeTwoPrev != nodeTwo && nodeThreePrev != nodeThree)
        {
                    a[0] = x3 - x1;
                    a[1] = y3 - y1;
                    b[0] = x3 - x2;
                    b[1] = y3 - y2;
                    c = ((pow(nodeOne,2) - pow(nodeThree,2)) - (pow(x1,2) - pow(x3,2)) - (pow(y1,2) - pow(y3,2)))/2.0;
                    d = ((pow(nodeTwo,2) - pow(nodeThree,2)) - (pow(x2,2) - pow(x3,2)) - (pow(y2,2) - pow(y3,2)))/2.0;


                    x = (c*b[1] - d*a[1]) / (a[0]*b[1] - a[1]*b[0]);
                    y = (c*b[0] - d*a[0]) / (a[1]*b[0] - a[0]*b[1]);

                    nodeOnePrev = nodeOne;
                    nodeTwoPrev = nodeTwo;
                    nodeThreePrev = nodeThree;

        }




        EV << "Distance from node one =" << nodeOne << "\n Distance from node two =" << nodeTwo
                << "\n Distance from node three =" << nodeThree << endl;

        EV << "Position of node 0 is (" << x << "," << y << ")" << endl;



        	EV << "I (" << nodeAddr << ") received a message from node "
                << m->getSrcAddr() << " of size " << m->getBitLength() << "at" << m->getTimestamp() << endl;
        	EV << "time taken = "<<simTime() - m->getTimestamp() << endl;
        	EV << "distance = "<< ((simTime() - m->getTimestamp()) - timeConstant) / distanceConstant  << endl;
        nbPacketsReceived++;
        // 56393.768617146*

        if (stats) {
            simtime_t theLatency = msg->getArrivalTime() - msg->getCreationTime();
            latencies[m->getSrcAddr()].collect(SIMTIME_DBL(theLatency));
            testStat.collect(SIMTIME_DBL(theLatency));
        }

        if (trace) {
            simtime_t theLatency = msg->getArrivalTime() - msg->getCreationTime();
            latenciesRaw.record(SIMTIME_DBL(theLatency));
        }

        delete msg;
    } else if (msg->getArrivalGateId() == ctrlIn) {
    	debugEV << "Received a control message." << endl;
        // msg announces end of transmission.
        if (msg->getKind() ==BaseMacLayer::TX_OVER) {
        	isTransmitting = false;
            if (remainingPackets > 0 && flood && !delayTimer->isScheduled()) {
                scheduleAt(simTime() + 0.001*001 + uniform(0, 0.001*0.001), delayTimer);
            }
        }
        delete msg;
    } else {
        // default case
        if (debug) {
			ApplPkt* m = static_cast<ApplPkt*>(msg);
			debugEV << "I (" << nodeAddr << ") received a message from node "
					<< (static_cast<ApplPkt*>(msg))->getSrcAddr() << " of size " << m->getBitLength() << "." << endl;
        }
        delete msg;
    }
}

