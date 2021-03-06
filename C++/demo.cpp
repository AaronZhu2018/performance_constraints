/*This is a demo of Performance Constraints for a robotic manipulator.
The KUKA LWR 4+ is provided as an example here.

A description and the overall algorithm of the method is in:
- Dimeas, Fotios, Vassilis C. Moulianitis, and Nikos Aspragathos. 
"Manipulator performance constraints in human-robot cooperation." 
Robotics and Computer-Integrated Manufacturing (2017).

Author: Fotis Dimeas
Copyright 2017 Fotios Dimeas
 */

#include <cstdlib>
#include <iostream>

#define ARMA_DONT_USE_CXX11 //remove warning for incomplete C++11 support
#include "armadillo" //Linear Algebra Library
#include "performanceConstraints.h"

using namespace std;

int main(int argc, char** argv) {
	cout << "Armadillo version: " << arma::arma_version::as_string() << endl;

	/*Initialize performance constraints
	* Arguments:
	* 1: w_cr for translation
	* 2: w_th for translation
	* 3: w_cr for rotation
	* 4: w_th for rotation
	* 5: lambda for translation
	* 6: lambda for rotation
	* 7: Performance indices [1, 2, 3]  1: Manipulability Index, 2: Minimum Singular Value, 3: Inverse Condition Number
	* 8  Calculation methods: [_serial, _parallel, _parallel_nonblock]
	*/
	// PC pConstraints(	0.011,	0.03,	0.15,	0.5,	1.0,	1.0,	1,		_serial); //Using manipulability index
	PC pConstraints(	0.03,	0.14,	0.1,	0.5,	1.0,	1.0,	2,		_parallel); //Using MSV (better for human robot interaction)
	
	pConstraints.setVerbose(1); //Set debug info. Comment or set to 0 to disable

	arma::vec q; q << 0. << -M_PI/4. << 0. << M_PI/2. << 0. << -M_PI/4. << 0.0; //An example initial configuration 
	
	arma::mat J(6,7);  
	cout.precision(4); cout.setf(ios::fixed);
	
	cout << "Entering control loop...\n";
	arma::wall_clock timer; timer.tic();
	int i;
	for (i=0; i<100; i++) //this is supposed to be a simulation control loop
	{
		pConstraints.updateCurrentConfiguration(q); //measure the robot's configuration and put it here
		pConstraints.get_Jsym(q, J); //calculate J from current q
		pConstraints.updateCurrentJacobian(J); 
		pConstraints.updatePC(); //Performance constraints are calculated in here

		arma::vec F = pConstraints.getSingularityTreatmentForce(); //

		if (pConstraints.checkForSingularity()) { //check for singularity since no handling of the robot's motion is done here
			cout << "Robot became singular. Stopping simulation..." <<endl; 
			break;	//stop the simulation
		}

		//gradually guide the robot to a singular configuration just to see the constraint forces
		q(1)+=0.01;
		q(3)-=0.01;
		q(5)+=0.01;
		
		//Put your controller here (e.g impedance or admittance)
		//...

	} 
	double time = timer.toc();
	cout << "Simulation completed in " <<  time<< "sec."<< endl;
	cout << "Average time per cycle: " << time/i << "sec."<< endl;
	// pConstraints.threadpool_join();
}