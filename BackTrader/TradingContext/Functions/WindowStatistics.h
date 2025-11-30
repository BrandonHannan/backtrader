#ifndef WINDOWSTATISTICS_H
#define WINDOWSTATISTICS_H

#include <queue>
#include <set>
#include <memory>
#include <cmath>

using namespace std;

class WindowStatistics {
	private:
		int windowSize;
		queue<double> dataPoints;
		multiset<double> sortedWindow;
		double sum;
		double sumSQ;
		double sumXY;
		double indexSum;
		double indexSumSQ;

		double mean;
		double std;
		double slope;
		double slopeSE;
		double slopeRSQ;
		double slopeTStatistic;

		static constexpr double EPS = 1e-14;
		static constexpr double PValue = 0.01;

		void updateVariables();
	public:
		WindowStatistics(int windowSize);

		void addDataPoint(double value);
		int getSize() const;
		double getMean() const;
		double getStd() const;
		double getSlope() const;
		double getSlopeSE() const;
		double getSlopeRSQ() const;
		bool getSlopeSignificance() const;

		double getMin() const;
        double getMax() const;

		bool isReady() const;
};

#endif