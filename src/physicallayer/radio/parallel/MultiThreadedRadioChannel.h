//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MULTITHREADEDRADIOCHANNEL_H_
#define __INET_MULTITHREADEDRADIOCHANNEL_H_

#include <queue>
#include <deque>
#include <pthread.h>
#include "RadioChannel.h"

/**
 * This implementation uses 1 background thread for invalidation and multiple
 * other background threads for computing the cache.
 * TODO: INCOMPLETE
 *
 * Common tasks (among all threads)
 *  - read cache (read arrivals/receptions/reception decisions)
 *    acquire cache read lock
 *  - write cache (write arrivals/receptions/reception decisions)
 *    acquire cache write lock
 *  - read jobs (peek/iterate computation jobs)
 *    acquire job read lock
 *  - write jobs (push/pop computation jobs)
 *    acquire job write lock
 *
 * Main thread tasks
 *  - read radio channel state (read/iterate radios/transmissions)
 *    just read, no concurrent writers, race conditions impossible, locks not acquired
 *  - write radio channel state (add/remove radios/transmissions)
 *    just write, no concurrent readers, race conditions impossible, locks not acquired
 *  - invalidate cache and all ongoing computations in worker threads
 *    acquire cache write lock and increment cache version number
 *  - remove obsolete channel state
 *    disable workers, wait all workers to complete their jobs, remove obsolete state
 *
 * Worker thread tasks
 *  - read radio channel state (read/iterate radios/transmissions)
 *    vectors of radios/transmissions are copied, just read immutable state,
 *    no concurrent writers, race conditions impossible, locks not acquired
 *  - write radio channel state (add/remove radios/transmissions)
 *    this is not allowed, raise an error
 */
// TODO: error handling to avoid leaving locks when unwinding
// TODO: seems to be unreasonable slow?
class INET_API MultiThreadedRadioChannel : public RadioChannel
{
    protected:
        class InvalidateCacheJob
        {
            public:
                const IRadioSignalTransmission *transmission;

            public:
                InvalidateCacheJob(const IRadioSignalTransmission *transmission) :
                    transmission(transmission)
                {}
        };

        class ComputeCacheJob
        {
            public:
                const IRadio *receiver;
                const IRadioSignalListening *listening;
                const IRadioSignalTransmission *transmission;
                simtime_t receptionStartTime;
                simtime_t receptionEndTime;

            public:
                ComputeCacheJob(const IRadio *receiver, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission, const simtime_t receptionStartTime, const simtime_t receptionEndTime) :
                    receiver(receiver),
                    listening(listening),
                    transmission(transmission),
                    receptionStartTime(receptionStartTime),
                    receptionEndTime(receptionEndTime)
                {}

                static bool compareReceptionStartTimes(const ComputeCacheJob &job1, const ComputeCacheJob &job2)
                {
                    return job1.receptionEndTime < job2.receptionEndTime;
//                    return job1.receptionStartTime < job2.receptionStartTime;
                }
        };

        class ComputeCacheJobComparer
        {
            public:
                bool operator()(const ComputeCacheJob &job1, const ComputeCacheJob &job2)
                {
                    return job1.receptionStartTime > job2.receptionStartTime;
                }
        };

        class ComputeCacheJobQueue : public std::vector<ComputeCacheJob>
        {
            protected:
                ComputeCacheJobComparer comparer;

            public:
                ComputeCacheJobQueue()
                {
                    make_heap(begin(), end(), comparer);
                }

                const ComputeCacheJob& top()
                {
                    return front();
                }

                void push(const ComputeCacheJob& job)
                {
                    push_back(job);
                    push_heap(begin(), end(), comparer);
                }

                void pop()
                {
                    pop_heap(begin(), end(), comparer);
                    pop_back();
                }

                void sort()
                {
                    make_heap(begin(), end(), comparer);
                }
        };

    protected:
        bool isWorkersEnabled;
        pthread_t mainThread;
        std::vector<pthread_t *> workers;
        mutable pthread_mutex_t cacheLock;
        mutable pthread_mutex_t invalidateCacheJobsLock;
        mutable pthread_mutex_t computeCacheJobsLock;
        mutable pthread_cond_t invalidateCacheJobsCondition;
        mutable pthread_cond_t computeCacheJobsCondition;
        int runningInvalidateCacheJobCount;
        int runningComputeCacheJobCount;
        std::vector<const IRadioSignalTransmission *> transmissionsCopy;
        std::deque<InvalidateCacheJob> invalidateCacheJobs;
        ComputeCacheJobQueue computeCacheJobs;

    protected:
        virtual void initialize(int stage);
        virtual void initializeWorkers(int workerCount);
        virtual void terminateWorkers();
        static void *callInvalidateCacheWorkerLoop(void *argument);
        static void *callComputeCacheWorkerLoop(void *argument);
        virtual void *invalidateCacheWorkerLoop(void *argument);
        virtual void *computeCacheWorkerLoop(void *argument);
        virtual void waitForInvalidateCacheJobsToComplete() const;

        virtual void assertMainThread() const { ASSERT(pthread_self() == mainThread); }
        virtual void assertWorkerThread() const { ASSERT(pthread_self() != mainThread); }

//        virtual const IRadioSignalArrival *getCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
//        virtual void setCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalArrival *arrival) const;
//        virtual void removeCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
//
//        virtual const IRadioSignalReception *getCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
//        virtual void setCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const;
//        virtual void removeCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
//
//        virtual const IRadioSignalReceptionDecision *getCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
//        virtual void setCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReceptionDecision *decision);
//        virtual void removeCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission);

        virtual void removeNonInterferingTransmission(const IRadioSignalTransmission *transmission);

        virtual void invalidateCachedDecisions(const IRadioSignalTransmission *transmission);
        virtual void invalidateCachedDecision(const IRadioSignalReceptionDecision *decision);

    public:
        MultiThreadedRadioChannel() :
            RadioChannel(),
            isWorkersEnabled(false),
            runningInvalidateCacheJobCount(0),
            runningComputeCacheJobCount(0)
        {}

        MultiThreadedRadioChannel(const IRadioSignalPropagation *propagation, const IRadioSignalAttenuation *attenuation, const IRadioBackgroundNoise *noise, const simtime_t minInterferenceTime, const simtime_t maxTransmissionDuration, m maxCommunicationRange, m maxInterferenceRange, int workerCount) :
            RadioChannel(propagation, attenuation, noise, minInterferenceTime, maxTransmissionDuration, maxCommunicationRange, maxInterferenceRange),
            isWorkersEnabled(false),
            runningInvalidateCacheJobCount(0),
            runningComputeCacheJobCount(0)
        { initializeWorkers(workerCount); }

        virtual ~MultiThreadedRadioChannel();

        virtual void addRadio(const IRadio *radio);
        virtual void removeRadio(const IRadio *radio);

        virtual void transmitToChannel(const IRadio *radio, const IRadioSignalTransmission *transmission);
        virtual void sendToChannel(IRadio *radio, const IRadioFrame *frame);

        virtual const IRadioSignalReceptionDecision *receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const;
        virtual const IRadioSignalListeningDecision *listenOnChannel(const IRadio *radio, const IRadioSignalListening *listening) const;
};

#endif
