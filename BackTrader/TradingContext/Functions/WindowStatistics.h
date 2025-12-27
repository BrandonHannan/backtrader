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
		// A P value is generally used, but in this case the slopeTStatistic holds strength of the trend, whereas the p value is a probability derived 
		// from the T statistic. In this case, to compare the p value to the slopeTStatistic is to compare it to a critical value and a p value 
		// of 0.01 is roughly equal to 2.58
		// static constexpr double PValue = 0.01;
		static constexpr double CritialValue = 2.58;

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