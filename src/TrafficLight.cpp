#include <iostream>
#include <random>
#include <chrono>
#include <thread>

#include "TrafficLight.h"

using namespace std::chrono;

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	std::unique_lock<std::mutex> lock(_mtx); // Lock the mutex with unique_lock
  	_condition.wait(lock, [this]() { return !_queue.empty(); }); // Wait for a message

  	// Pull the message from the front of the queue using move semantics
  	T msg = std::move(_queue.front());
  	_queue.pop_front(); // Remove the message from the queue
  	return msg;     // Return the message
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::lock_guard<std::mutex> lock(_mtx);
  	_queue.emplace_back(msg);
  	_condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	
  	TrafficLightPhase recvLight;
  	while(true)
    {
      	if( _queue.receive() == TrafficLightPhase::green)
          	break;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
	
  	// Random number generator for cycle durations
    std::random_device rand; // Seed
    std::mt19937 gen(rand()); // Mersenne Twister RNG
    std::uniform_int_distribution<int> dist(4000, 6000); // Random duration in milliseconds

    auto previousTime = high_resolution_clock::now(); // Start time
    int currentCycleDuration = dist(gen); // Random duration for the first cycle
    int elapsedCycleTime = 0;

    while (true) {
        // Wait 1ms between each loop iteration
        std::this_thread::sleep_for(milliseconds(1));

        // Measure elapsed time since last cycle
        auto currentTime = high_resolution_clock::now();
        elapsedCycleTime = duration_cast<milliseconds>(currentTime - previousTime).count();

        // Check if the elapsed time exceeds the random cycle duration
        if (elapsedCycleTime >= currentCycleDuration) {
			_currentPhase = (getCurrentPhase() == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red);
          	_queue.send(std::move(_currentPhase));
            // Reset timer and generate a new random cycle duration
            previousTime = currentTime;
            currentCycleDuration = dist(gen);
        }
    }  
}

