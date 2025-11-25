#ifndef WINDOWSTATISTICS_H
#define WINDOWSTATISTICS_H

#include <queue>
#include <memory>
#include <cmath>

using namespace std;

class WindowStatistics {
	private:
		int windowSize;
		queue<double> dataPoints;
		double sum;
		double sumSQ;
		double sumY;
		double sumXY;
		double indexSum;
		double indexSumSQ;
		unique_ptr<double> mean;
		unique_ptr<double> std;
		unique_ptr<double> slope;
		unique_ptr<double> slopeSE;
	public:
		WindowStatistics(int windowSize);

		void addDataPoint(double value);
		unique_ptr<double> getMean() const;
		unique_ptr<double> getStd() const;
		unique_ptr<double> getSlope() const;
		unique_ptr<double> getSlopeSE() const;
};

#endif